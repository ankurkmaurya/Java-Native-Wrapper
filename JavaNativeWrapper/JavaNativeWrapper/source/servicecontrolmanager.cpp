
#include "pch.h"

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <string>

#include "logger.h"
#include "servicecontrolmanager.h"
#include "javavirtualmachine.h"

#pragma comment(lib, "advapi32.lib")


bool  ServiceControlManager::logEnabled = false;
LPWSTR ServiceControlManager::serviceName = nullptr;
SERVICE_STATUS ServiceControlManager::gSvcStatus = {}; 
SERVICE_STATUS_HANDLE ServiceControlManager::gSvcStatusHandle = {};
HANDLE ServiceControlManager::ghSvcStopEvent;


/*
 *
 Purpose:
  Runs the service executable in console mode for testing or verifying the application

 Parameters:
  serviceName - Name of the service configured for running

 Return value:
  None
 *
 */
void ServiceControlManager::runServiceInConsoleMode(const std::string& serviceName) {
    if (AppConfiguration::getEnableJNWConsole()) {
        //Enable Console if configured
        AppUtil::enableConsole();
        AppConfiguration::setShowCoutConsole(true);
    }

    ServiceControlManager::logEnabled && Logger::log(LogLevel::INFO, "Service '{}' is running in test/console mode", serviceName);
    JavaVirtualMachine::startJavaVirtualMachine(ServiceControlManager::logEnabled);
    ServiceControlManager::logEnabled && Logger::log(LogLevel::INFO, "Service '{}' is closed.", serviceName);
}



/*
 * 
 Purpose:
  Check if the service is running

 Parameters:
  schService - Handle to the service in query

 Return value:
  Boolean - TRUE if Running, Otherwise FALSE
 *
 */
BOOL ServiceControlManager::isServiceRunning(SC_HANDLE schService) {
    DWORD dwBytesNeeded;
    SERVICE_STATUS_PROCESS ssp{};

    if (!QueryServiceStatusEx(schService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded)) {
        ServiceControlManager::logEnabled && Logger::log(LogLevel::ERR, "Service Status Query Failed.");
        return FALSE;
    }

    if (ssp.dwCurrentState == SERVICE_RUNNING)
    {
        ServiceControlManager::logEnabled && Logger::log(LogLevel::INFO, "Service is Running.");
        return TRUE;
    }
    else {
        ServiceControlManager::logEnabled && Logger::log(LogLevel::INFO, "Service is not Running.");
        return FALSE;
    }
}



/*
 * 
  Purpose:
   Logs messages to the event log

 Parameters:
   szFunction - name of function that failed

 Return value:
   None

 Remarks:
   The service must have an entry in the Application event log.
 *
 */
void ServiceControlManager::svcReportEvent(LPTSTR szFunction)
{
    HANDLE hEventSource;
    LPCTSTR lpszStrings[2]{};
    TCHAR Buffer[80];

    hEventSource = RegisterEventSource(NULL, ServiceControlManager::serviceName);

    if (NULL != hEventSource)
    {
        StringCchPrintf(Buffer, 80, TEXT("%s failed with %d"), szFunction, GetLastError());

        lpszStrings[0] = ServiceControlManager::serviceName;
        lpszStrings[1] = Buffer;

        ReportEvent(hEventSource, // event log handle
            EVENTLOG_ERROR_TYPE,  // event type
            0,                    // event category
            NULL,                 // event identifier
            NULL,                 // no security identifier
            2,                    // size of lpszStrings array
            0,                    // no binary data
            lpszStrings,          // array of strings
            NULL);                // no binary data

        DeregisterEventSource(hEventSource);
    }
}



/*
 * 
 Purpose:
   Sets the current service status and reports it to the SCM.

 Parameters:
   dwCurrentState  - The current state (see SERVICE_STATUS)
   dwWin32ExitCode - The system error code
   dwWaitHint      - Estimated time for pending operation, in milliseconds

 Return value:
   None
 *
 */
void ServiceControlManager::reportSvcStatus(DWORD dwCurrentState,
                                            DWORD dwWin32ExitCode,
                                            DWORD dwWaitHint)
{
    static DWORD dwCheckPoint = 1;

    // Fill in the SERVICE_STATUS structure.
    ServiceControlManager::gSvcStatus.dwCurrentState = dwCurrentState;
    ServiceControlManager::gSvcStatus.dwWin32ExitCode = dwWin32ExitCode;
    ServiceControlManager::gSvcStatus.dwWaitHint = dwWaitHint;

    if (dwCurrentState == SERVICE_START_PENDING)
        ServiceControlManager::gSvcStatus.dwControlsAccepted = 0;
    else ServiceControlManager::gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

    if ((dwCurrentState == SERVICE_RUNNING) ||
        (dwCurrentState == SERVICE_STOPPED))
        ServiceControlManager::gSvcStatus.dwCheckPoint = 0;
    else ServiceControlManager::gSvcStatus.dwCheckPoint = dwCheckPoint++;

    // Report the status of the service to the SCM.
    SetServiceStatus(ServiceControlManager::gSvcStatusHandle, &(ServiceControlManager::gSvcStatus));
}



/*
 * 
    Purpose:
     Installs(Add) a service in the Service Control Manager (SCM) database

    Parameters:
     appConfiguration - Application configuration information from config.
     log_enabled      - Control the printing of logs in log file.

    Return value:
     None
 *
 */
void ServiceControlManager::svcInstall()
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    TCHAR szUnquotedPath[MAX_PATH];

    if (!GetModuleFileName(NULL, szUnquotedPath, MAX_PATH))
    {
        ServiceControlManager::logEnabled && Logger::log(LogLevel::ERR, "Cannot install Service ({})", GetLastError());
        std::wcout << L"[ERROR] Cannot install Service (" << GetLastError() << ")" << std::endl;
        return;
    }

    /*
     * 
    In case the path contains a space, it must be quoted so that it is correctly interpreted.
    For example:
      "d:\my share\myservice.exe" should be specified as ""d:\my share\myservice.exe"".
     *
     */
    TCHAR szPath[MAX_PATH];
    if (ServiceControlManager::logEnabled) {
       StringCbPrintf(szPath, MAX_PATH, TEXT("\"%s\" -log.enable"), szUnquotedPath);
    } else {
       StringCbPrintf(szPath, MAX_PATH, TEXT("\"%s\""), szUnquotedPath);
    }

   


    // Get a handle to the SCM database. 
    schSCManager = OpenSCManager(
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 

    if (NULL == schSCManager)
    {
        ServiceControlManager::logEnabled && Logger::log(LogLevel::ERR, "OpenSCManager failed ({})", GetLastError());
        std::wcout << L"[ERROR] OpenSCManager failed (" << GetLastError() << ")" << std::endl;
        return;
    }

    //Determine the Service Name
    std::string serviceNameStr = (AppConfiguration::getJnwServiceName() == "") ? SERVICE_NAME_DEFAULT : AppConfiguration::getJnwServiceName();
    ServiceControlManager::serviceName = StringUtil::convertStringToLPWSTR(serviceNameStr);

    //Determine the Service Display Name
    std::string serviceNameDisplayStr = (AppConfiguration::getJnwServiceNameDisplay() == "") ? SERVICE_NAME_DISPLAY_DEFAULT : AppConfiguration::getJnwServiceNameDisplay();
    LPWSTR serviceNameDisplay = StringUtil::convertStringToLPWSTR(serviceNameDisplayStr);

    //Determine the Service Display Name
    std::string serviceDescriptionStr = (AppConfiguration::getJnwServiceDescription() == "") ? SERVICE_DESCRIPTION : AppConfiguration::getJnwServiceDescription();
    LPSTR serviceDescriptionBuffer = serviceDescriptionStr.data();

    // Create the service
    schService = CreateService(
        schSCManager,              // SCM database 
        ServiceControlManager::serviceName, // name of service 
        serviceNameDisplay,        // service name to display 
        SERVICE_ALL_ACCESS,        // desired access 
        SERVICE_WIN32_OWN_PROCESS, // service type 
        SERVICE_AUTO_START,        // start type 
        SERVICE_ERROR_NORMAL,      // error control type 
        szPath,                    // path to service's binary 
        NULL,                      // no load ordering group 
        NULL,                      // no tag identifier 
        NULL,                      // no dependencies 
        NULL,                      // LocalSystem account 
        NULL);                     // no password 

    if (schService == NULL)
    {
        ServiceControlManager::logEnabled&& Logger::log(LogLevel::ERR, "CreateService failed ({})", GetLastError());
        std::wcout << L"[ERROR] CreateService failed (" << GetLastError() << ")" << std::endl;
        CloseServiceHandle(schSCManager);
        return;
    }
    else {
        SERVICE_DESCRIPTIONA serviceDescription;
        ZeroMemory(&serviceDescription, sizeof(serviceDescription));
        serviceDescription.lpDescription = serviceDescriptionBuffer;

        if (!ChangeServiceConfig2A(schService, SERVICE_CONFIG_DESCRIPTION, &serviceDescription)) {
            ServiceControlManager::logEnabled && Logger::log(LogLevel::ERR, "Service Description change failed");
            std::wcout << L"[ERROR] Service Description change failed" << std::endl;
        }

        ServiceControlManager::logEnabled && 
            Logger::log(LogLevel::INFO, L"Service Installed Successfully : ", ServiceControlManager::serviceName);
            std::wcout << L"[INFO] Service Installed Successfully : " << ServiceControlManager::serviceName << std::endl;
    }

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}



/*
 * 
    Purpose:
     UnInstalls(Delete) a service from Service Control Manager (SCM) database

    Parameters:
     appConfiguration - Application configuration information from config.
     log_enabled      - Control the printing of logs in log file.

    Return value:
     None
 *
 */
void ServiceControlManager::svcUnInstall()
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;

    // Get a handle to the SCM database. 
    schSCManager = OpenSCManager(
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 

    if (NULL == schSCManager)
    {
        ServiceControlManager::logEnabled && Logger::log(LogLevel::ERR, "OpenSCManager failed ({})", GetLastError());
        std::wcout << L"[ERROR] OpenSCManager failed : " << GetLastError() << std::endl;
        return;
    }

    //Determine the Service Name
    std::string serviceNameStr = (AppConfiguration::getJnwServiceName() == "") ? SERVICE_NAME_DEFAULT : AppConfiguration::getJnwServiceName();
    ServiceControlManager::serviceName = StringUtil::convertStringToLPWSTR(serviceNameStr);

    // Get handle to the existing service. 
    schService = OpenServiceW(schSCManager, ServiceControlManager::serviceName, SERVICE_ALL_ACCESS);
    if (schService == NULL)
    {
        ServiceControlManager::logEnabled && Logger::log(LogLevel::ERR, " Open Service failed ({})", GetLastError());
        std::wcout << L"[ERROR] Open Service failed : " << GetLastError() << std::endl;
        CloseServiceHandle(schSCManager);
        return;
    }
    else {
        //Removing the Service
        SERVICE_STATUS sStatus;
        ZeroMemory(&sStatus, sizeof(sStatus));

        //First stop the service if it is Running currently
        if (ServiceControlManager::isServiceRunning(schService) && 
            !ControlService(schService, SERVICE_CONTROL_STOP, &sStatus)) {
            ServiceControlManager::logEnabled && Logger::log(LogLevel::ERR, "Failed to send Service Stop command.");
            std::wcout << L"[ERROR] Failed to send Service Stop command." << std::endl;
            return;
        }

        Sleep(500);
        if (sStatus.dwCurrentState == SERVICE_STOPPED || sStatus.dwCurrentState == SERVICE_STOP_PENDING)
        {
            ServiceControlManager::logEnabled && Logger::log(LogLevel::INFO, "Service Stopped Successfully.");
            std::wcout << L"[INFO] Service Stopped Successfully." << std::endl;
        }

        if (!DeleteService(schService)) {
            ServiceControlManager::logEnabled && Logger::log(LogLevel::ERR, "Failed to Remove Service.");
            std::wcout << L"[ERROR] Failed to Remove Service." << std::endl;
            return;
        }
        ServiceControlManager::logEnabled && 
            Logger::log(LogLevel::INFO, L"Service Removed Successfully : ", ServiceControlManager::serviceName);
        std::wcout << L"[INFO] Service Removed Successfully : " << ServiceControlManager::serviceName << std::endl;
    }

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}



/*
 * 
  Purpose:
   Called by SCM whenever a control code is sent to the service
   using the ControlService function.

 Parameters:
   dwCtrl - control code

 Return value:
   None
 *
 */
VOID WINAPI ServiceControlManager::svcCtrlHandler(DWORD dwCtrl)
{
    // Handle the requested control code. 
    switch (dwCtrl) {
        case SERVICE_CONTROL_STOP:
            if (ServiceControlManager::gSvcStatus.dwCurrentState != SERVICE_RUNNING)
                break;

            ServiceControlManager::reportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);
            SetEvent(ServiceControlManager::ghSvcStopEvent);
            break;

        default:
            break;
    }
}


DWORD WINAPI ServiceControlManager::serviceWorkerThread(LPVOID lpParam)
{
    ServiceControlManager::logEnabled &&
        Logger::log(LogLevel::INFO, L"Worker Thread Started.");

    // Start JVM
    JavaVirtualMachine::startJavaVirtualMachine(ServiceControlManager::logEnabled);

    ServiceControlManager::logEnabled&&
        Logger::log(LogLevel::INFO, L"JVM Stopped");
    return 0;
}


/*
 Purpose:
   The service function code

 Parameters:
   dwArgc   - Number of arguments in the lpszArgv array
   lpszArgv - Array of strings. The first string is the name of
              the service and subsequent strings are passed by the process
              that called the StartService function to start the service.

 Return value:
   None
*/
void ServiceControlManager::svcInit(DWORD dwArgc, LPTSTR* lpszArgv)
{
    /*
       Create an event.The control handler function, SvcCtrlHandler,
       signals this event when it receives the stop control code.
    */
    ServiceControlManager::ghSvcStopEvent = CreateEvent(
        NULL,    // default security attributes
        TRUE,    // manual reset event
        FALSE,   // not signaled
        NULL);   // no name

    if (ServiceControlManager::ghSvcStopEvent == NULL) {
        ServiceControlManager::logEnabled && Logger::log(LogLevel::ERR, L"Failed to create Event.");
        ServiceControlManager::reportSvcStatus(SERVICE_STOPPED, GetLastError(), 0);
        return;
    }
    ServiceControlManager::reportSvcStatus(SERVICE_START_PENDING, NO_ERROR, 5000);

    // Start the service work in a background thread
    HANDLE hThread = CreateThread(
        NULL, 0,
        ServiceControlManager::serviceWorkerThread,
        NULL, 0, NULL);

    if (hThread == NULL)
    {
        ServiceControlManager::logEnabled && Logger::log(LogLevel::ERR, L"Failed to create Worker Thread.");
        ServiceControlManager::reportSvcStatus(SERVICE_STOPPED, GetLastError(), 0);
        return;
    }

    // Report service is running
    ServiceControlManager::reportSvcStatus(SERVICE_RUNNING, NO_ERROR, 0);
    ServiceControlManager::logEnabled && Logger::log(LogLevel::INFO, L"Service Running");

    // BLOCK here until stop event is signaled
    WaitForSingleObject(ServiceControlManager::ghSvcStopEvent, INFINITE);

    // Optional: Wait for the worker thread to finish
    if (hThread) {
        CloseHandle(hThread);
        ServiceControlManager::logEnabled && Logger::log(LogLevel::INFO, L"Worker Thread Closed.");
    }

    // Service is stopping now
    ServiceControlManager::reportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
    ServiceControlManager::logEnabled && Logger::log(LogLevel::INFO, L"Service stopped");
    return;
}


/*
 * 
 Purpose:
   Entry point for the service

 Parameters:
   dwArgc   - Number of arguments in the lpszArgv array
   lpszArgv - Array of strings. The first string is the name of the service
              and subsequent strings are passed by the process that called 
              the StartService function to start the service.

 Return value:
   None.
*
*/
static void WINAPI svcMain(DWORD dwArgc, LPWSTR* lpszArgv)
{
    ServiceControlManager::logEnabled && Logger::log(LogLevel::ERR, "Application is started by Service Control Manager (SCM).");
    // Register the handler function for the service
    ServiceControlManager::gSvcStatusHandle = RegisterServiceCtrlHandler(
        ServiceControlManager::serviceName,
        ServiceControlManager::svcCtrlHandler);

    if (!ServiceControlManager::gSvcStatusHandle)
    {
        ServiceControlManager::logEnabled && Logger::log(LogLevel::ERR, L"Failed to Register Service Ctrl Handler.");
        ServiceControlManager::svcReportEvent((LPTSTR)TEXT("RegisterServiceCtrlHandler"));
        return;
    }

    // These SERVICE_STATUS members remain as set here
    ServiceControlManager::gSvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    ServiceControlManager::gSvcStatus.dwServiceSpecificExitCode = 0;

    // Report initial status to the SCM
    ServiceControlManager::reportSvcStatus(SERVICE_START_PENDING, NO_ERROR, 10000);

    //Perform service-specific initialization and work.
    ServiceControlManager::svcInit(dwArgc, lpszArgv);
}


void ServiceControlManager::dispatchServiceProcess() {
    ServiceControlManager::logEnabled && Logger::log(LogLevel::INFO, "Dispatch Service Process Called.");

    //Determine the Service Name
    std::string serviceNameStr = (AppConfiguration::getJnwServiceName() == "") ? SERVICE_NAME_DEFAULT : AppConfiguration::getJnwServiceName();
    ServiceControlManager::serviceName = StringUtil::convertStringToLPWSTR(serviceNameStr);

    ServiceControlManager::logEnabled && Logger::log(LogLevel::INFO, L"Starting Service: ", ServiceControlManager::serviceName);

    // TO_DO: Add any additional services for the process to this table.
    SERVICE_TABLE_ENTRY dispatchTable[] = 
    {
        { ServiceControlManager::serviceName, (LPSERVICE_MAIN_FUNCTION)svcMain },
        { nullptr, nullptr }
    };

    // This call returns when the service has stopped. 
    // The process should simply terminate when the call returns.
    if (!StartServiceCtrlDispatcher(dispatchTable)) {
        DWORD err = GetLastError();
        if (err == ERROR_FAILED_SERVICE_CONTROLLER_CONNECT) {
            ServiceControlManager::logEnabled && Logger::log(LogLevel::ERR, "Failed to Connect to Service Controller ({})", err);
            // Not started by SCM — likely run manually
            runServiceInConsoleMode(serviceNameStr);
        }
        else {
            ServiceControlManager::logEnabled && Logger::log(LogLevel::ERR, "Failed to Start Service Ctrl Dispatcher ({})", err);
            ServiceControlManager::svcReportEvent((LPTSTR)TEXT("StartServiceCtrlDispatcher"));
        }
    }
    ServiceControlManager::logEnabled && Logger::log(LogLevel::INFO, "Dispatch Service Process Returned.");
}





