#pragma once

#include <fstream>
#include <string>
#include <sstream>
#include <windows.h>
#include <filesystem>
#include <chrono>

enum class LogLevel {
    INFO,
    WARN,
    ERR
};

namespace Logger {
    int log(LogLevel level, const wchar_t* msg);
    int log(LogLevel level, const wchar_t* msg1, const wchar_t* msg2);
    int log(LogLevel level, const wchar_t* msg1, const wchar_t* msg2, const wchar_t* msg3);
    int log(LogLevel level, const std::string& msg);

    int log(LogLevel level, const std::string& msgFormat, DWORD arg1);
    int log(LogLevel level, const std::string& msgFormat, std::string arg1);

    int log(LogLevel level, const std::string& msgFormat, std::string arg1, std::string arg2);
}