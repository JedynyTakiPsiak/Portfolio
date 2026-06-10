#include "user_interface.h"

#include <iostream>
#include <string>
#include <chrono>
#include <format>
#include <fstream>
#include <filesystem>


int UserInterface::AskChoice(const int& minValue, const int& maxValue) {
    while (true) {
        std::cout << "Wybierz opcję (" << minValue << "-" << maxValue << "): ";
        std::string line;
        std::getline(std::cin, line);
        // https://www.codecademy.com/article/how-to-convert-a-string-to-an-integer-in-c
        const char* str = line.c_str();
        char* endptr;
        int num = strtol(str, &endptr, 10);

        if (*endptr != '\0' || line.empty() || endptr == str) {
            std::cout << "Niepoprawny wybór! ";
            continue;
        }

        int value = (int)num;
        if (value < minValue || value > maxValue) {
            std::cout << "Wybór poza zakresem! ";
            continue;
        }

        if (value >= minValue && value <= maxValue) return value;
    }
}

// Funkcja do parsowania według standardu: YYYY-MM-DD
// Poprzez referencje zmieniamy string na integer, bez dodatkowego zwracania
static bool ParseStringToDateInt(const std::string& string, Date& date) {
    if (string.size() != 10) return false;
    if (string[4] != '-' || string[7] != '-') return false;

    for (int i : {0, 1, 2, 3, 5, 6, 8, 9}) if (!isdigit(string[i])) return false;

    int year = std::stoi(string.substr(0, 4));
    int month = std::stoi(string.substr(5, 2));
    int day = std::stoi(string.substr(8, 2));

    if (month < 1 || month > 12) return false;
    if (day < 1 || day > 31) return false;

    date = Date {year, month, day};
    return true;
}

static bool DateLessOrEqual(const Date& dateA, const Date& dateB) {
    if (dateA.year != dateB.year) return dateA.year < dateB.year;
    if (dateA.month != dateB.month) return dateA.month < dateB.month;
    return dateA.day <= dateB.day;
}

// https://stackoverflow.com/questions/75882118/is-there-any-date-time-function-that-handles-tomorrows-date-including-carry-ove
static std::string TodayDate() {
    std::chrono::zoned_time zt_now{ std::chrono::current_zone(), std::chrono::system_clock::now() };
    auto date = std::chrono::floor<std::chrono::days>(zt_now.get_local_time());
    return std::format("{:%F}", date);
}

std::string UserInterface::AskChoiceDate(const std::string& firstDraw, const std::string& todayDate) {
    std::cout << "Podaj datę rozpoczęcia pobierania danych w formacie YYYY-MM-DD \n"
              << "Wpisz 'dzisiaj', aby wybrać dzisiejszą datę. \n"
              << "Możliwy zakres: " << firstDraw << " a " << todayDate << ": ";
    while (true) {
        std::string line;
        std::getline(std::cin, line);

        if (line == "dzisiaj" || line == "dzis" || line == "d" || line == "today") {
            return todayDate;
        }

        Date inputDate{};
        if (!ParseStringToDateInt(line, inputDate)) {
            std::cout << "Błąd: Niepoprawna data lub zły format! \n";
            continue;
        }

        Date firstDrawDate{};
        ParseStringToDateInt(firstDraw, firstDrawDate);
        Date todayDateDate{};
        ParseStringToDateInt(todayDate, todayDateDate);


        if (!(DateLessOrEqual(firstDrawDate, inputDate) && DateLessOrEqual(inputDate, todayDateDate))) {
            std::cout << "Błąd: Data poza wskazanym zakresem! \n";
            continue;
        }

        return (line + "T21:00:00Z");
    }
}

int UserInterface::AskChoiceCount(const int& minValue, const int& maxValue) {
    std::cout << "Podaj liczbę losowań do pobrania (0 = cała historia): ";
    while (true) {
        std::string line;
        std::getline(std::cin, line);
        // https://www.codecademy.com/article/how-to-convert-a-string-to-an-integer-in-c
        const char* str = line.c_str();
        char* endptr;
        int num = strtol(str, &endptr, 10);

        if (*endptr != '\0' || line.empty() || endptr == str) {
            std::cout << "Błąd: Niepoprawna liczba! \n";
            continue;
        }

        int value = (int)num;
        if (value < minValue || value > maxValue) {
            std::cout << "Błąd: Liczba poza zakresem! \n";
            continue;
        }

        if (value >= minValue && value <= maxValue) return value;
    }
}

// https://stackoverflow.com/questions/16400009/why-the-char-comparison-ifc-a-c-z-is-not-portable
static bool IsAllowedChar(char c) {
    if (c >= 'a' && c <= 'z') return true;
    if (c >= 'A' && c <= 'Z') return true;
    if (c >= '0' && c <= '9') return true;
    if (c == '/') return true;
    return false;
}

// Sprawdzamy czy podana ścieżka jest według wzora:
// same znaki aA-zZ oraz 0-9
// znak / nie występuje na początku i końcu
// znak / występuje jedynie jeden raz, 
// aby nie było przypadku pustego folderu bądź błędny zapis
static bool IsValidOutPath(const std::string& s) {
    if (s.empty()) return false;
    if (s.front() == '/' || s.back() == '/') return false;
    for (char c : s) if (!IsAllowedChar(c)) return false;
    if (s.find("//") != std::string::npos) return false;
    return true;
}

// https://stackoverflow.com/questions/12774207/fastest-way-to-check-if-a-file-exists-using-standard-c-c11-14-17-c
static inline bool IfFileExists(const std::string& name) {
    std::ifstream f(name.c_str());
    return f.good();
}

// https://stackoverflow.com/questions/71658440/c17-create-directories-automatically-given-a-file-path
static bool CreateDirectoryRecursive(const std::filesystem::path& dirName, std::error_code& err) {
    err.clear();
    if (!std::filesystem::create_directories(dirName, err)) {
        if (!err && std::filesystem::exists(dirName)) {
            // Folder już istnieje
            err.clear();
            return true;
        }
        return false;
    }
    return true;
}

static bool IfDirectoryExists(const std::filesystem::path& dirName, std::error_code& err) {
    err.clear();
    if (!std::filesystem::exists(dirName, err) || err) return false;
    if (!std::filesystem::is_directory(dirName, err) || err) return false;
    return true;
}

std::string UserInterface::AskChoicePath(const std::string& ifInOutFile, const bool& dirCreate) {
    std::cout << "Info: Program operuje tylko na plikach z folderu '/output' (obok programu)\n";
    if (ifInOutFile == "out" && dirCreate) std::cout << "Info: Przy analizie są tworzone odrębne pliki, dlatego potrzebny jest folder. \n";
    if (ifInOutFile == "out" || dirCreate) std::cout << "Przykład: 'test123', 'temp1/dasTest321' \n";
    if (ifInOutFile == "in" && !dirCreate) std::cout << "Przykład: 'test.csv', 'temp1/das.csv' \n";

    while (true) {
        if (ifInOutFile == "in" && !dirCreate) std::cout << "Podaj ścieżkę do pliku wejściowego: ";
        if (ifInOutFile == "in" && dirCreate) std::cout << "Podaj ścieżkę do folderu z plikami analizy: ";
        if (ifInOutFile == "out" && !dirCreate) std::cout << "Podaj nazwę pliku wyjściowego: ";
        if (ifInOutFile == "out" && dirCreate) std::cout << "Podaj nazwę folderu wyjściowego: ";
        std::string line;
        std::getline(std::cin, line);
        std::string baseDir = "output";
        std::string fullPath = baseDir + "/" + line;

        if (line.empty()) {
            std::cout << "Błąd: Pusta wartość! \n";
            continue;
        }

        if (ifInOutFile == "in") {
            if (!dirCreate) {
                if (!IfFileExists(fullPath)) {
                    std::cout << "Błąd: Plik nie istnieje lub nie ma uprawnień! \n";
                    continue;
                }
                return fullPath;
            } else {
                std::error_code err;
                if (!IfDirectoryExists(std::filesystem::path(fullPath), err)) {
                    std::cout << "Błąd: Plik nie istnieje lub nie ma uprawnień! \n";
                    continue;
                }
                return fullPath;
            }
        }

        if (ifInOutFile == "out") {
            if (!IsValidOutPath(fullPath)) {
                std::cout << "Błąd: Dozwolone znaki to: a-z A-Z 0-9 oraz '/'! \n"
                          << "Znak '/' nie może być na końcu lub początku, oraz powtarzać. \n";
                continue;
            }

            std::filesystem::path filePath(fullPath);
            std::filesystem::path parent = filePath.parent_path();

            if (dirCreate) {
                std::error_code err;
                if (!CreateDirectoryRecursive(filePath, err)) {
                    std::cout << "Błąd: Podczas tworzenia foldera wystąpił problem: " << err.message() << "\n";
                    continue;
                }
                return fullPath;
            }


            // Jeśli użytkownik podał jedynie plik bez folderów, parent będzie pusty
            if (!parent.empty()) {
                std::error_code err;
                if (!CreateDirectoryRecursive(parent, err)) {
                    std::cout << "Błąd: Podczas tworzenia foldera wystąpił problem: " << err.message() << "\n";
                    continue;
                }
            }

            {
                std::error_code err;
                if (std::filesystem::exists(fullPath, err) && !err) {
                    std::cout << "Uwaga: Plik już istnieje i zostanie nadpisany: " << fullPath << "\n";
                    std::string answer;
                    do {
                        std::cout << "Czy chcesz nadpisać plik? y/n \n";
                        std::getline(std::cin, answer);
                        if (answer.empty()) continue;
                    } while (!isalpha(answer[0]) || !(std::tolower(answer[0]) == 'y' || std::tolower(answer[0]) == 'n'));

                    if (answer == "n") {
                        continue;
                    }
                }
            }

            return fullPath;
        }
    }
}

// https://www.algorytm.edu.pl/wstp-do-c/ascii.html
static bool IsValidTitle(const std::string& s) {
    if (s.empty()) return false;
    for (unsigned char c : s) {
        // Odrzucamy znaki kontrolne i DEL prócz spacji
        if (c < 32 || c == 127) return false;

        // Odrzucamy znaki niebezpieczne dla struktury HTML
        if (c == '<' || c == '>' || c == '&' || c == '"' || c == '\'' || c == '`') {
            return false;
        }
    }
    return true;
}

std::string UserInterface::AskChoiceTitle(const int& maxLenght, const std::string& defaultTitle) {
    while (true) {
        std::cout << "Podaj tytuł dokumentu (Enter = domyślny tytuł): ";
        std::string line;
        std::getline(std::cin, line);

        if (line.empty()) {
            return defaultTitle;
        }

        if ((int) line.size() > maxLenght) {
            std::cout << "Błąd: Maksymalna wielkość tytułu to 100 znaków! \n";
            continue;
        }

        if (!IsValidTitle(line)) {
            std::cout << "Błąd: Dozwolone znaki to: a-z, A-Z, 0-9 oraz spacja! \n";
            continue;
        }

        return line;
    }
}

void UserInterface::PrintHeader() {
    std::cout
        << "\n" << " ┌────────────────────────────────────────────┐ "
        << "\n" << " │ SYSTEM ANALIZY STATYSTYCZNEJ GIER LOSOWYCH │ "
        << "\n" << " └────────────────────────────────────────────┘ "
        << "\n" << ""
        << "\n" << "Autor: Jakub Ćwiek"
        << "\n" << "Uczelnia: Uniwersytet im. Adama Mickiewicza w Poznaniu"
        << "\n" << "Kierunek: Technologie Komputerowe 2022-2026"
        << "\n\n\n" << "";
}

void UserInterface::PrintMenu() {
    std::cout
        << "0. Zakończ program" << "\n"
        << "1. Zwaliduj klucz API" << "\n"
        << "2. Pobierz dane" << "\n"
        << "3. Przeanalizuj dane" << "\n"
        << "4. Zwizualizuj dane" << "\n"
        << "5. Wykonaj pełny program" << "\n"
        << "\n" << "";
}

// https://stackoverflow.com/questions/41430453/how-to-clear-console-in-c-on-windows-using-netbeans-cygwin
void UserInterface::ClearConsole() {
    std::cout << "\033[2J\033[H";
}

// https://stackoverflow.com/questions/43164712/hit-enter-to-continue-or-skip-function
void UserInterface::WaitForAction() {
    std::cout << "\n\n Naciśnij Enter aby kontynuować program...";
    std::string temp;
    std::getline(std::cin, temp);
}



DataManagerParams UserInterface::AskAboutDataManagerParams() {
    DataManagerParams params;
    
    { // 1 Parametr: string gameType
        std::cout << "\n\n 1. Lotto \n";
        int userChoice = AskChoice(1, 1);
        if (userChoice == 1) params.gameType = "Lotto";
    }

    { // 2 Parametr: string startDrawDate
        std::cout << "\n\n";
        const std::string firstDrawOfLotto = "2009-10-10";
        const std::string todayDate = TodayDate();
        if (params.gameType == "Lotto") {
            std::cout << "Info: Pierwsze losowanie nowego systemu Lotto odbyło się: " << firstDrawOfLotto << "\n";
        }

        params.startDrawDate = AskChoiceDate(firstDrawOfLotto, todayDate);
    }

    { // 3 Parametr: int N
        std::cout << "\n\n";
        params.N = AskChoiceCount(0, 999999);
    }

    { // 4 Parametr: string outCsvPath
        std::cout << "\n\n";
        params.outCsvPath = AskChoicePath("out", false) + ".csv";
    }

    return params;
}

AnalizeFrequencyParams UserInterface::AskAboutAnalizeFrequencyParams() {
    AnalizeFrequencyParams params;

    { // 1 Parametr: string inCsvPath
        std::cout << "\n\n";
        params.inCsvPath = AskChoicePath("in", false);
    }

    { // 2 Parametr: string outCsvPath
        std::cout << "\n\n";
        params.outCsvPath = AskChoicePath("out", true);
    }

    return params;
}

AnalisysToHtmlParams UserInterface::AskAboutAnalisysToHtmlParams() {
    AnalisysToHtmlParams params;

    { // 1 Parametr: string inCsvPath
        std::cout << "\n\n";
        params.inCsvPath = AskChoicePath("in", true);
    }

    { // 2 Parametr: string outHtmlPath
        std::cout << "\n\n";
        params.outHtmlPath = AskChoicePath("out", false) + ".html";
    }

    { // 3 Parametr: string title
        std::cout << "\n\n";
        params.title = AskChoiceTitle(100, "Wizualizacja Analizy Lotto");
    }

    return params;
}

FullProgramParams  UserInterface::AskAboutFullProgramParams() {
    FullProgramParams params;

    { // 1 Parametr: string gameType
        std::cout << "\n\n 1. Lotto \n";
        int userChoice = AskChoice(1, 1);
        if (userChoice == 1) params.gameType = "Lotto";
    }

    { // 2 Parametr: string startDrawDate
        std::cout << "\n\n";
        const std::string firstDrawOfLotto = "2009-10-10";
        const std::string todayDate = TodayDate();
        if (params.gameType == "Lotto") {
            std::cout << "Info: Pierwsze losowanie nowego systemu Lotto odbyło się: " << firstDrawOfLotto << "\n";
        }

        params.startDrawDate = AskChoiceDate(firstDrawOfLotto, todayDate);
    }

    { // 3 Parametr: int N
        std::cout << "\n\n";
        params.N = AskChoiceCount(0, 999999);
    }

    { // 4 Parametr: string title
        std::cout << "\n\n";
        params.title = AskChoiceTitle(100, "Wizualizacja Analizy Lotto");
    }

    { // 5 Parametr: string outHtmlPath
        std::cout << "\n\n";
        params.outHtmlPath = AskChoicePath("out", false) + ".html";
    }

    return params;
}