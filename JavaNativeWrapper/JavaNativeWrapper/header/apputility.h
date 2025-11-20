#pragma once

#include <filesystem>
#include <string>

namespace AppUtil {

	extern std::string relativeJREPath;
	extern std::string relativeJVMDLLPath;


	std::string checkJVMDLLFileExists(std::string const& filePath);

	std::string getEnvVar(std::string const& key);

	std::string findJVMDLLPath(bool log_enabled);

	void enableConsole();

	std::filesystem::path getExecutableModulePath();

}





