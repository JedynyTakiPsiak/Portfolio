#include "data_manager.h"
#include "api/api_client.h"
#include "funkcje/rate_limiter.h"
#include "funkcje/indicators/progress_bar.hpp"
#include "funkcje/indicators/cursor_control.hpp"

#include <iostream>
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <filesystem>

// Polecane aby użyć https://json.nlohmann.me/api/json/
using json = nlohmann::json;

// Globalna konfiguracja rate limitera
// max 5 tokenów, odnowienie 2.68 token na sekunde
static RateLimiter rate_limiter(3.0, 2.68);

// Tworzymy na nowo url z każdym nowym dniem
// Przyczyna: nie ma żadnego zapytania od LottoApi aby pobierać przez index
// Wymuszają aby było pobieranie ostatniego losowania, bądź z konkretnej daty
static std::string BuildUrl(const std::string& gameType, const std::string& drawDate) {
	return "https://developers.lotto.pl/api/open/v1/lotteries/draw-results/by-date-per-game" 
		"?gameType=" + gameType +
		"&drawDate=" + drawDate +
		"&index=1" "&size=100" "&sort=drawSystemId" "&order=ASC";
}

// Funkcja do parsowania według standardu: YYYY-MM-DD
// Poprzez referencje zmieniamy string na integer, bez dodatkowego zwracania
static void ParseDate(const std::string& date, int& year, int& month, int& day) {
	if (date.size() < 10) throw std::runtime_error("Zły format daty: " + date);
	year = std::stoi(date.substr(0, 4));
	month = std::stoi(date.substr(5, 2));
	day = std::stoi(date.substr(8, 2));
}

// Funckja do "cofnięcia" o 1 dzień od zapodanej daty
static std::string PrevDay(const std::string& date) {
	int year, month, day;
	ParseDate(date, year, month, day);

	day -= 1;
	if (day < 1) {
		// Jeśli prześliszmy wszystkie dni w danym miesiącu, resetujemy i odejmujemy w miesiącu oraz ewentulanie w roku, jeśli przekroczy zakres
		day = 31;
		month -= 1;
		if (month < 1) {
			month = 12;
			year -= 1; // Dla roku chyba nie trzeba, bo w teorii nigdy nie dojdziemy nawet 1900 roku
		}
	}

	// Ustawiamy szerokość na każdy element, jeśli cofniemy liczbę poniżej dziesiątek itd. 
	// To uzupełniamy zerami, a na końcu wpisujemy resztę daty, czyli godzinę
	// https://cplusplus.com/reference/sstream/ostringstream/str/
	// Aby uzyskać 2026-12-31T21:00:00Z
	std::ostringstream oss;
	oss << std::setw(4) << std::setfill('0') << year << "-"
		<< std::setw(2) << std::setfill('0') << month << "-"
		<< std::setw(2) << std::setfill('0') << day
		<< date.substr(10);
	return oss.str();
}

void DataManager::SaveDrawsToCsv(
	ApiClient& client,
	const std::string& gameType,
	const std::string& startDrawDate,
	int N,
	const std::string& outCsvPath) {
	
	std::ofstream out(outCsvPath);
	if (!out) throw std::runtime_error("Nie można otworzyć pliku CSV: " + outCsvPath);

	out << "drawSystemId,drawDate,gameType,n1,n2,n3,n4,n5,n6\n";

	// Tworzenie pliku do zapisu logów i błędów
	std::string temp = outCsvPath;
	const std::string prefix = "output/";
	const std::string domain = ".csv";
	if (temp.rfind(prefix, 0) == 0) temp.erase(0, prefix.size());
	if (temp.ends_with(domain)) temp.erase(temp.size() - domain.size());
	std::string outLogsPath = "output/logs/downloads/" + temp + ".log";

	std::error_code err;
	std::filesystem::path baseDir(outLogsPath);
	std::filesystem::path parent = baseDir.parent_path();
	if (!parent.empty()) {
		if (!std::filesystem::create_directories(parent, err) && err) {
			std::cout << "Błąd: Nie można utworzyć folderu dla logów pobierania: " << err.message() << "\n";
			std::cout << "Info: Owy problem nie stanowi przeszkodą do prawidłowego działania programu.\n";
		}
	}

	std::ofstream logs_error(outLogsPath);
	if (!logs_error) {
		std::cout << "Błąd: Nie można otworzyć pliku do zapisu logów i błędów pobierania: " << outLogsPath << "\n";
		std::cout << "Info: Owy problem nie stanowi przeszkodą do prawidłowego działania programu.\n";
	}

	std::string currentDate = startDrawDate;
	int found = 0;
	int daysChecked = 0;
	int safetyDaysLimit = 9999;

	int firstDrawSystemId = 4755;
	// Jeśli N == 0, to szukamy od podanej daty (najczęściej od najnowszego) do pierwszego losowania
	const bool infinitySearch = (N == 0);
	const int targetCount = N;
	// Wartość -1 w celu podkreślenia iż żadna wartość nie została zapisana
	int lastSavedDrawSystemId = -1;
	bool existPrevSavedId = false;
	int prevSavedId = -1;

	std::size_t total = 0;
	bool totalInitialized = false;

	// Estetyczna rzecz
	std::cout << "\n";
	
	// Pasek stanu pobrania oraz przybliżony czas oczekiwania
	// https://github.com/p-ranav/indicators
	indicators::show_console_cursor(false);
	indicators::ProgressBar bar {
		indicators::option::BarWidth{33},
		indicators::option::ShowPercentage{true},
		indicators::option::ShowElapsedTime{true},
		indicators::option::ShowRemainingTime{true},
		indicators::option::PrefixText{"Pobieranie wyników "}
	};

	while (daysChecked < safetyDaysLimit) {

		if (!infinitySearch && found >= targetCount) break;
		if (infinitySearch && lastSavedDrawSystemId != -1 && lastSavedDrawSystemId <= firstDrawSystemId) break;

		daysChecked++;

		const std::string url = BuildUrl(gameType, currentDate);

		// Rate Limiter sprawdzenie czy można wykonać żądanie do API,
		// jeśli nie, to czeka aż zostaną uzupełnione tokeny
		rate_limiter.Consume();

		try {
			json j = json::parse(client.GetResults(url));

			if (!j.contains("items") || !j["items"].is_array()) {
				currentDate = PrevDay(currentDate);
				continue;
			}

			bool printThisDay = false;

			for (const auto& item : j["items"]) {
				if (logs_error) logs_error << "Znaleziono - currentDate: " << currentDate << "\n";
				if (!item.contains("gameType")) continue;
				if (item["gameType"].get<std::string>() != gameType) continue;

				const long drawSystemId = item["drawSystemId"].get<long>();
				const std::string drawDate = item["drawDate"].get<std::string>();

				if (!item.contains("results") || !item["results"].is_array() || item["results"].empty()) continue;
				const auto& results0 = item["results"][0];
				if (!results0.contains("resultsJson") || !results0["resultsJson"].is_array()) continue;

				auto numbers = results0["resultsJson"].get<std::vector<int>>();
				std::sort(numbers.begin(), numbers.end());
				if (numbers.size() != 6) continue;

				out << drawSystemId << "," << drawDate << "," << gameType << ","
					<< numbers[0] << "," << numbers[1] << ","
					<< numbers[2] << "," << numbers[3] << ","
					<< numbers[4] << "," << numbers[5] << "\n";

				if (existPrevSavedId && drawSystemId != (prevSavedId - 1)) {
					if (logs_error) logs_error << "Pominięto losowanie! Poprzednie zapisane losowanie nr: "
							   << prevSavedId << " jest inne od obecnie pobranego o nr: " << drawSystemId << "\n";
				}
				existPrevSavedId = true;
				prevSavedId = drawSystemId;

				lastSavedDrawSystemId = drawSystemId;
				found++;
				printThisDay = true;

				if (!totalInitialized) {
					if (!infinitySearch) {
						// Aby nie dzielić przez zero
						total = (targetCount > 0) ? (std::size_t)targetCount : 1;
					} else {
						int difference = lastSavedDrawSystemId - firstDrawSystemId + 1;
						// Aby nie dzielić przez zero
						if (difference < 1) difference = 1;
						total = (std::size_t)difference;
					}
					totalInitialized = true;
				}

				// Procenty w zakresie 0-100
				if (totalInitialized && total > 0) {
					int percent = (total > 0) ? (int) (100.0 * found / (double)total) : 0;
					if (percent < 0) percent = 0;
					if (percent > 100) percent = 100;
					bar.set_progress(percent);
				}

				bar.set_option(indicators::option::PostfixText{
					"ID: " + std::to_string(lastSavedDrawSystemId) + ", Data: " + currentDate 
				});

				if (logs_error) logs_error << "Aktualny zapisany ID: " << drawSystemId << "," << drawDate << "\n";

				break;
			}

			if (!printThisDay) bar.set_option(indicators::option::PostfixText{ "Brak dla: " + currentDate });

		} catch (const std::exception& e) {
			if (logs_error) logs_error << "Pominieto - currentDate: " << currentDate << " ";
			std::string error_msg = e.what();
		
			// Jeśli będzie HTTP Error 429 - too many request 
			// Robimy przerwe w żądaniach
			if (error_msg.find("429") != std::string::npos) {
				if (logs_error) logs_error << "HTTP Error 429 - waiting..." << "\n";
				rate_limiter.WaitForRateLimitReset();
				daysChecked--;
				continue;
			}

			if (error_msg.find("404") == std::string::npos && logs_error) logs_error << "#ERROR! " << e.what() << " ";
			if (logs_error) logs_error << "\n";
		}

		currentDate = PrevDay(currentDate);
	}

	indicators::show_console_cursor(true);

	if (logs_error && found == 0) {
		logs_error << "Nie udało się pobrać żadnych losowań :(";
		throw std::runtime_error("Nie udało się pobrać żadnych losowań :(");
	}
	std::cout << "\nZnaleziono " << found << " losowań\n";
	if (logs_error) logs_error << "Znaleziono " << found << " losowań\n";
	std::cout << "\nZakończono pomyślnie pobieranie losowań.\nZapisano do pliku: " << outCsvPath << "\n";
}

