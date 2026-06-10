// Wczytanie bibliotek programu
#include <curl/curl.h>
#include <iostream>
#include <string>
#include <Windows.h>
#include <filesystem>

// Wczytanie nagłówków programu
#include <config/config.h>
#include <api/api_client.h>
#include <dane/data_manager.h>
#include <analiza/analisys.h>
#include <wizualizacja/analisys_raport.h>
#include <ui/user_interface.h>


static std::string CreateTempDir(const std::string& baseDirPath, const int maxN = 999) {
    std::error_code err;
    std::filesystem::path baseDir(baseDirPath);
    std::filesystem::create_directories(baseDir, err);
    if (err) {
        throw std::runtime_error("Nie można utworzyć folderu roboczego: " + err.message());
    }

    for (int i = 1; i <= maxN; i++) {
        std::filesystem::path tempDir = baseDir / ("temp" + std::to_string(i));
        err.clear();

        if (std::filesystem::create_directories(tempDir, err)) {
            std::filesystem::path analyzedDir = tempDir / "AnalyzedDraws";
            err.clear();

            std::filesystem::create_directories(analyzedDir, err);
            if (err) {
                throw std::runtime_error("Nie można utworzyć folderu AnalyzedDraws: " + err.message());
            }

            return tempDir.generic_string();
        }

        if (err) {
            throw std::runtime_error("Nie można utworzyć folderu roboczego: " + tempDir.string() + ", ponieważ: " + err.message());
        }
    }
    throw std::runtime_error("Nie znaleziono wolnej nazwy dla folderu roboczego 'tempX'. \nWszystkie pozycje od 1-" + std::to_string(maxN) + " są zajęte.");
}

int main() {
    // Konfig pod polskie znaki
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);
    
    // Pętla dla wyświetlania ciągle menu interfejsu użytkownika
    while (true) {
        UserInterface::ClearConsole();
        UserInterface::PrintHeader();
        UserInterface::PrintMenu();

        // W lepszy sposób próbujemy wykonać program, jeśli napotka na jakiś error,
        // odrazu przerywa program i przez catch wyrzuca nam co jest problemem
        try {
            int userChoice = UserInterface::AskChoice(0, 5);

            if (userChoice == 0) {
                return 0; // Zakończ program z racji wybrania opcji 0
            } 
            
            // Globalna inicjacja CURL
            if (curl_global_init(CURL_GLOBAL_DEFAULT) != 0) throw std::runtime_error("Nieudana inicjacja CURL globalnie");

            // Próba odczytu klucza API
            const std::string apiKey = Config::LoadApiKey("config.ini");

            // Zapis klucza API do programu
            ApiClient client(apiKey);

            switch (userChoice) {
            case 1: // Zwaliduj klucz API
                client.ValidateApiKey();
                break;
            case 2: { // Pobierz dane
                auto params = UserInterface::AskAboutDataManagerParams();
                DataManager::SaveDrawsToCsv(client, params.gameType, params.startDrawDate, params.N, params.outCsvPath);
                break;
            }
            case 3: { // Przeanalizuj dane
                auto params = UserInterface::AskAboutAnalizeFrequencyParams();
                Analisys::AnalizeFrequency(params.inCsvPath, params.outCsvPath);
                break;
            }
            case 4: { // Zwizualizuj dane
                auto params = UserInterface::AskAboutAnalisysToHtmlParams();
                Raport{}.AnalisysToHtml(params.inCsvPath, params.outHtmlPath, params.title);
                break;
            }
            case 5: { // Wykonaj pełny program
                auto params = UserInterface::AskAboutFullProgramParams();
                std::string tempDir = CreateTempDir("output");
                DataManager::SaveDrawsToCsv(client, params.gameType, params.startDrawDate, params.N, std::string(tempDir + "/DownloadedDraws.csv"));
                Analisys::AnalizeFrequency(std::string(tempDir + "/DownloadedDraws.csv"), std::string(tempDir + "/AnalyzedDraws"));
                Raport{}.AnalisysToHtml(std::string(tempDir + "/AnalyzedDraws"), params.outHtmlPath, params.title);
                break;
            }
            default:
                break;
            }
        } catch (const std::exception& e) {
            std::cerr << "\n\n#ERROR! " << e.what() << "\n\n";
            curl_global_cleanup();
            return 1;
        }

        UserInterface::WaitForAction();
    }
    curl_global_cleanup();
    return 0;
}
