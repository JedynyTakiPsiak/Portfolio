#pragma once

#include <string>
#include <array>
#include <unordered_map>

struct GroupNumberStats {
	int draws = 0; // Liczba poprawnie zczytanych losowań
	long long totalNumbers = 0; // Liczba łącznie zczytanych liczb (powinno być draws * 6!)

	// Inicjujemy tablicę nie ułożonych map dla grup liczb 0-6 (np. 2 == pary liczb, 4 == czwórki itd.)
	// (dlatego inicjujemy do 7, z czego 0 nie będzie używane z racji wygody indeksowania)
	// W każdej mapie mamy klucz opisujący konkretny zestaw liczb oraz wartość liczby wystąpień danego zestawienia
	std::array <std::unordered_map <std::uint64_t, long long>, 7> count;
};

class Analisys {
public: 
	static void AnalizeFrequency(const std::string& inCsvPath, const std::string& outCsvPath);
};