#pragma once

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>

#include "appconfiguration.h"
#include "javavirtualmachine.h"


#define SERVICE_NAME_DEFAULT            "JavaNativeWrapper"
#define SERVICE_NAME_DISPLAY_DEFAULT    "Java Native Wrapper"
#define SERVICE_DESCRIPTION             "Native executable wrapper for java compiled binaries"


class ServiceControlManager
{
   public:
	    static bool                    logEnabled;
		static LPWSTR                  serviceName;
		static SERVICE_STATUS          gSvcStatus;
		static SERVICE_STATUS_HANDLE   gSvcStatusHandle;
		static HANDLE                  ghSvcStopEvent;


		static void runServiceInConsoleMode(const std::string& serviceName);
		static void reportSvcStatus(DWORD, DWORD, DWORD);
		static void svcReportEvent(LPTSTR);
		static BOOL isServiceRunning(SC_HANDLE);


		static void svcInstall();
		static void svcUnInstall();
		static VOID WINAPI svcCtrlHandler(DWORD);
		static void svcInit(DWORD, LPTSTR*);
		static void dispatchServiceProcess();

		static DWORD WINAPI serviceWorkerThread(LPVOID lpParam);
};

