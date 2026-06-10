#pragma once

#include <string>
#include <vector>

// Struktura do zczytania wszystkich wyników, posortowania ich
// i zapisu wybranych do pliku html
struct Row {
	std::string numbers;
	long long count = 0;
	double probHistory = 0.0;
	double probTheory = 0.0;
	double expected = 0.0;
	double share = 0.0;
};

struct GroupFileData {
	int groupOfNumbers = 0;
	int totalNumberOfDraws = 0;
	long long totalNumberOfNumbers = 0;
	std::vector <Row> rows;
};

class Raport {
public:
	void AnalisysToHtml(const std::string& inDirPath, const std::string& outHtmlPath, const std::string& title);
	void WriteHtmlHeader(std::ofstream& outFile, const std::string& title);
};