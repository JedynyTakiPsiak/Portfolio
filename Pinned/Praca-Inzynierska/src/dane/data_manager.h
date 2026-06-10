#pragma once

#include <string>

class ApiClient;

class DataManager {
public:
	static void SaveDrawsToCsv(
		ApiClient& client,
		const std::string& gameType,
		const std::string& startDrawDate,
		int N,
		const std::string& outCsvPath
	);
};