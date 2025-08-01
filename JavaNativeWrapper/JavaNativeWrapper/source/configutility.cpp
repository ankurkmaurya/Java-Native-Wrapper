#include "pch.h"
#include "fileutility.h"
#include "stringutility.h"
#include "configutility.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>


namespace ConfigUtil {

	std::string getConfigValue(const std::string& configFilePath, const std::string& keyName) {
		std::vector<std::string> fileLines = FileUtil::readTextFileAllLines(configFilePath);
		for (size_t i = 0; i < fileLines.size(); ++i) {
			std::string configKeyValue = fileLines[i];
			if (StringUtil::contains(configKeyValue, keyName)) {
				return StringUtil::replaceAll(std::string(configKeyValue),
					std::string(keyName),
					std::string(""));
			}
		}
		return "";
	}

}


