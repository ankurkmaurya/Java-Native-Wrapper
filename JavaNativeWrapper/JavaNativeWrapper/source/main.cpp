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


#pragma comment(lib, "Shell32.lib")



BOOL static isRunningInAdminMode() {
    BOOL isAdmin = FALSE;
    HANDLE token = nullptr;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
        TOKEN_ELEVATION elevation = {};
        DWORD size;

        if (GetTokenInformation(token, TokenElevation, &elevation, sizeof(elevation), &size)) {
            isAdmin = elevation.TokenIsElevated;
        }
        CloseHandle(token);
    }
    return isAdmin;
}


BOOL static elevatePermissionAsAdmin(bool log_enabled) {
    BOOL permissionElevated = FALSE;
    // Get the program path
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    log_enabled && Logger::log(LogLevel::INFO, "Attempting to elevate the application permission as Admin");

    // Relaunch with admin rights
    SHELLEXECUTEINFOA sei = { sizeof(sei) };
    sei.lpVerb = "runas";      // Causes UAC elevation prompt
    sei.lpFile = exePath;
    sei.hwnd = NULL;
    sei.nShow = SW_NORMAL;

    if (ShellExecuteExA(&sei)) {
        permissionElevated = TRUE;
    } else {
       DWORD error = GetLastError();
       if (error == ERROR_CANCELLED)
           log_enabled && Logger::log(LogLevel::INFO, "User Refused Admin Elevation.");
       else
           log_enabled && Logger::log(LogLevel::INFO, "Failed to elevate permission. Error: {}", error);
    }
    return permissionElevated;
}


// Application Entry point
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, 
        _In_ PWSTR pCmdLine, _In_ int nCmdShow) {

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
    int serviceMode = -2;
    if (argc > 1) {
        for (int i = 0; i < argc; i++) {
            if (_wcsicmp(argv[i], L"-log.enable") == 0) {
                log_enabled = true;
            }
            else if (_wcsicmp(argv[i], L"-log.disable") == 0) {
                log_enabled = false;
            }
            else if (_wcsicmp(argv[i], L"-service.uninstall") == 0) {
                serviceMode = -1;
            }
            else if (_wcsicmp(argv[i], L"-service.install") == 0) {
                serviceMode = 1;
            }
            else if (_wcsicmp(argv[i], L"-service.run") == 0) {
                serviceMode = 0;
            }
        }
    }
    ServiceControlManager::logEnabled = log_enabled;
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
        log_enabled && Logger::log(LogLevel::INFO, "Checking, Application need to be started with Admin privileges.");

        BOOL isInAdminMode = isRunningInAdminMode();
        if (isInAdminMode) {
            log_enabled && Logger::log(LogLevel::INFO, "Application is started with Admin privilege.");
        }
        else {
            log_enabled && Logger::log(LogLevel::INFO, "Application is not started with Admin privilege, Elevating the privilege.");
            BOOL permissionElevated = elevatePermissionAsAdmin(log_enabled);
            if (permissionElevated) {
                log_enabled && Logger::log(LogLevel::INFO, "Application started with Admin privilege, Exit this instance.");
                exit(EXIT_SUCCESS);
            } else {
                log_enabled && Logger::log(LogLevel::ERR, "Failed to start the Application with Admin privilege, Closing the instance.");
                exit(EXIT_FAILURE);
            }
        }
    }


    if (serviceMode == -1) {
        AppUtil::enableConsole();
        AppConfiguration::setShowCoutConsole(true);
        ServiceControlManager::svcUnInstall();
        system("pause");
    } 
    else if (serviceMode == 1) {
        AppUtil::enableConsole();
        AppConfiguration::setShowCoutConsole(true);
        ServiceControlManager::svcInstall();
        system("pause");
    }
    else {
        ServiceControlManager::dispatchServiceProcess();
    }

    exit(EXIT_SUCCESS);
}


