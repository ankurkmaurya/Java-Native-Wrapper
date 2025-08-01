#pragma once

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>


namespace FileUtil {

    extern  std::string consoleOutLogFile;

    bool isFileExists(const std::string& filePath);

    std::vector<std::string> readTextFileAllLines(const std::string& filePath);

    void redirectStdErrToLogFile();

    void redirectConsoleToLogFile();

}


