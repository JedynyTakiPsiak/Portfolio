#include "config.h"
#include <fstream>
#include <string>

// Format pliku "config.ini" musi być: API_KEY=<KEY>

std::string Config::LoadApiKey(const std::string& path) {
    std::ifstream f(path);
    if (!f) throw std::runtime_error("Brak pliku lub nie można otworzyć pliku: " + path);

    std::string line;
    const std::string prefix = "API_KEY=";
    while (std::getline(f, line)) {
        std::string value = line.substr(prefix.size());
        if (value.empty()) {
            throw std::runtime_error("Brak wartości API_KEY w pliku: " + path
                + "\n"
                + "Aby uzyskać prawidłowy klucz API, należy napisać mail\n"
                + "do: openapi@totalizator.pl\n"
                + "zawierajacy: imię i nazwisko, nr telefonu i mail.\n"
                + "Przybliżony czas oczekiwania na klucz to 1 dzień roboczy.\n"
            );
        }
        return value;
    }
    throw std::runtime_error("Brak wartości API_KEY w pliku: " + path
        + "\n"
        + "Aby uzyskać prawidłowy klucz API, należy napisać mail\n"
        + "do: openapi@totalizator.pl\n"
        + "zawierajacy: imię i nazwisko, nr telefonu i mail.\n"
        + "Przybliżony czas oczekiwania na klucz to 1 dzień roboczy.\n"
    );
}
