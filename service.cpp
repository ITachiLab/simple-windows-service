#include <Windows.h>
#include <tchar.h>
#include <strsafe.h>

#pragma comment(lib, "advapi32.lib")

#define SVCNAME TEXT("FooService")

SERVICE_STATUS svcStatus;
SERVICE_STATUS_HANDLE svcStatusHandle;
HANDLE ghSvcStopEvent = NULL;

void svcInstall();
void svcRemove();
void reportSvcStatus(DWORD, DWORD, DWORD);
void svcInit(DWORD, LPTSTR *);

void WINAPI svcCtrlHandler(DWORD);
void WINAPI svcMain(DWORD, LPTSTR *);


int wmain(int argc, wchar_t *argv[]) {
    if (argc == 1) {
        SERVICE_TABLE_ENTRY dispatchTable[] = {
                {SVCNAME, (LPSERVICE_MAIN_FUNCTION) svcMain},
                {nullptr, nullptr}
        };

        StartServiceCtrlDispatcher(dispatchTable);
    } else if (_wcsicmp(L"install", argv[1]) == 0) {
        svcInstall();
    } else if (_wcsicmp(L"remove", argv[1]) == 0) {
        svcRemove();
    }


    return 0;
}

void svcInstall() {
    TCHAR path[MAX_PATH];

    if (!GetModuleFileName(nullptr, path, MAX_PATH)) {
        printf("Cannot install service (%lu)\n", GetLastError());
        return;
    }

    SC_HANDLE schSCManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);

    if (schSCManager == nullptr) {
        printf("OpenSCManager failed (%lu)\n", GetLastError());
        return;
    }

    SC_HANDLE schService = CreateService(
            schSCManager,                 // SCM handle
            SVCNAME,                      // service name
            SVCNAME,                      // service display name
            SERVICE_ALL_ACCESS,           // access type
            SERVICE_WIN32_OWN_PROCESS,    // service type
            SERVICE_DEMAND_START,         // start type
            SERVICE_ERROR_NORMAL,         // error control type
            path,                         // path to service's binary
            nullptr,                      // no load ordering group
            nullptr,                      // no tag identifier
            nullptr,                      // no dependencies
            nullptr,                      // LocalSystem account
            nullptr);                     // no password

    if (schService == nullptr) {
        printf("CreateService failed (%lu)\n", GetLastError());
        CloseServiceHandle(schSCManager);
        return;
    } else {
        printf("Service installed successfully\n");
    }

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}

void svcRemove() {
    SC_HANDLE schSCManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);

    if (schSCManager != nullptr) {
        SC_HANDLE hService = OpenService(schSCManager, SVCNAME, SERVICE_ALL_ACCESS);

        if (hService != nullptr) {
            if (DeleteService(hService) == 0) {
                printf("Could not delete service: (%lu)\n", GetLastError());
            } else {
                printf("Service has been deleted\n");
            }
        }
    }
}

void WINAPI svcMain(DWORD dwArgc, LPTSTR *lpszArgv) {
    svcStatusHandle = RegisterServiceCtrlHandler(SVCNAME, svcCtrlHandler);

    if (svcStatusHandle) {
        svcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
        svcStatus.dwServiceSpecificExitCode = 0;

        reportSvcStatus(SERVICE_START_PENDING, NO_ERROR, 3000);

        svcInit(dwArgc, lpszArgv);
    }
}

void svcInit(DWORD dwArgc, LPTSTR *lpszArgv) {
    ghSvcStopEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

    if (ghSvcStopEvent == NULL) {
        reportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
        return;
    }

    reportSvcStatus(SERVICE_RUNNING, NO_ERROR, 0);

    while (true) {
        WaitForSingleObject(ghSvcStopEvent, INFINITE);

        reportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
        return;
    }
}

void reportSvcStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint) {
    static DWORD dwCheckPoint = 1;

    svcStatus.dwCurrentState = dwCurrentState;
    svcStatus.dwWin32ExitCode = dwWin32ExitCode;
    svcStatus.dwWaitHint = dwWaitHint;

    if (dwCurrentState == SERVICE_START_PENDING) {
        svcStatus.dwControlsAccepted = 0;
    } else {
        svcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    }

    if ((dwCurrentState == SERVICE_RUNNING) || (dwCurrentState == SERVICE_STOPPED)) {
        svcStatus.dwCheckPoint = 0;
    } else {
        svcStatus.dwCheckPoint = dwCheckPoint++;
    }

    SetServiceStatus(svcStatusHandle, &svcStatus);
}

void WINAPI svcCtrlHandler(DWORD dwCtrl) {
    switch (dwCtrl) {
        case SERVICE_CONTROL_STOP:
            reportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);
            SetEvent(ghSvcStopEvent);
        default:
            break;
    }
}