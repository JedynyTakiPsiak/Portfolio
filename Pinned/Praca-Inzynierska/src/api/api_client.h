#pragma once

#include <string>
#include <curl/curl.h>

class ApiClient {
public:
	// Używając metody RAII możemy tworzyć zasób w konstruktorze, i potem zwalniać w destruktorze
	// Destruktor automatycznie odpala się w momencie zakończenia, 
	// np. przy return oraz niby według wyjścia przez błędy runtime? (tego drugiego nie jestem pewien)
	// https://www.reddit.com/r/cpp_questions/comments/d78wg4/what_is_raii/?tl=pl
	// https://en.cppreference.com/w/cpp/language/raii.html
	ApiClient(const std::string& apiKey);
	~ApiClient();

	// Główna funkcja do pobierania zawartości API i zwrócenie przez string
	std::string GetResults(const std::string& url);

	void ValidateApiKey();

private:
	// Inicjujemy sobie w klasie zmienne dla nagłówka aby prościej sobie później wywoływać ten sam nagłówek wiele razy
	CURL* curl_ = nullptr;
	struct curl_slist* headers_ = nullptr;

	// Funkcja do zapisy danych z odpowiedzi HTTP
	static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
};






