#include "pch.h"

#include <fstream>
#include <string>
#include <sstream>
#include <windows.h>
#include <filesystem>
#include <chrono>
#include <format>

#include "logger.h"
#include "stringutility.h"
#include "appconfiguration.h"


namespace Logger {
    namespace fs = std::filesystem;

    const std::wstring LOG_DIR = L"servicelog";

    const wchar_t* toString(LogLevel level) {
        switch (level) {
            case LogLevel::INFO : return L"INFO";
            case LogLevel::WARN : return L"WARN";
            case LogLevel::ERR  : return L"ERROR";
            default:              return L"UNKNOWN";
        }
    }


    // Create log folder if it doesn't exist
    void createDirectoryIfNotExists() {
        if (!fs::exists(LOG_DIR)) {
            fs::create_directories(LOG_DIR);
        }
    }


    // Build daily log file name
    std::wstring getLogFilePath() {
        SYSTEMTIME st;
        GetLocalTime(&st);

        std::wstringstream filename;
        filename << LOG_DIR << L"\\ServiceLog_"
            << st.wYear << L"-"
            << (st.wMonth < 10 ? L"0" : L"") << st.wMonth << L"-"
            << (st.wDay < 10 ? L"0" : L"") << st.wDay << L".log";

        return filename.str();
    }


    // Delete logs older than 30 days
    void cleanupOldLogs(int daysToKeep = 30) {
        namespace chrono = std::chrono;

        const auto now = chrono::system_clock::now();
        const auto keepDuration = chrono::hours(daysToKeep * 24);

        for (const auto& entry : fs::directory_iterator(LOG_DIR)) {
            if (!entry.is_regular_file()) continue;

            try {
                auto ftime = entry.last_write_time();

                // Convert to system_clock time_point
                auto sys_time = chrono::time_point_cast<chrono::system_clock::duration>(
                    ftime - fs::file_time_type::clock::now() + chrono::system_clock::now()
                );

                if (now - sys_time > keepDuration) {
                    fs::remove(entry);
                }
            }
            catch (const std::exception& ex) {
                // Ignore or log file deletion errors
                OutputDebugStringA(("Log cleanup failed: " + std::string(ex.what()) + "\n").c_str());
            }
        }
    }


    // Main logging function
    int log(LogLevel level, const wchar_t* msg) {
        createDirectoryIfNotExists();
        cleanupOldLogs(7);  // Keep logs for 7 days

        std::wstring logPath = getLogFilePath();
        std::wofstream log(logPath, std::ios::app);
        if (!log.is_open()) {
            OutputDebugString(L"Failed to open daily log file.\n");
            return 0;
        }

        SYSTEMTIME st;
        GetLocalTime(&st);

        log << L"[" << (st.wHour < 10 ? L"0" : L"") << st.wHour << L":"
            << (st.wMinute < 10 ? L"0" : L"") << st.wMinute << L":"
            << (st.wSecond < 10 ? L"0" : L"") << st.wSecond << L"] "
            << L"[" << toString(level) << L"] "
            << msg << std::endl;

        if (AppConfiguration::getShowCoutConsole()) {
            std::wcout << L"[" << (st.wHour < 10 ? L"0" : L"") << st.wHour << L":"
                << (st.wMinute < 10 ? L"0" : L"") << st.wMinute << L":"
                << (st.wSecond < 10 ? L"0" : L"") << st.wSecond << L"] "
                << L"[" << toString(level) << L"] "
                << msg << std::endl;
        }

        return 1;
    }


    int log(LogLevel level, const wchar_t* msg1, const wchar_t* msg2) {
        std::wstringstream combinedMsg;
        combinedMsg << msg1 << msg2;

        // Convert wstringstream to wstring, then to c_str()
        std::wstring finalMsg = combinedMsg.str();
        log(level, finalMsg.c_str());
        return 1;
    }


    int log(LogLevel level, const wchar_t* msg1, const wchar_t* msg2, const wchar_t* msg3) {
        std::wstringstream combinedMsg;
        combinedMsg << msg1 << msg2 << msg3;
        std::wstring finalMsg = combinedMsg.str();
        log(level, finalMsg.c_str());
        return 1;
    }


    int log(LogLevel level, const std::string& msg) {
        log(level, StringUtil::convertStringToLPWSTR(msg.c_str()));
        return 1;
    }


    int log(LogLevel level, const std::string& msgFormat, DWORD arg1) {
        std::string msgStr = std::vformat(msgFormat, std::make_format_args(arg1));
        log(level, StringUtil::convertStringToLPWSTR(msgStr.c_str()));
        return 1;
    }


    int log(LogLevel level, const std::string& msgFormat, std::string arg1) {
        std::string msgStr = std::vformat(msgFormat, std::make_format_args(arg1));
        log(level, StringUtil::convertStringToLPWSTR(msgStr.c_str()));
        return 1;
    }


    int log(LogLevel level, const std::string& msgFormat, std::string arg1, std::string arg2) {
        std::string msgStr = std::vformat(msgFormat, std::make_format_args(arg1, arg2));
        log(level, StringUtil::convertStringToLPWSTR(msgStr.c_str()));
        return 1;
    }

}



