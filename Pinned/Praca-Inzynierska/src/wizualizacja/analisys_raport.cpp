#include "analisys_raport.h"
#include "version.h"

#include <iostream>
#include <vector>
#include <array>
#include <sstream>
#include <fstream>
#include <string>
#include <algorithm>
#include <optional>
#include <charconv>
#include <iomanip>
#include <filesystem>


static std::vector <std::string> SplitLine(const std::string line) {
	std::vector <std::string> columns;
	std::string field;
	std::stringstream ss(line);
	while (std::getline(ss, field, ',')) columns.push_back(field);
	return columns;
}

static bool StartsWith(const std::string& s, const std::string& prefix) {
	if (prefix.size() > s.size()) return false;
	return std::equal(prefix.begin(), prefix.end(), s.begin());
}

static std::optional <int> TryParseInt(std::string_view sv) {
	int value = 0;
	const auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), value);
	if (ec == std::errc{} && ptr == sv.data() + sv.size()) {
		return value;
	}
	return std::nullopt; // Niepowodzenie: zły format, przepełnienie lub niedopełniony parse
}

static std::optional <long long> TryParseLongLong(std::string_view sv) {
	long long value = 0;
	const auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), value);
	if (ec == std::errc{} && ptr == sv.data() + sv.size()) {
		return value;
	}
	return std::nullopt; // Niepowodzenie: zły format, przepełnienie lub niedopełniony parse
}

static void Trim(std::string& s) {
	while (!s.empty() && (s.back() == '\r' || s.back() == '\n' || s.back() == ' ' || s.back() == '\t')) {
		s.pop_back();
	}
}

static GroupFileData ReadGroupFile(std::ofstream& logs_error, const std::string& inCsvPathFinal, const std::string& outHtmlPath) {
	std::ifstream inFile(inCsvPathFinal);
	if (!inFile) throw std::runtime_error("Nie można otworzyć pliku inCSV: " + inCsvPathFinal);

	GroupFileData data;
	std::string line;
	std::size_t lineIndex = 0;

	bool foundHeader = false;
	while (std::getline(inFile, line)) {
		lineIndex++;
		if (line.empty()) continue;

		if (StartsWith(line, "groupOfNumbers")) {
			size_t equalSignPosition = line.find('=');
			if (equalSignPosition == std::string::npos) break;
			auto parsed = TryParseInt(line.substr(equalSignPosition + 2));
			if (parsed) {
				data.groupOfNumbers = *parsed;
				continue;
			} else {
				throw std::runtime_error("Nie można przekonwertować wartości 'groupOfNumbers' na liczbę całkowitą");
			}
		}
		if (StartsWith(line, "totalNumberOfDraws")) {
			size_t equalSignPosition = line.find('=');
			if (equalSignPosition == std::string::npos) break;
			auto parsed = TryParseInt(line.substr(equalSignPosition + 2));
			if (parsed) {
				data.totalNumberOfDraws = *parsed;
				continue;
			}
			else {
				throw std::runtime_error("Nie można przekonwertować wartości 'totalNumberOfDraws' na liczbę całkowitą");
			}
		}
		if (StartsWith(line, "totalNumberOfNumbers")) {
			size_t equalSignPosition = line.find('=');
			if (equalSignPosition == std::string::npos) break;
			auto parsed = TryParseLongLong(line.substr(equalSignPosition + 2));
			if (parsed) {
				data.totalNumberOfNumbers = *parsed;
				continue;
			}
			else {
				throw std::runtime_error("Nie można przekonwertować wartości 'totalNumberOfNumbers' na liczbę całkowitą");
			}
		}
		if (StartsWith(line, "numbers,count,probHistory,probTheory,expected,share")) {
			foundHeader = true;
			break;
		}
	}

	if (!foundHeader) throw std::runtime_error("Nie znaleziono nagłówka CSV w pliku: " + inCsvPathFinal);

	while (std::getline(inFile, line)) {
		lineIndex++;
		if (line.empty()) continue;

		auto columns = SplitLine(line);
		if (columns.size() < 6) {
			if (logs_error) logs_error << "Możliwie nie znalazł 6 kolumn danych oddzielonych przecinkiem. Numer linii: " 
				<< lineIndex << ", liczba kolumn: " << columns.size() << ", treść linii" << line << "\n";
			continue;
		}

		for (auto& c : columns) Trim(c);

		Row row;
		row.numbers = columns[0];

		// Próbujemy pobrać dane z csv, jeśli nie uda się zwyczajnie pomijamy
		// Może zdarzyć się że były jakieś ala "śmieci" w danych
		try {
			row.count = std::stoll(columns[1]);
			row.probHistory = std::stod(columns[2]);
			row.probTheory = std::stod(columns[3]);
			row.expected = std::stod(columns[4]);
			row.share = std::stod(columns[5]);

		} catch (const std::exception& e) {
			if (logs_error) logs_error << "Wystąpił błąd podczas zmiany 'string' na 'double' w linii: "
				<< lineIndex << ", liczba kolumn: " << columns.size() << ", treść linii" << line 
				<< ", zawartość poszczególnych kolumn: K1=" << columns[1] << ", K2=" << columns[2] << ", K3=" << columns[3] 
				<< ", K4=" << columns[4] << ", K5=" << columns[5] << ", powód błędu: " << e.what() << "\n";
			continue;
		} catch (...) {
			if (logs_error) logs_error << "Wystąpił nieznany błąd podczas zmiany 'string' na 'double' w linii: "
				<< lineIndex << ", liczba kolumn: " << columns.size() << ", treść linii" << line
				<< ", zawartość poszczególnych kolumn: K1=" << columns[1] << ", K2=" << columns[2] << ", K3=" << columns[3]
				<< ", K4=" << columns[4] << ", K5=" << columns[5] << "\n";
			continue;
		}

		data.rows.push_back(std::move(row));
	}

	return data;
}

static std::string TodayDate() {
	using namespace std::chrono;

	zoned_time zt_now{ current_zone(), system_clock::now() };
	auto localDate = floor<days>(zt_now.get_local_time());
	year_month_day date{ localDate };

	const char* monthPL[] = {
		"stycznia", "lutego", "marca", "kwietnia", "maja", "czerwca",
		"lipca", "sierpnia", "września", "października", "listopada", "grudnia"
	};

	const unsigned day = static_cast <unsigned> (date.day());
	const unsigned month = static_cast <unsigned> (date.month());
	const int year = static_cast <int> (date.year());

	return std::to_string(day) + " " + monthPL[month - 1] + " " + std::to_string(year) + "r.";
}


static void WriteHtmlFooter(std::ofstream& outFile) {
	const std::string footerDate = TodayDate();

	outFile << R"(
			</section>
		
			<aside class="sideColumn">
				<section class="panel sticky">
					<h2>Nawigacja</h2>
					<div class="toc">
						<div class="divider"></div>
						<a href="#top6" class="tocMain">Top 6</a>
						<a href="#chiSquare" class="tocMain">Test chi-kwadrat</a>
						<a href="#ks" class="tocMain">Test Kołmogorowa-Smirnowa</a>
						<a href="#all" class="tocMain">Pełna tabela</a>
						<div class="divider"></div>
						<a href="#grupa1">Pojedyncze liczby</a>
						<a href="#grupa2">Pary</a>
						<a href="#grupa3">Trójki</a>
						<a href="#grupa4">Czwórki</a>
						<a href="#grupa5">Piątki</a>
						<a href="#grupa6">Szóstki</a>
					</div>
				</section>
			</aside>
		</main>
	</div>

	<div class="footer">
		<div class="footerContent">
			<span>Wygenerowano z programu autorstwa: <strong>Jakub Ćwiek</strong></span>
			<span>▪︎</span>
			<span>Wersja programu: <strong>)" << infoSystemVersion << R"( </strong></span>
			<span>▪︎</span>
			<span>Data wygenerowania: <strong>)" << footerDate << R"(</strong></span>
		</div>
	</div>
</body>
</html>
)";
}

static void WriteTopTable(std::ofstream& outFile,
	const std::string& title, const std::vector <Row>& rows) {
	outFile << "\t\t\t<div class=\"card\">\n";
	outFile << "\t\t\t\t<h3>" << title << "</h3>\n";
	outFile << "\t\t\t\t<table>\n";
	outFile << "\t\t\t\t\t<thead>\n";
	outFile << "\t\t\t\t\t\t<tr>"
			<< "<th>Liczby</th>"
			<< "<th>Liczebność</th>"
			<< "<th>Prawdopobieństwo</th></tr>\n";
	outFile << "\t\t\t\t\t</thead>\n";
	outFile << "\t\t\t\t\t<tbody>\n";

	for (const auto& row : rows) {
		outFile << "\t\t\t\t\t\t<tr>"
				<< "<td>" << row.numbers << "</td>"
				<< "<td>" << row.count << "</td>"
				<< "<td>" << std::fixed << std::setprecision(4) << row.probHistory * 100 << "%" << "</td>"
				<< "</tr>\n";
	}

	outFile << "\t\t\t\t\t</tbody>\n";
	outFile << "\t\t\t\t</table>\n";
	outFile << "\t\t\t</div>\n";
}

static void WriteTopSection(std::ofstream& outFile,
	const std::array <GroupFileData, 7>& arr) {

	const char* groupNames[] = {
		"", // Nieużyty index 0
		"Pojedyncze liczby",
		"Pary",
		"Trójki",
		"Czwórki",
		"Piątki",
		"Szóstki"
	};

	outFile << "\n\t\t<div class=\"panel\" id=\"top6\">\n";
	outFile << "\t\t\t<div class=\"panelHead\">\n";
	outFile << "\t\t\t\t<h2>Top 6 - szybki podgląd najczęstszych i najrzadszych grup liczb</h2>\n";
	outFile << "\t\t\t</div>\n";
	outFile << "\t\t\t<div class=\"cards\">\n";
	//outFile << "\t\t\t\t<p class=\"subText\">Całkowita liczba losowań: " << arr[1].totalNumberOfDraws << "</p>\n";
	//outFile << "\t\t\t\t<p class=\"subText\">Całkowita liczba przeanalizowanych liczb: " << arr[1].totalNumberOfNumbers << "</p>\n";

	for (int i = 1; i <= 6; i++) {
		const auto& data = arr[i];
		std::vector <Row> sorted = data.rows;
		// Sortujemy wyniki używając "projection" aby wiedzieć po czym sortuje
		// https://en.cppreference.com/w/cpp/algorithm/ranges/sort.html
		// https://en.cppreference.com/w/cpp/utility/functional/less.html
		std::ranges::sort(sorted, std::greater <> {}, &Row::count);
		std::vector <Row> topMost;
		for (int i = 0; i < (int) sorted.size() && i < 6; i++) topMost.push_back(sorted[i]);

		std::ranges::sort(sorted, std::less <> {}, &Row::count);
		std::vector <Row> topLeast;
		for (int i = 0; i < (int)sorted.size() && i < 6; i++) topLeast.push_back(sorted[i]);

		WriteTopTable(outFile, "Top 6 najczęstszych: " + std::string(groupNames[i]), topMost);
		WriteTopTable(outFile, "Top 6 najrzadszych: " + std::string(groupNames[i]), topLeast);
	}

	outFile << "\t\t\t</div>\n";
	outFile << "\t\t</div>\n";
}

static void WriteFullTable(std::ofstream& outFile,
	const GroupFileData& data) {

	const char* groupNames[] = {
		"", // Nieużyty index 0
		"Pojedyncze liczby",
		"Pary",
		"Trójki",
		"Czwórki",
		"Piątki",
		"Szóstki"
	};

	outFile << "\t\t\t<details class=\"accordion\" id=\"grupa" << data.groupOfNumbers << "\">\n"
			<< "\t\t\t\t<summary>\n"
			<< "\t\t\t\t\t<span>Grupa " << groupNames[data.groupOfNumbers] << "</span>\n"
			<< "\t\t\t\t</summary>\n"

			<< "\t\t\t\t<div class=\"tableWrap\">\n"
			<< "\t\t\t\t\t<table>\n"
			<< "\t\t\t\t\t\t<thead>\n"
			<< "\t\t\t\t\t\t\t<tr>\n"
			<< "\t\t\t\t\t\t\t\t<th>Grupa liczb (liczebność)</th>\n"
			<< "\t\t\t\t\t\t\t\t<th>Prawd. historyczne</th>\n"
			<< "\t\t\t\t\t\t\t\t<th>Prawd. teorytyczne</th>\n"
			<< "\t\t\t\t\t\t\t\t<th>Wartość oczekiwana</th>\n"
			<< "\t\t\t\t\t\t\t\t<th>Udział w grupie</th>\n"
			<< "\t\t\t\t\t\t\t</tr>\n"
			<< "\t\t\t\t\t\t</thead>\n"
			<< "\t\t\t\t\t\t<tbody>\n";

	std::vector <Row> sorted = data.rows;
	std::ranges::sort(sorted, std::greater <> {}, &Row::count);

	for (const auto& row : sorted) {
		outFile << "\t\t\t\t\t\t<tr>"
				<< "<td>" << row.numbers << " (" << row.count << ")</td>"
				<< "<td>" << std::fixed << std::setprecision(6) << row.probHistory * 100<< "%" << "</td>"
				<< "<td>" << std::fixed << std::setprecision(6) << row.probTheory * 100 << "%" << "</td>"
				<< "<td>" << std::fixed << std::setprecision(6) << row.expected << "</td>"
				<< "<td>" << std::fixed << std::setprecision(6) << row.share * 100 << "%" << "</td>"
				<< "</tr>\n";
	}

	outFile << "\t\t\t\t\t\t</tbody>\n"
			<< "\t\t\t\t\t</table>\n"
			<< "\t\t\t\t</div>\n"
			<< "\t\t\t</details>\n";
}

static void WriteFullSection(std::ofstream& outFile,
	const std::array <GroupFileData, 7>& arr) {
	outFile << "\n\t\t<section class=\"panel\" id=\"all\">\n";
	outFile << "\t\t\t<div class=\"panelHead\">\n";
	outFile << "\t\t\t\t<h2>Pełne zestawienie - wersja szczegółowa</h2>\n";
	outFile << "\t\t\t</div>\n";

	outFile << "\t\t\t<div class=\"description\">\n"
			<< "\t\t\t\t<strong>Opis kolumn: </strong><br>\n"
			<< "\t\t\t\t<strong>Grupa liczb (liczność)</strong> - konkretna para/trójka itd. oraz ile razy owa grupa padła w całej bazie.<br>\n"
			<< "\t\t\t\t<strong>Prawdopodobieństwo historyczne</strong> - jak często dana grupa pojawiła się w losowaniach w pobranej bazie.<br>\n"
			<< "\t\t\t\t<strong>Prawdopodobieństwo teorytyczne</strong> - jaka jest szansa na konkretną grupę liczb (wynika to z rozkładu hipergeometrycznego)<br>\n"
			<< "\t\t\t\t<strong>Wartość oczekiwana</strong> - ile razy taki układ powinien średnio wypaść w pobranej bazie (liczba losowań * prawdopodobieństwo teorytyczne)<br>\n"
			<< "\t\t\t\t<strong>Udział w grupie</strong> - jaki dokładnie procent wszystkich par/trójek itd. zebranych w bazie stanowi akurat ta konkretna grupa liczb \n"
			<< "\t\t\t\t(liczba wszystkich liczb / (liczba losowań * P(6, n))) <br>\n"
			<< "\t\t\t</div>\n";


	for (int i = 1; i <= 6; i++) {
		WriteFullTable(outFile, arr[i]);
	}

	outFile << "\t\t</section>\n";
}

static void WriteChiSquareSection(std::ofstream& logs_error, const std::string& inTxtPathFinal, std::ofstream& outFile) {
	std::ifstream inFile(inTxtPathFinal);
	if (!inFile) throw std::runtime_error("Nie można otworzyć pliku inTxt: " + inTxtPathFinal);

	bool isValid = false;
	std::string h0;
	std::string decisionRule;
	std::string alpha;
	std::string df;
	std::string criticalValue;
	std::string chiSqaureValue;
	std::string verdict;

	std::string line;
	while (std::getline(inFile, line)) {
		Trim(line);
		if (line.empty()) continue;

		if (StartsWith(line, "Test chi-square = ")) {
			if (line.find("nieprawidłowa wartość oczekiwana") != std::string::npos) {
				isValid = false;
				break;
			}
			if (line.find("brak danych") != std::string::npos) {
				isValid = false;
				break;
			}
		}

		if (StartsWith(line, "Test Chi-Square")) {
			isValid = true;
		} else if (StartsWith(line, "H0:")) {
			h0 = line.substr(std::string("H0:").size());
		} else if (StartsWith(line, "Reguła decyzji:")) {
			decisionRule = line.substr(std::string("Reguła decyzji:").size());
		} else if (StartsWith(line, "Poziom istotności: alpha =")) {
			alpha = line.substr(std::string("Poziom istotności: alpha =").size());
		} else if (StartsWith(line, "Stopień swobody: df =")) {
			df = line.substr(std::string("Stopień swobody: df =").size());
		} else if (StartsWith(line, "Wartość krytyczna =")) {
			criticalValue = line.substr(std::string("Wartość krytyczna =").size());
		} else if (StartsWith(line, "Wynik sumy chiSquare =")) {
			chiSqaureValue = line.substr(std::string("Wynik sumy chiSquare =").size());
		} else if (StartsWith(line, "Werdykt:")) {
			verdict = line.substr(std::string("Werdykt:").size());
		}
	}

	outFile << "\n\t\t<section class=\"panel\" id=\"chiSquare\">\n";
	outFile << "\t\t\t<div class=\"panelHead\">\n";
	outFile << "\t\t\t\t<h2>Test Chi-Kwadrat</h2>\n";
	outFile << "\t\t\t</div>\n";

	if (!isValid) {
		outFile << "\t\t\t<p>Nie znaleziono prawidłowego pliku z wynikiem analizy testu chi-kwadrat!</p>\n";
		outFile << "\t\t</section>\n";
		return;
	}


	outFile << "\t\t\t<div class=\"description\">\n"
			<< "\t\t\t\t<strong>H0:</strong> " << h0 << "<br>\n"
			<< "\t\t\t\t<strong>Reguła decyzji:</strong> " << decisionRule << "<br>\n"
			<< "\t\t\t\t<strong>Poziom istotności: alpha =</strong> " << alpha << "<br>\n"
			<< "\t\t\t\t<strong>Stopień swobody: df =</strong> " << df << "<br>\n"
			<< "\t\t\t\t<strong>Wartość krytyczna =</strong> " << criticalValue << "<br>\n"
			<< "\t\t\t\t<strong>Wynik sumy chiSquare =</strong> " << chiSqaureValue << "<br>\n"
			<< "\t\t\t\t<br>\n"
			<< "\t\t\t\t<strong>Werdykt: </strong> " << verdict << "<br>\n"
			<< "\t\t\t</div>\n"
			<< "\t\t</section>\n";
}

static void WriteKolmogorowaSmirnowaSection(std::ofstream& logs_error, const std::string& inCsvPathFinal, std::ofstream& outFile) {
	std::ifstream inFile(inCsvPathFinal);
	if (!inFile) throw std::runtime_error("Nie można otworzyć pliku inTxt: " + inCsvPathFinal);

	bool isValid = false;
	bool noDataCsv = false;
	std::string h0;
	std::string decisionRule;
	std::string alpha;
	std::string criticalValue;
	std::string totalNumbers;
	std::string maxDistance;
	std::string atNumber;
	std::string verdict;

	struct ksRow {
		int number = 0;
		long long count = 0;
		long long sum = 0;
		double DE = 0.0;
		double DT = 0.0;
		double difference = 0.0;
	};

	std::vector <ksRow> rows;
	
	bool csvPart = false;
	std::string line;
	std::size_t lineIndex = 0;
	while (std::getline(inFile, line)) {
		lineIndex++;
		Trim(line);
		if (line.empty()) continue;

		if (line == "Test Kolmogorowa-Smirnowa = brak danych") {
			isValid = false;
			break;
		}

		if (!csvPart) {
			if (StartsWith(line, "Test Kolmogorowa-Smirnowa")) {
				isValid = true;
			} else if (StartsWith(line, "H0:")) {
				h0 = line.substr(std::string("H0:").size());
			} else if (StartsWith(line, "Reguła decyzji:")) {
				decisionRule = line.substr(std::string("Reguła decyzji:").size());
			} else if (StartsWith(line, "Poziom istotności: alpha =")) {
				alpha = line.substr(std::string("Poziom istotności: alpha =").size());
			} else if (StartsWith(line, "Wartość krytyczna =")) {
				criticalValue = line.substr(std::string("Wartość krytyczna =").size());
			} else if (StartsWith(line, "Ilość wylosowanych liczb = ")) {
				totalNumbers = line.substr(std::string("Ilość wylosowanych liczb = ").size());
			} else if (StartsWith(line, "Największa odległość =")) {
				maxDistance = line.substr(std::string("Największa odległość =").size());
			} else if (StartsWith(line, "Przy liczbie:")) {
				atNumber = line.substr(std::string("Przy liczbie:").size());
			} else if (StartsWith(line, "Werdykt:")) {
				verdict = line.substr(std::string("Werdykt:").size());
			} else if (StartsWith(line, "Liczba,Wystąpienia,Skumulowane,Empiryczna,Teoretyczna,Różnica")) {
				csvPart = true;
			}
			continue;
		}

		auto columns = SplitLine(line);
		if (columns.size() < 6) {
			if (logs_error) logs_error << "Możliwie nie znalazł 6 kolumn danych oddzielonych przecinkiem. Numer linii: "
				<< lineIndex << ", liczba kolumn: " << columns.size() << ", treść linii" << line << "\n";
			continue;
		}

		for (auto& c : columns) Trim(c);

		try {
			ksRow row;
			row.number = std::stoi(columns[0]);
			row.count = std::stoll(columns[1]);
			row.sum = std::stoll(columns[2]);
			row.DE = std::stod(columns[3]);
			row.DT = std::stod(columns[4]);
			row.difference = std::stod(columns[5]);
			rows.push_back(std::move(row));
		}
		catch (const std::exception& e) {
			if (logs_error) logs_error << "Wystąpił błąd podczas zmiany 'string' na 'double' w linii: "
				<< lineIndex << ", liczba kolumn: " << columns.size() << ", treść linii" << line
				<< ", zawartość poszczególnych kolumn: K0=" << columns[0] << "K1 = " << columns[1] << ", K2 = " << columns[2] << ", K3 = " << columns[3]
				<< ", K4=" << columns[4] << ", K5=" << columns[5] << ", powód błędu: " << e.what() << "\n";
			continue;
		}
		catch (...) {
			if (logs_error) logs_error << "Wystąpił nieznany błąd podczas zmiany 'string' na 'double' w linii: "
				<< lineIndex << ", liczba kolumn: " << columns.size() << ", treść linii" << line
				<< ", zawartość poszczególnych kolumn: K0=" << columns[0] << "K1=" << columns[1] << ", K2=" << columns[2] << ", K3=" << columns[3]
				<< ", K4=" << columns[4] << ", K5=" << columns[5] << "\n";
			continue;
		}
	}

	outFile << "\n\t\t<section class=\"panel\" id=\"ks\">\n";
	outFile << "\t\t\t<div class=\"panelHead\">\n";
	outFile << "\t\t\t\t<h2>Test Kołmogorowa-Smirnowa</h2>\n";
	outFile << "\t\t\t</div>\n";

	if (!isValid) {
		outFile << "\t\t\t<p>Nie znaleziono prawidłowego pliku z wynikiem analizy testu kołmogorowa-smirnowa!</p>\n";
		outFile << "\t\t</section>\n";
		return;
	}


	outFile << "\t\t\t<div class=\"cards ksCards\">\n"
			<< "\t\t\t\t<div class=\"card\">\n"
			<< "\t\t\t\t\t<div class=\"description\">\n"
			<< "\t\t\t\t\t\t<strong>H0:</strong> " << h0 << "<br>\n"
			<< "\t\t\t\t\t\t<strong>Reguła decyzji:</strong> " << decisionRule << "<br>\n"
			<< "\t\t\t\t\t\t<strong>Poziom istotności: alpha =</strong> " << alpha << "<br>\n"
			<< "\t\t\t\t\t\t<strong>Wartość krytyczna =</strong> " << criticalValue << "<br>\n"
			<< "\t\t\t\t\t\t<strong>Ilość wylosowanych liczb =</strong> " << totalNumbers << "<br>\n"
			<< "\t\t\t\t\t\t<strong>Największa odległość =</strong> " << maxDistance << "<br>\n"
			<< "\t\t\t\t\t\t<strong>Przy liczbie:</strong> " << atNumber << "<br>\n"
			<< "\t\t\t\t\t\t<br>\n"
			<< "\t\t\t\t\t\t<strong>Werdykt: </strong> " << verdict << "<br>\n"
			<< "\t\t\t\t\t</div>\n"
			<< "\t\t\t\t</div>\n";

	if (!rows.empty()) {
		outFile << "\t\t\t\t<div class=\"card\">\n";
		outFile << "\t\t\t\t\t<div class=\"tableWrap\">\n";
		outFile << "\t\t\t\t\t\t<table>\n";
		outFile << "\t\t\t\t\t\t\t<thead>\n";
		outFile << "\t\t\t\t\t\t\t\t<tr>"
				<< "<th>Liczba</th>"
				<< "<th>Wystąpienia</th>"
				<< "<th>Skumulowane</th>"
				<< "<th>Empiryczna</th>"
				<< "<th>Teoretyczna</th>"
				<< "<th>Różnica</th></tr>\n";
		outFile << "\t\t\t\t\t\t\t</thead>\n";
		outFile << "\t\t\t\t\t\t\t<tbody>\n";

		for (const auto& row : rows) {
			outFile << "\t\t\t\t\t\t\t\t<tr>"
				<< "<td>" << row.number << "</td>"
				<< "<td>" << row.count << "</td>"
				<< "<td>" << row.sum << "</td>"
				<< "<td>" << std::fixed << std::setprecision(6) << row.DE << "</td>"
				<< "<td>" << std::fixed << std::setprecision(6) << row.DT << "</td>"
				<< "<td>" << std::fixed << std::setprecision(6) << row.difference << "</td>"
				<< "</tr>\n";
		}

		outFile << "\t\t\t\t\t\t\t</tbody>\n";
		outFile << "\t\t\t\t\t\t</table>\n";
		outFile << "\t\t\t\t\t</div>\n";
		outFile << "\t\t\t\t</div>\n";
	}
	
	outFile	<< "\t\t\t</div>\n";
	outFile	<< "\t\t</section>\n";
}

void Raport::AnalisysToHtml(const std::string& inDirPath, const std::string& outHtmlPath, const std::string& title) {
	std::ofstream outFile(outHtmlPath);
	if (!outFile) throw std::runtime_error("Nie można otworzyć pliku outHTML: " + outHtmlPath);

	WriteHtmlHeader(outFile, title);

	std::array <GroupFileData, 7> arr{};

	// Tworzenie pliku do zapisu logów i błędów
	std::string temp = outHtmlPath;
	const std::string prefix = "output/";
	const std::string domain = ".html";
	if (temp.rfind(prefix, 0) == 0) temp.erase(0, prefix.size());
	if (temp.ends_with(domain)) temp.erase(temp.size() - domain.size());
	std::string outLogsPath = "output/logs/raport/" + temp + ".log";

	std::error_code err;
	std::filesystem::path baseDir(outLogsPath);
	std::filesystem::path parent = baseDir.parent_path();
	if (!parent.empty()) {
		if (!std::filesystem::create_directories(parent, err) && err) {
			std::cout << "Błąd: Nie można utworzyć folderu dla logów raportu: " << err.message() << "\n";
			std::cout << "Info: Owy problem nie stanowi przeszkodą do prawidłowego działania programu.\n";
		}
	}

	std::ofstream logs_error(outLogsPath);
	if (!logs_error) {
		std::cout << "Błąd: Nie można otworzyć pliku do zapisu logów i błędów raportu: " << outLogsPath << "\n";
		std::cout << "Info: Owy problem nie stanowi przeszkodą do prawidłowego działania programu.\n";
	}

	// Zapis 6 tabel dla każdej kombinacji grup liczb 1-6
	for (int i = 1; i <= 6; i++) {
		std::string inCsvPathFinal = inDirPath + "/analisys_group_" + std::to_string(i) + ".csv";
		arr[i] = ReadGroupFile(logs_error, inCsvPathFinal, outHtmlPath);
	}

	WriteTopSection(outFile, arr);

	const std::string chiPath = inDirPath + "/analisys_test_chi_square.txt";
	const std::string ksPath = inDirPath + "/analisys_test_K_S.csv";
	WriteChiSquareSection(logs_error, chiPath, outFile);
	WriteKolmogorowaSmirnowaSection(logs_error, ksPath, outFile);

	WriteFullSection(outFile, arr);

	WriteHtmlFooter(outFile);

	std::cout << "\nZakończono pomyślnie generowanie raportu jako strony internetowej.\nZapisano w pliku: " << outHtmlPath << "\n";
}