#include "api_client.h"

#include <iostream>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

// Funkcja do zapisy danych z odpowiedzi HTTP
// https://stackoverflow.com/questions/9786150/save-curl-content-result-into-a-string-in-c
size_t ApiClient::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

ApiClient::ApiClient(const std::string& apiKey) {
    // Według poradnika: https://curl.se/libcurl/c/libcurl-tutorial.html
    // Przykład z github: https://gist.github.com/alghanmi/c5d7b761b2c9ab199157?permalink_comment_id=2046201
    // Lokalna inicjacja CURL
    curl_ = curl_easy_init();

    if (!curl_) throw std::runtime_error("Nieudana inicjacja CURL lokalnie");

    headers_ = curl_slist_append(headers_, "accept: application/json");
    headers_ = curl_slist_append(headers_, ("secret: " + apiKey).c_str());

    curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers_);
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, ApiClient::WriteCallback);
}

ApiClient::~ApiClient() {
    if (headers_) curl_slist_free_all(headers_);
    if (curl_) curl_easy_cleanup(curl_);
    curl_global_cleanup();
}

std::string ApiClient::GetResults(const std::string& url) {
    std::string response;
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl_);
    if (res != CURLE_OK) throw std::runtime_error(std::string("CURL: ") + curl_easy_strerror(res));

    long httpCode = 0;
    curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &httpCode);

    // Dla walidacji klucza API dodane konkretne sprawdzenie kodu HTTP
    if (httpCode == 401) throw std::runtime_error(std::string("HTTP Code: ") + std::to_string(httpCode)
        + std::string(", Klucz API nieautoryzowany lub nieprawidłowy!"));

    if (httpCode != 200) throw std::runtime_error(std::string("HTTP Code: ") + std::to_string(httpCode));

    return response;
}

void ApiClient::ValidateApiKey() {
    const std::string url = "https://developers.lotto.pl/api/open/v1/lotteries/info?gameType=Lotto";
    nlohmann::json j = nlohmann::json::parse(GetResults(url));
    
    // Estetyczna rzecz
    std::cout << "\n";

    if (j.contains("gameType") && j["gameType"].is_string()) {
        std::string gameType = j["gameType"].get<std::string>();
        if (gameType == "Lotto" && j.contains("nextDrawDate")) {
            std::cout << "\nKlucz API jest prawidłowy i przeszedł pełną walidację. \n";
        } else {
            std::cout << "\nKlucz API przeszedł w połowie walidację. \n"
                << "Odpowiedź zapytania zwraca klucz ""gameType"", lecz jest nieprawidłowy z zadanym parametrem! \n";
        }
    } else {
        std::cout << "\nKlucz API przeszedł w połowie walidację. \n"
            << "Odpowiedź zapytania nie zawiera klucza ""gameType""! \n";
    }
}
