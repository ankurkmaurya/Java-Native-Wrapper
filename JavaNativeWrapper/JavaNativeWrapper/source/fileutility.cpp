#include "pch.h"
#include "fileutility.h"
#include "logger.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>

#include <windows.h>


namespace FileUtil {

    std::string consoleOutLogFile = "";


    bool isFileExists(const std::string& filePath) {
        return std::filesystem::exists(filePath);
    }


    std::vector<std::string> readTextFileAllLines(const std::string& filePath) {
        std::vector<std::string> fileLines;
        if (!isFileExists(filePath)) {
            Logger::log(LogLevel::ERR, "File '{}' does not exist.", filePath);
            return fileLines;
        }

        // Read from the text file
        std::ifstream fileToRead(filePath);
        if (!fileToRead.is_open()) {
            Logger::log(LogLevel::ERR, "Failed to open file '{}'", filePath);
            return fileLines;
        }

        // Use a while loop together with the getline() function to read the file line by line
        std::string fileLine;
        while (getline(fileToRead, fileLine)) {
            fileLines.push_back(fileLine);
        }

        // Close the file
        fileToRead.close();

        return fileLines;
    }


    std::string createOrGetConsoleLogFolder() {
        std::string consoleLogFolder = "consolelogs";
        // Define the path for the new directory
        std::filesystem::path dirPath = consoleLogFolder;
        // Attempt to create the directory
        if (std::filesystem::create_directory(dirPath)) {
            //std::cout << "Directory '" << consoleLogFolder << "' created successfully." << std::endl;
        }
        return consoleLogFolder;
    }


    void redirectStdErrToLogFile() {
        std::string consoleLogFolder = createOrGetConsoleLogFolder();
        FILE* streamStdErr;
        std::string consoleErrorLogFile = consoleLogFolder + "/stderr.log";
        if ((streamStdErr = freopen(consoleErrorLogFile.c_str(), "a", stderr)) == NULL) {
            std::cerr << "[Error] Failed to redirect standard error to file." << std::endl;
        }
    }


    void redirectConsoleToLogFile() {
        //Redirect Standard Output Logs
        std::string consoleLogFolder = createOrGetConsoleLogFolder();
        DWORD current_pid = GetCurrentProcessId();
        FILE* streamStdOut;
        //consoleOutLogFile = consoleLogFolder + "/stdconsole-" + std::to_string(current_pid) +".log";
        consoleOutLogFile = consoleLogFolder + "/stdconsole.log";
        if ((streamStdOut = freopen(consoleOutLogFile.c_str(), "a", stdout)) == NULL) {
            std::cerr << "[Error] Failed to redirect standard output to file." << std::endl;
        }

        //Redirect Standard Error Logs
        redirectStdErrToLogFile();
    }

}


