#pragma once

#include <Windows.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>


namespace StringUtil {

	bool contains(const std::string& searchString, const std::string& subString);

	std::string replaceAll(std::string sourceString, const std::string& target, const std::string& replace);

	std::vector<std::string> split(const std::string& inputStr, char delimiter);

	LPWSTR convertStringToLPWSTR(const std::string& inputStr);

	std::string convertLPWSTRToString(LPCWSTR input);
}


