#include "pch.h"
#include "stringutility.h"

#include <Windows.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>


namespace StringUtil {

	bool contains(const std::string& searchString, const std::string& subString) {
		if (searchString.find(subString) != std::string::npos) {
			return true;
		}
		return false;
	}

	std::string replaceAll(std::string sourceString, const std::string& target, const std::string& replace) {
		size_t start_pos = 0;
		while ((start_pos = sourceString.find(target, start_pos)) != std::string::npos) {
			sourceString.replace(start_pos, target.length(), replace);
			start_pos += replace.length(); // Handles case where 'replace' is a substring of 'target'
		}
		return sourceString;
	}

	std::vector<std::string> split(const std::string& inputStr, char delimiter) {
		std::vector<std::string> tokens;
		if (inputStr.empty()) {
			return tokens;
		}

		size_t start = 0;
		size_t end = inputStr.find(delimiter);
		while (end != std::string::npos) {
			tokens.push_back(inputStr.substr(start, end - start));
			start = end + 1;
			end = inputStr.find(delimiter, start);
		}

		tokens.push_back(inputStr.substr(start));
		return tokens;
	}

	// Converts std::string (assumed UTF-8) to LPWSTR
	LPWSTR convertStringToLPWSTR(const std::string& input) {
		// Get required buffer size
		int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, input.c_str(), -1, nullptr, 0);
		if (sizeNeeded == 0) return nullptr;

		// Allocate buffer (caller must free this later using delete[])
		LPWSTR result = new WCHAR[sizeNeeded];

		// Perform conversion
		MultiByteToWideChar(CP_UTF8, 0, input.c_str(), -1, result, sizeNeeded);

		return result;
	}

	std::string convertLPWSTRToString(LPCWSTR input) {
		if (!input) return {};

		// Get required buffer size
		int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, input, -1, nullptr, 0, nullptr, nullptr);
		if (sizeNeeded == 0) return {};

		// Allocate string
		std::string result(sizeNeeded, 0);

		// Perform conversion
		WideCharToMultiByte(CP_UTF8, 0, input, -1, &result[0], sizeNeeded, nullptr, nullptr);

		// Remove the null terminator if not needed
		result.pop_back(); // Optional: remove the '\0' added by Windows

		return result;
	}


}
