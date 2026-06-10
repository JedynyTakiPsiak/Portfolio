#pragma once
// pragma once albo #ifndef/#define zabezpiecza przed wielokrotnym dołączeniem nagłówka
// z czego dla pragma potrzeba popularnego kompilatora który go wspiera, ponieważ w standardzie C++ tego nie ma

#include <string>

class Config {
public: 
	static std::string LoadApiKey(const std::string& path = "config.ini");
};