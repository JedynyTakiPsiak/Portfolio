#include "analisys.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <optional>
#include <charconv>
#include <string_view>
#include <system_error>
#include <bit>
#include <algorithm>
#include <filesystem>

// Dzielimy linie na "kolumny" danych słów, które są oddzielone przecinkiem
// https://stackoverflow.com/questions/52689845/split-comma-separated-string
static std::vector <std::string> SplitLine(const std::string line) {
	std::vector <std::string> columns;
	std::string field;
	std::stringstream ss(line);
	while (std::getline(ss, field, ',')) columns.push_back(field);
	return columns;
}

// Funkcja do zamiany łańcucha znaków na liczbę całkowitą
// https://cpp-polska.pl/szybkie-konwersje-lancuchow-znakow-na-liczby-z-std-from-chars/
static std::optional <int> TryParseInt(std::string_view sv) {
	int value = 0;
	const auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), value);
	if (ec == std::errc{} && ptr == sv.data() + sv.size()) {
		return value;
	}
	return std::nullopt; // Niepowodzenie: zły format, przepełnienie lub niedopełniony parse
}

// Tworzymy maskę bitową w której ustawiamy dokładnie tylko jeden bit używając przesunięcia bitowego w lewo, otrzymując bit na pozycji n-1
// https://medium.com/@ramy.sameh.radwan/understanding-1-n-1-a-fundamental-bit-mask-pattern-d9e5d18c3f5a
static std::uint64_t BitOfNumber(int n) {
	// liczba n w zakresie 1-49
	return (std::uint64_t(1) << (n - 1));
}

// Logika zliczania wszystkich kombinacji grup liczb 1-6 jest oparta na
// generowaniu podzbiorów bitmaską oraz użyciu zliczania przy uzyciu popcount
// który zlicza wartości 1 na danych bitach i zwraca wielkość podzbioru
// https://aryansh.gitbook.io/informatics-notes/misc-tricks/bitmask-subsets
// https://stackoverflow.com/questions/7659503/regarding-bit-masking-in-c-why-0-n-is-preferred-than-1-n-1
// Z racji że w każdym losowaniu mamy:
// 6 jedynek, 15 dwójek, 20 trójek, 15 czwórek, 6 piątek i 1 szóstkę, co daje razem 63 kombinacji różnych
// Używając właśnie maski 2^6-1 daje nam tą możliwość przejścia przez wszystkie kombinacje
static void CountAllGroups(const int numbers[6], GroupNumberStats& stats) {
	for (unsigned subset = 1; subset < 64u; subset++) {
		int n = std::popcount(subset);
		std::uint64_t mask = 0;

		for (int i = 0; i < 6; i++) {
			if (subset & (1u << i)) {
				mask |= BitOfNumber(numbers[i]);
			}
		}
		stats.count[n][mask] += 1;
		//std::cout << "Debug mask: " << mask << "\n";
	}
}

// Konwert maski na stringa w zapisie "X1-X2-X3..."
// W celu nie zepsucia pliku csv, w którym mamy przecinki
static std::string MaskToString(std::uint64_t mask) {
	std::ostringstream oss;
	bool firstValue = true;

	for (int i = 1; i <= 49; i++) {
		if (mask & BitOfNumber(i)) {
			if (!firstValue) oss << "-";
			oss << i;
			firstValue = false;
		}
	}

	return oss.str();
}

// Wzór dwumianu newtona
// https://cp-algorithms.com/combinatorics/binomial-coefficients.html
// https://www.geeksforgeeks.org/dsa/binomial-coefficient-dp-9/
static double BinomialCoefficients(int n, int k) {
	if (k < 0 || k > n) return 0.0;
	k = std::min(k, n - k);
	double res = 1.0;
	for (int i = 1; i <= k; i++) {
		res = res * (n - k + i) / i;
	}
	return res;
}

// Rozkład hipergeometryczny dla opisania losowania bez zwracania ze skończonej populacji
// Przez fakt iż sprawdzamy czy konkretna para występuje w losowaniu, dlatego możemy ukrucić pierwszą część wzoru, z racji (2,2) == 1
// https://home.agh.edu.pl/~mariuszp/wfiis_stat/wyklad_stat_7_1011.pdf
static double ProbDrawContainsFixedSubset(int fixedSubsetSize, int populationSize = 49, int drawSize = 6) {
	return BinomialCoefficients(populationSize - fixedSubsetSize, drawSize - fixedSubsetSize) / BinomialCoefficients(populationSize, drawSize);
}

// Analizę testu Chi-kwadrat robimy jedynie dla grupy jedno liczbowych / jedynek, 
// Z racji iż mamy tam liczbę ponad min. 100 wyników na każdą wylosowaną liczbę,
// i nigdzie nie ma sytuacji jak w parach, trójkach itd. iż jakaś kombinacja nie wystąpiła bądź wystąpiła mniej niż te minimum 5 razy,
// które dla owego testu jest bardzo wymagane aby uzyskać prawidłowe wyniki
static void AnalisysTestChiSquare(const GroupNumberStats& stats, const std::string& outCsvPath) {
	std::string outCsvPathFinal = std::string(outCsvPath) + std::string("/analisys_test_chi_square.txt");
	std::ofstream outFile(outCsvPathFinal);
	if (!outFile) throw std::runtime_error("Nie można otworzyć pliku outCSV: " + outCsvPathFinal);

	outFile << "Test Chi-Square\n"
		<< "H0: liczby występują z jednakową częstotliwością, zgodnie z rozkładem jednostajnym\n"
		<< "Reguła decyzji: odrzucamy H0, jeśli suma chiSquare będzie większa niż wartość krytyczna\n"
		<< "Poziom istotności: alpha = 0.05\n"
		<< "Stopień swobody: df = 48\n"
		<< "Wartość krytyczna = 65.171\n";

	// Brak wylosowanych danych 
	if (stats.draws <= 0) {
		outFile << "Test chi-square = brak danych";
		return;
	}

	// Wartości mamy stałe z racji iż robimy tylko dla jedynek
	const double probTheory = ProbDrawContainsFixedSubset(1);
	const double expected = (double)stats.draws * probTheory;

	if (expected <= 0.0) {
		outFile << "Test chi-square = nieprawidłowa wartość oczekiwana\n";
		return;
	}

	// Wykonujemy teraz sumę: (wartość obserwowana - wartość oczekiwana)^2 / wartość oczekiwana
	double chiSquare = 0.0;

	for (int i = 1; i <= 49; i++) {
		const std::uint64_t mask = BitOfNumber(i);
		auto number = stats.count[1].find(mask);
		double observed;
		// Jeśli nie znajdzie wartości to ustawiamy na 0.0, aby nie wywowałać błędnej operacji
		if (number != stats.count[1].end()) {
			observed = static_cast <double> (number->second);
		}
		else {
			observed = 0.0;
		}
		const double difference = observed - expected;
		chiSquare += (difference * difference) / expected;
	}

	outFile << "Wynik sumy chiSquare = " << chiSquare << "\n\n";

	// Według tabeli dla df = 48 i alpha = 0.05 otrzymamy owe 65.171
	// "Critical Values for Chi-Square Distribution."
	// https://www.stat.purdue.edu/~lfindsen/stat503/Chi-Square.pdf
	const double chiSquareCritical = 65.171;
	const bool rejectH0 = (chiSquare > chiSquareCritical);

	if (rejectH0) {
		outFile << "Werdykt: Odrzucamy H0\n";
	}
	else {
		outFile << "Werdykt: Brak podstaw do odrzucenia H0\n";
	}
}

// Analizę testu Kołmogorowa-Smirnowa robimy jedynie dla grupy jedno liczbowych / jedynek, 
// na takiej samej zasadności co przy teście chi-kwadrat
static void AnalisysTestKolmogorowaSmirnowa(const GroupNumberStats& stats, const std::string& outCsvPath) {
	std::string outCsvPathFinal = std::string(outCsvPath) + std::string("/analisys_test_K_S.csv");
	std::ofstream outFile(outCsvPathFinal);
	if (!outFile) throw std::runtime_error("Nie można otworzyć pliku outCSV: " + outCsvPathFinal);

	outFile << "Test Kolmogorowa-Smirnowa\n"
		<< "H0: rozkład wystąpień liczb w zakresie 1-49 jest zgodny z rozkładem jednostajnym\n"
		<< "Reguła decyzji: odrzucamy H0, jeśli najdalsza odległość będzie większa wartość krytyczna\n";

	// Brak wylosowanych danych 
	if (stats.draws <= 0) {
		outFile << "Test Kolmogorowa-Smirnowa = brak danych";
		return;
	}

	const long long N = stats.totalNumbers;
	const double expectedPerNumber = (double)N / 49.0;
	double maxDifference = 0.0;
	long long sum = 0;
	int numberWithMaxDifference = 0;

	// Tworzymy tzw. buffor string aby zapisać dane później w pliku
	std::ostringstream oss;
	oss << "Liczba,Wystąpienia,Skumulowane,Empiryczna,Teoretyczna,Różnica\n\n";

	for (int i = 1; i <= 49; i++) {
		const std::uint64_t mask = BitOfNumber(i);
		auto number = stats.count[1].find(mask);
		long long testVar = 0;
		if (number != stats.count[1].end()) {
			testVar = number->second;
			sum += testVar;
		}

		// Dystrybuanta empiryczna: jaki % liczb wypadło na ten moment
		double DE = static_cast <double> (sum) / N;
		// Dystrybuanta teoretyczna: jaki % liczb wypadło na ten moment
		double DT = static_cast <double> (i) / 49;
		// Różnica bezwzględna
		double difference = std::abs(DE - DT);

		oss << i << "," << testVar
			<< "," << sum
			<< "," << std::fixed << std::setprecision(8) << DE
			<< "," << std::fixed << std::setprecision(8) << DT
			<< "," << std::fixed << std::setprecision(8) << difference
			<< "\n";

		// Szukamy największej różnicy między dystrybuantami
		if (difference > maxDifference) {
			maxDifference = difference;
			numberWithMaxDifference = i;
		}
	}

	// Wartość krytyczna jest obliczana za pomocą wyznaczenia stopnia istotności, u nas 0.05.
	// Wtedy za pomocą wzoru na funkcję Q(lambda) = 1 - alpha, u nas będzie to 0.95
	// Wartość funkcji Q = 0.95 wynosi według tabeli: 1.36
	// https://agstrzelczak.zut.edu.pl/fileadmin/PiPD/rozklad_kolmogorowa_smirnowa.pdf

	const double aplha = 0.05;
	const double criticalAlpha = 1.36;
	double criticalValue = criticalAlpha / std::sqrt(N);

	const bool rejectH0 = (maxDifference > criticalValue);

	outFile << "Poziom istotności: alpha = 0.05\n"
		<< "Wartość krytyczna = " << criticalValue << "\n"
		<< "Ilość wylosowanych liczb = " << N << "\n"
		<< "Największa odległość = " << maxDifference << "\n"
		<< "Przy liczbie: " << numberWithMaxDifference << "\n\n";

	if (rejectH0) {
		outFile << "Werdykt: Odrzucamy H0\n\n";
	}
	else {
		outFile << "Werdykt: Brak podstaw do odrzucenia H0\n\n";
	}

	outFile << oss.str();
}

void Analisys::AnalizeFrequency(const std::string& inCsvPath, const std::string& outCsvPath) {
	// Inicjacja struktury
	GroupNumberStats stats;

	std::ifstream inFile(inCsvPath);
	if (!inFile) throw std::runtime_error("Nie można otworzyć pliku inCSV: " + inCsvPath);

	// Tworzenie pliku do zapisu logów i błędów
	std::string temp = outCsvPath;
	const std::string prefix = "output/";
	if (temp.rfind(prefix, 0) == 0) temp.erase(0, prefix.size());
	std::string outLogsPath = "output/logs/analisys/" + temp + ".log";

	std::error_code err;
	std::filesystem::path baseDir(outLogsPath);
	std::filesystem::path parent = baseDir.parent_path();
	if (!parent.empty()) {
		if (!std::filesystem::create_directories(parent, err) && err) {
			std::cout << "Błąd: Nie można utworzyć folderu dla logów analizy: " << err.message() << "\n";
			std::cout << "Info: Owy problem nie stanowi przeszkodą do prawidłowego działania programu.\n";
		}
	}

	std::ofstream logs_error(outLogsPath);
	if (!logs_error) {
		std::cout << "Błąd: Nie można otworzyć pliku do zapisu logów i błędów analizy: " << outLogsPath << "\n";
		std::cout << "Info: Owy problem nie stanowi przeszkodą do prawidłowego działania programu.\n";
	}

	std::string line;
	bool firstLine = true;

	while (std::getline(inFile, line)) {
		// Jeśli pusta linia pomijamy
		if (line.empty()) continue;

		// Pomijamy pierwszą linie, w której jest tylko informacja o nazwach danej kolumny
		if (firstLine) {
			firstLine = false;
			continue;
		}

		auto columns = SplitLine(line);

		// Lekkie sprawdzenie czy liczba kolumn w danej lini jest więcej niż 9,
		// czyli dla: drawSystemId,drawDate,gameType i 6 liczb (lub więcej)
		if (columns.size() < 9) {
			if (logs_error) logs_error << "#!Error: Linia ma mniej niż 9 kolumn: " << line << "\n";
			continue;
		}

		int numbers[6]{};
		bool ok = true;
		for (int i = 0; i < 6; i++) {
			// Konwert liczby ze stringa na int
			auto parsedInt = TryParseInt(columns[3 + static_cast<std::vector<std::string, std::allocator<std::string>>::size_type>(i)]);
			if (!parsedInt) {
				if (logs_error) logs_error << "#!Error: Nie można przekonwertować liczby: " << columns[3 + static_cast<std::vector<std::string, std::allocator<std::string>>::size_type>(i)] << "! W linijce: " << line << "\n";
				ok = false;
				break;
			}

			// Jeśli udał się konwert na liczbę całkowitą zapisujemy ją do tablicy
			numbers[i] = parsedInt.value();

			if (numbers[i] < 1 || numbers[i] > 49) {
				if (logs_error) logs_error << "#!Error: Liczba: " << numbers[i] << " jest poza zakresem 1-49 dla Lotto! W linijce: " << line << "\n";
				ok = false;
				break;
			}
		}

		// Jeśli był problem w zakresie/konwercie liczb pomijamy linię
		if (!ok) continue;

		stats.draws += 1;
		stats.totalNumbers += 6;
		std::sort(numbers, numbers + 6);
		// Zliczamy występy danej liczby do głównego licznika
		CountAllGroups(numbers, stats);
	}

	int ok = 0;
	// Zapis 6 plików csv dla każdej kombinacji grup liczb 1-6
	for (int i = 1; i <= 6; i++) {
		// Zapis do pliku wyjściowego w celu dalszej przedstawiania analizy
		std::string outCsvPathFinal = std::string(outCsvPath) + std::string("/analisys_group_") + std::to_string(i) + std::string(".csv");
		std::ofstream outFile(outCsvPathFinal);
		if (!outFile) throw std::runtime_error("Nie można otworzyć pliku outCSV: " + outCsvPathFinal);

		outFile << "groupOfNumbers = " << i << "\n";
		outFile << "totalNumberOfDraws = " << stats.draws << "\n";
		outFile << "totalNumberOfNumbers = " << stats.totalNumbers << "\n\n";
		outFile << "numbers,count,probHistory,probTheory,expected,share\n\n";

		if (stats.draws == 0) {
			std::cout << "#!Error: Nie przeanalizowano żadnych liczb dla grupy: " << i << "\n";
			ok++;
			continue;
		}

		const double probTheory = ProbDrawContainsFixedSubset(i); // i=1-6, P=49, N=6
		const double groupShare = (double)stats.draws * (double)BinomialCoefficients(6, i);

		for (const auto& [mask, count] : stats.count[i]) {
			const double probHistory = (double)count / (double)stats.draws;
			const double expected = (double)stats.draws * probTheory;
			const double share = (groupShare > 0.0) ? ((double)count / groupShare) : 0.0;
			outFile << MaskToString(mask)
				<< "," << count
				<< "," << std::fixed << std::setprecision(8) << probHistory
				<< "," << std::fixed << std::setprecision(8) << probTheory
				<< "," << std::fixed << std::setprecision(8) << expected
				<< "," << std::fixed << std::setprecision(8) << share
				<< "\n";
		}
	}

	// Informacja o mało wiarygodnych wynikach analizy poniższych testów
	if (stats.draws < 100) {
		std::cout << "\nUwaga: Mała liczba wyników może obniżac wiarygodność werdyktów następnych analiz: \nChi-Kwadrat oraz Kołmogorowa-Smirnowa\n";
	}

	// W funkcji wykonamy całą analizę testu Chi-Kwadrat
	AnalisysTestChiSquare(stats, outCsvPath);
	// W funkcji wykonamy całą analizę testu Kołmogorowa-Smirnowa
	AnalisysTestKolmogorowaSmirnowa(stats, outCsvPath);

	if (ok > 0) std::cout << "\n#!Error: Zakończono niepomyślnie analizę! "
		<< "\nNie odczytano żadnych danych do analizy!"
		<< "\nZapisano puste pliki do folderu: " << outCsvPath << "\n";

	std::cout << "\nZakończono pomyślnie analizę.\nZapisano do folderu: " << outCsvPath << "\n";
}