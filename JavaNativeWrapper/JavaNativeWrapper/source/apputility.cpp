#include "pch.h"
#include "apputility.h"
#include "stringutility.h"
#include "fileutility.h"
#include "logger.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>


namespace AppUtil {

    std::string relativeJREPath = "\\jre";
    std::string relativeBinPath = "\\bin";
    std::string relativeJVMDLLPath = "\\server\\jvm.dll";


    static std::string removeEndingPathSeperator(std::string const& filePath) {
        char endingChar = filePath.back();
        if (endingChar == '\\') {
            return filePath.substr(0, filePath.length() - 1);
        }
        if (endingChar == '/') {
            return filePath.substr(0, filePath.length() - 1);
        }
        return filePath;
    }


    std::string checkJVMDLLFileExists(std::string const& filePath) {
        if (filePath.compare("") != 0) {
            std::string jvmHomePath = removeEndingPathSeperator(filePath);

            if (FileUtil::isFileExists(jvmHomePath + relativeJVMDLLPath)) {
                return std::string(jvmHomePath + relativeJVMDLLPath);
            }
            if (FileUtil::isFileExists(jvmHomePath + relativeBinPath + relativeJVMDLLPath)) {
                return std::string(jvmHomePath + relativeBinPath + relativeJVMDLLPath);
            }
            if (FileUtil::isFileExists(jvmHomePath + relativeJREPath + relativeBinPath + relativeJVMDLLPath)) {
                return std::string(jvmHomePath + relativeJREPath + relativeBinPath + relativeJVMDLLPath);
            }
        }
        return std::string("");
    }


    std::string getEnvVar(std::string const& key) {
        char* val = getenv(key.c_str());
        return val == NULL ? std::string("") : std::string(val);
    }


    std::string findJVMDLLPath(bool log_enabled) {
        //Check 'jre' folder located within the executable folder
        std::filesystem::path exeModulePath = AppUtil::getExecutableModulePath();
        std::filesystem::path jreFolderPath = std::filesystem::path(exeModulePath.parent_path()) / "jre";
        std::string jre_path_java_home = checkJVMDLLFileExists(jreFolderPath.string());
        if (jre_path_java_home.compare("") != 0) {
            log_enabled&& Logger::log(LogLevel::INFO, "JVMDLL found from 'jre' folder in same parent directory.");
            return jre_path_java_home;
        }


        //Check 'JAVA_HOME' environment variable
        std::string env_JAVA_HOME_Val = checkJVMDLLFileExists(AppUtil::getEnvVar("JAVA_HOME"));
        if (env_JAVA_HOME_Val.compare("") != 0) {
            log_enabled && Logger::log(LogLevel::INFO, "JVMDLL found from 'JAVA_HOME' environment variable");
            return env_JAVA_HOME_Val;
        }
          
        //Check 'JRE_HOME' environment variable
        std::string env_JRE_HOME_Val = checkJVMDLLFileExists(AppUtil::getEnvVar("JRE_HOME"));
        if (env_JRE_HOME_Val.compare("") != 0) {
            log_enabled && Logger::log(LogLevel::INFO, "JVMDLL found from 'JRE_HOME' environment variable");
            return env_JRE_HOME_Val;
        }
       
        //Check 'path' environment variable
        std::string env_path_Val = AppUtil::getEnvVar("path");
        std::vector<std::string> env_paths = StringUtil::split(env_path_Val, ';');
        for (size_t i = 0; i < env_paths.size(); ++i) {
            std::string env_path = env_paths[i];
            std::string env_path_java_home = checkJVMDLLFileExists(env_path);
            if (env_path_java_home.compare("") != 0) {
                log_enabled && Logger::log(LogLevel::INFO, "JVMDLL found from 'path' environment variable");
                return env_path_java_home;
            }
        }

        //no jvm.dll path found
        log_enabled && Logger::log(LogLevel::INFO, "JVMDLL not found");
        return std::string("");
    }


    void enableConsole() {
        //Attach Console to this process
        AllocConsole();
        // Redirect stdout and stdin to console
        FILE* pCout;
        freopen_s(&pCout, "CONOUT$", "w", stdout);
        FILE* pCin;
        freopen_s(&pCin, "CONIN$", "r", stdin);
        std::cout.clear();
        std::cin.clear();
    }


    std::filesystem::path getExecutableModulePath() {
        char exePath[MAX_PATH];
        GetModuleFileNameA(NULL, exePath, MAX_PATH);
        std::filesystem::path p(exePath);
        return p;
    }

}





