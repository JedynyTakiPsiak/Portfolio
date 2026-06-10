#pragma once

#include <string>

struct Date {
    int year = 0;
    int month = 0;
    int day = 0;
};

struct DataManagerParams {
    std::string gameType;
    std::string startDrawDate;
    int N = 1;
    std::string outCsvPath;
};

struct AnalizeFrequencyParams {
    std::string inCsvPath;
    std::string outCsvPath;
};

struct AnalisysToHtmlParams {
    std::string inCsvPath;
    std::string outHtmlPath;
    std::string title;
};

struct FullProgramParams {
    std::string gameType;
    std::string startDrawDate;
    int N = 1;
    std::string title;
    std::string outHtmlPath;
};

class UserInterface {
public:
    static int AskChoice(const int& minValue, const int& maxValue);
    static std::string AskChoiceDate(const std::string& firstDraw, const std::string& todayDate);
    static int AskChoiceCount(const int& minValue, const int& maxValue);
    static std::string AskChoicePath(const std::string& ifInOutFile, const bool& folderCreate);
    static std::string AskChoiceTitle(const int& maxLenght, const std::string& defaultTitle);

    static void PrintHeader();
    static void PrintMenu();
    static void ClearConsole();
    static void WaitForAction();

    static DataManagerParams AskAboutDataManagerParams();
    static AnalizeFrequencyParams AskAboutAnalizeFrequencyParams();
    static AnalisysToHtmlParams AskAboutAnalisysToHtmlParams();
    static FullProgramParams AskAboutFullProgramParams();
};