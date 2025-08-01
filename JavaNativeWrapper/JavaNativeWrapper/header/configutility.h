#pragma once

#include "fileutility.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>


namespace ConfigUtil {

	std::string getConfigValue(const std::string& configFilePath, const std::string& keyName);

}

