#include "stdafx.h"
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")

// Preserve the exception context so faults inside Windows libraries can be traced.
void CaptureClientCrash(EXCEPTION_POINTERS* exception)
{
    SYSTEMTIME timestamp = {};
    GetLocalTime(&timestamp);
    char path[MAX_PATH] = {};
    _snprintf_s(path, sizeof(path), _TRUNCATE,
        "CustomerService/client-%04u%02u%02u-%02u%02u%02u.dmp",
        timestamp.wYear, timestamp.wMonth, timestamp.wDay,
        timestamp.wHour, timestamp.wMinute, timestamp.wSecond);
    HANDLE file = CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_NEW,
        FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) return;
    MINIDUMP_EXCEPTION_INFORMATION info = {};
    info.ThreadId = GetCurrentThreadId();
    info.ExceptionPointers = exception;
    info.ClientPointers = FALSE;
    MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), file,
        static_cast<MINIDUMP_TYPE>(MiniDumpWithIndirectlyReferencedMemory |
            MiniDumpWithDataSegs), &info, NULL, NULL);
    CloseHandle(file);
}

static LONG WINAPI CaptureUnhandledClientException(EXCEPTION_POINTERS* exception)
{
    CaptureClientCrash(exception);
    return EXCEPTION_EXECUTE_HANDLER;
}

static LONG WINAPI ObserveClientException(EXCEPTION_POINTERS* exception)
{
    const DWORD code = exception->ExceptionRecord->ExceptionCode;
    if (code == EXCEPTION_ACCESS_VIOLATION || code == 0xC0000374 ||
        code == 0xC0000409)
    {
        static LONG captured = 0;
        if (InterlockedCompareExchange(&captured, 1, 0) == 0)
            CaptureClientCrash(exception);
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

void InitializeClientCrashCapture()
{
    CreateDirectoryA("CustomerService", NULL);
    SetUnhandledExceptionFilter(CaptureUnhandledClientException);
    AddVectoredExceptionHandler(1, ObserveClientException);
}
