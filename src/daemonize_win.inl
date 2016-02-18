/*
* daemonize_win.inl
* oschina.net subversion proxy service
* author: Force.Charlie
* Date: 2016.02
* Copyright (C) 2016. OSChina.NET. All Rights Reserved.
*/
#ifndef _WIN32
#error "This file only supports the Windows platform"
#endif
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <string>
#include <io.h>
#include <stdio.h>
#ifdef _MSC_VER
#include <Winternl.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <sdkddkver.h>
#if defined(_WIN32_WINNT_WIN8) && defined(_WIN32_WINNT) &&                     \
    _WIN32_WINNT >= _WIN32_WINNT_WIN8
#include <Processthreadsapi.h>
#include <comutil.h>
#include <wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "comsuppw.lib")
template <class Interface>
inline void SafeRelease(Interface **ppInterfaceToRelease) {
  if (*ppInterfaceToRelease != NULL) {
    (*ppInterfaceToRelease)->Release();
    (*ppInterfaceToRelease) = NULL;
  }
}
BOOL GetProcessCommandline(DWORD pid, std::wstring &cmdline) {
  HRESULT hr = 0;
  BOOL ret = FALSE;
  IWbemLocator *WbemLocator = NULL;
  IWbemServices *WbemServices = NULL;
  IEnumWbemClassObject *EnumWbem = NULL;

  // initializate the Windows security
  hr = CoInitializeEx(0, COINIT_MULTITHREADED);
  hr = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT,
                            RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
  hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
                        IID_IWbemLocator, (LPVOID *)&WbemLocator);
  if (SUCCEEDED(hr)) {
    hr = WbemLocator->ConnectServer(L"ROOT\\CIMV2", NULL, NULL, NULL, 0, NULL,
                                    NULL, &WbemServices);
    if (SUCCEEDED(hr)) {
      hr = WbemServices->ExecQuery(
          L"WQL",
          L"SELECT ProcessId, CommandLine, ExecutablePath FROM Win32_Process",
          WBEM_FLAG_FORWARD_ONLY, NULL, &EnumWbem);
      if (SUCCEEDED(hr)) {
        if (EnumWbem != NULL) {
          IWbemClassObject *result = NULL;
          ULONG returnedCount = 0;

          while ((hr = EnumWbem->Next(WBEM_INFINITE, 1, &result,
                                      &returnedCount)) == S_OK) {
            VARIANT ProcessId;
            VARIANT CommandLine;
            VARIANT ExecutablePath;

            // access the properties
            hr = result->Get(L"ProcessId", 0, &ProcessId, 0, 0);
            hr = result->Get(L"CommandLine", 0, &CommandLine, 0, 0);
            hr = result->Get(L"ExecutablePath", 0, &ExecutablePath, 0, 0);
            if (ProcessId.uintVal == pid) {
              _bstr_t va(CommandLine.bstrVal);
              const wchar_t *ptr = va;
              cmdline = ptr;
              ret = TRUE;
              result->Release();
              goto COMRelease;
            }
            result->Release();
          }
        }
      }
    }
  }

COMRelease:
  SafeRelease(&EnumWbem);
  SafeRelease(&WbemServices);
  SafeRelease(&WbemLocator);
  CoUninitialize();
  return ret;
}
#else
#include <Winternl.h>
#include <Windows.h>
typedef NTSTATUS(WINAPI *ZwQueryInformationProcessPtr)(
    _In_ HANDLE ProcessHandle, _In_ PROCESSINFOCLASS ProcessInformationClass,
    _Out_ PVOID ProcessInformation, _In_ ULONG ProcessInformationLength,
    _Out_opt_ PULONG ReturnLength);

BOOL GetProcessCommandline(DWORD pid, std::wstring &cmdline) {
  //
  HMODULE hMod = GetModuleHandleW(L"Ntdll.dll");
  BOOL result = false;
  ZwQueryInformationProcessPtr ZwQueryInformationProcess =
      (ZwQueryInformationProcessPtr)GetProcAddress(hMod,
                                                   "ZwQueryInformationProcess");
  if (ZwQueryInformationProcess == NULL) {
    return false;
  }
  auto hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ |
                                  PROCESS_TERMINATE,
                              FALSE, pid);
  if (hProcess == nullptr)
    return false;
  PROCESS_BASIC_INFORMATION processinfo;
  ZeroMemory(&processinfo, sizeof(processinfo));
  ULONG returnLength = 0;
  if (ZwQueryInformationProcess(hProcess, ProcessBasicInformation, &processinfo,
                                sizeof(PROCESS_BASIC_INFORMATION),
                                &returnLength) == ERROR_SUCCESS) {
    PEB peb;
    if (ReadProcessMemory(hProcess, processinfo.PebBaseAddress, &peb,
                          sizeof(PEB), NULL)) {
      RTL_USER_PROCESS_PARAMETERS rtlProcessParameters;
      ZeroMemory(&rtlProcessParameters, sizeof(rtlProcessParameters));
      if (ReadProcessMemory(hProcess, peb.ProcessParameters,
                            &rtlProcessParameters,
                            sizeof(RTL_USER_PROCESS_PARAMETERS), NULL)) {
        PWSTR wBuffer = rtlProcessParameters.CommandLine.Buffer;
        USHORT len = rtlProcessParameters.CommandLine.Length;
        PWSTR wBufferCopy = (PWSTR)malloc(len);
        result = ReadProcessMemory(hProcess, wBuffer,
                                   wBufferCopy, // command line goes here
                                   len, NULL);
        if (result) {
          cmdline.assign(wBufferCopy, len / sizeof(WCHAR));
        }
        free(wBufferCopy);
      }
    }
  }
  CloseHandle(hProcess);
  return result;
}
#endif
///
static DWORD check_pid(const char *pidfile) {
  int pid = 0;
  FILE *fp = fopen(pidfile, "r");
  if (fp == nullptr)
    return 0;
  int n = fscanf(fp, "%" PRIu32 "", &pid);
  fclose(fp);
  if (n != 1 || pid == 0 || pid == GetCurrentProcessId()) {
    return 0;
  }
  HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
  if (hProcess == NULL) {
    return 0;
  }
  CloseHandle(hProcess);
  return pid;
}

// store pid to pidfile
static bool write_pid(const char *pidfile) {
  FILE *fp = nullptr;
  if ((fp = fopen(pidfile, "w+")) == nullptr) {
    klogger::Log(klogger::kFatal, "cannot open %s", pidfile);
    return false;
  }
  fprintf(fp, "%d", GetCurrentProcessId());
  fclose(fp);
  return true;
}

int Daemonize(const std::string &pidfile) {
  auto pid = check_pid(pidfile.c_str());
  if (pid != 0) {
    fprintf(stderr, "svnsrv is already running, daemon pid = %" PRIu32 ".\n",
            pid);
    return 1;
  }
  // freopen stdout stdin to nul
  // close console window
  if (!freopen("nul", "w+b", stdout)) {
    return -1;
  }
  if (!freopen("nul", "r+b", stdin)) {
    return -1;
  }
  FreeConsole();
  if (write_pid(pidfile)) {
    return 0;
  }
  return -2;
}

bool DaemonWait(int Argc, char **Argv, bool crashRestart) {
  (void)crashRestart;
  return 0;
}

void WhenExit() {
  klogger::Destroy("svnsrv shutdown");
  //_exit(0);
}

int DaemonSignalMethod() {
  atexit(WhenExit);
  return 0;
}

int ForegroundSignalMethod() {
  atexit(WhenExit);
  return 0;
}

///////////
bool DaemonStop(const std::string &pidFile) {
  auto pid = check_pid(pidFile.c_str());
  if (pid == 0) {
    perror("svnsrv daemon not runing !\n");
    return false;
  }
  HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
  if (hProcess == NULL) {
    perror("svnsrv daemon not runing !\n");
    return false;
  }
  if (TerminateProcess(hProcess, 0) == 0) {
    perror("terminate svnsrv failed !\n");
    CloseHandle(hProcess);
  }
  printf("stop svnsrv daemon success !\n");
  CloseHandle(hProcess);
  _unlink(pidFile.c_str());
  return true;
}

bool DaemonRestart(const std::string &pidFile) {
  std::wstring cmdline;
  auto pid = check_pid(pidFile.c_str());
  if (pid == 0) {
    perror("svnsrv daemon not runing !\n");
    return false;
  }
  if (!GetProcessCommandline(pid, cmdline)) {
    perror("get svnsrv daemon commandline failed !\n");
    return false;
  }
  HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
  if (hProcess == NULL) {
    perror("svnsrv daemon not runing !\n");
    CloseHandle(hProcess);
    return false;
  }
  if (TerminateProcess(hProcess, 0) == 0) {
    perror("terminate svnsrv failed !\n");
  }
  CloseHandle(hProcess);
  _unlink(pidFile.c_str());
  // printf("stop svnsrv daemon success !\n");
  return 0;
}
