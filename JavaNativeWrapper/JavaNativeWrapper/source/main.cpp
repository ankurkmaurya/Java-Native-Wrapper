/**
  ----- JavaNativeWrapper ----
  
  It wraps the java compiled binaries like .class files or .jar file 
  into executable windows binaries so that the java executables can 
  have the functionalities like other native window binaries.
  Java binaries wrapped though this utility can be made to run as a 
  windows service or can be seen as a executable with specific name
  in the task manager tool.

  This utility just loads and runs the already installed jvm binaries
  through native code by first configuring the java binaries 
  (.jar or .class) through the configurable file '.config' which is 
  required to run this utility.

*/


#include "pch.h"

#include <stdio.h>
#include <tchar.h>
#include <cwchar>
#include <windows.h>
#include <shellapi.h>
#include <fstream>

#include "logger.h"
#include "appconfiguration.h"
#include "servicecontrolmanager.h"
#include "javavirtualmachine.h"


//To check if the executable is started by Service Control Manager (SCM)
bool isStartedBySCM() {
    // Check if the process has a console window
    HWND hWnd = GetConsoleWindow();

    // Also check standard handles
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

    // If both are NULL/invalid, most likely SCM
    return (hWnd == NULL && hStdOut == INVALID_HANDLE_VALUE);
}



// Application Entry point
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, 
        _In_ PWSTR pCmdLine, _In_ int nCmdShow) {

    //If the executable is started by Service Control Manager (SCM) enable the Console
    if (!isStartedBySCM()) {
        AppUtil::enableConsole();
        AppConfiguration::setShowCoutConsole(true);
    }

    //Set the Current Working Directory as the directory from where the binary image has been launched
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    std::filesystem::path p(exePath);
    std::filesystem::path executable_dir = p.parent_path();
    std::filesystem::path executable_file_name = p.filename();
    SetCurrentDirectory(executable_dir.c_str());

    std::string configFileName = StringUtil::replaceAll(StringUtil::convertLPWSTRToString(executable_file_name.c_str()), 
        ".exe", AppConfiguration::getConfigFileExtension());
    AppConfiguration::setConfigFileName(configFileName);

    //Convert the whole lpCmdLine string to multiple arguments
    int argc;
    LPWSTR* szArglist = CommandLineToArgvW(GetCommandLineW(), &argc);
    wchar_t** argv = new wchar_t* [argc];
    for (int i = 0; i < argc; i++) {
        int lgth = wcslen(szArglist[i]);
        argv[i] = new wchar_t[lgth + 1];
        for (int j = 0; j <= lgth; j++)
            argv[i][j] = wchar_t(szArglist[i][j]);
    }
    LocalFree(szArglist);

    //Identify and Initialize Runtime Arguments
    bool log_enabled = false;
    int serviceMode = 0;
    if (argc > 1) {
        for (int i = 0; i < argc; i++) {
            if (_wcsicmp(argv[i], L"-log.enable") == 0) {
                log_enabled = true;
            }
            else if (_wcsicmp(argv[i], L"-log.disable") == 0) {
                log_enabled = false;
            }
            else if (_wcsicmp(argv[i], L"-service.install") == 0) {
                serviceMode = 1;
            }
            else if (_wcsicmp(argv[i], L"-service.uninstall") == 0) {
                serviceMode = -1;
            } 
        }
    }
    log_enabled && Logger::log(LogLevel::INFO, "Runtime Arguments configured");
    log_enabled && Logger::log(LogLevel::INFO, "Starting Main");

    AppConfiguration::initializeAppConfiguration(log_enabled);

    //Redirect console output and error to Log File
    if (AppConfiguration::getEnableJNWSysOutLog()) {
        log_enabled && Logger::log(LogLevel::INFO, "Redirecting Console To Log File");
        FileUtil::redirectConsoleToLogFile();
    }


    //Check if the application configured to be run with Administrator priviledges.
    if (AppConfiguration::getRunJNWWithAdminPrivilege()) {
        Logger::log(LogLevel::INFO, "Checking, Application need to be started with Admin priviledges.");

        BOOL isAdmin = FALSE;
        HANDLE token = nullptr;
        if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
            TOKEN_ELEVATION elevation;
            DWORD size;

            if (GetTokenInformation(token, TokenElevation, &elevation, sizeof(elevation), &size)) {
                isAdmin = elevation.TokenIsElevated;
            }
            CloseHandle(token);
        }

        if (!isAdmin) {
            Logger::log(LogLevel::ERR, "Application is not started with Admin priviledge, Stopping the application.");
            exit(EXIT_FAILURE);
        }
        else {
            Logger::log(LogLevel::ERR, "Application is started with Admin priviledge.");
        }
    }


    ServiceControlManager::logEnabled = log_enabled;
    if (serviceMode == -1) {
        ServiceControlManager::svcUnInstall();
        system("pause");
    } else if (serviceMode == 1) {
        ServiceControlManager::svcInstall();
        system("pause");
    } else {
        ServiceControlManager::dispatchServiceProcess();
    }

    exit(EXIT_SUCCESS);
}


