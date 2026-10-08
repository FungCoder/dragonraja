#pragma once
#include <winsock2.h>
#include <windows.h>
#include <cerrno>
#include <climits>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

struct OperationsRates {
    int experiencePercent = 100;
    int dropPercent = 100;
    bool initialized = false;
    bool reported = false;
};

inline OperationsRates& CurrentOperationsRates()
{
    static OperationsRates rates;
    return rates;
}

inline bool ParseOperationsPercent(const WCHAR* value, int& percent)
{
    if (!value || !*value) return false;
    for (const WCHAR* cursor = value; *cursor; ++cursor) if (*cursor < L'0' || *cursor > L'9') return false;
    errno = 0;
    WCHAR* end = NULL;
    const long parsed = wcstol(value, &end, 10);
    if (errno == ERANGE || !end || *end || parsed < 10 || parsed > 1000) return false;
    percent = static_cast<int>(parsed);
    return true;
}

inline bool ScaleOperationsExperience(double base, int percent, int& awarded)
{
    const double value = base * percent / 100.0;
    if (percent < 10 || percent > 1000 || !std::isfinite(value) || base < 0 || value < 0 || value > INT_MAX) return false;
    awarded = static_cast<int>(value);
    return true;
}

// An integer number of full draws plus one Bernoulli draw preserves expected count.
// At 1x no extra random number is consumed by the caller.
inline bool ScaleOperationsDraws(int base, int percent, int randomPercent, int& draws)
{
    if (base < 0 || percent < 10 || percent > 1000 || randomPercent < 0 || randomPercent >= 100) return false;
    const __int64 hundredths = static_cast<__int64>(base) * percent;
    const __int64 scaled = hundredths / 100 + (randomPercent < hundredths % 100 ? 1 : 0);
    if (scaled > INT_MAX) return false;
    draws = static_cast<int>(scaled);
    return true;
}

inline bool InitializeOperationsRates()
{
    WCHAR directory[MAX_PATH] = {};
    const DWORD length = GetModuleFileNameW(NULL, directory, MAX_PATH);
    if (!length || length >= MAX_PATH) return false;
    WCHAR* separator = wcsrchr(directory, L'\\');
    if (!separator) return false;
    separator[1] = 0;
    WCHAR configuration[MAX_PATH] = {}, report[MAX_PATH] = {}, temporary[MAX_PATH] = {};
    if (swprintf_s(configuration, L"%sMapServer.ini", directory) < 0 ||
        swprintf_s(report, L"%soperations-loaded.json", directory) < 0 ||
        swprintf_s(temporary, L"%soperations-loaded.json.tmp", directory) < 0) return false;
    WCHAR experience[32] = {}, drops[32] = {};
    GetPrivateProfileStringW(L"Operations", L"CombatExperiencePercent", L"100", experience, 32, configuration);
    GetPrivateProfileStringW(L"Operations", L"NpcDropDrawPercent", L"100", drops, 32, configuration);
    OperationsRates& rates = CurrentOperationsRates();
    if (!ParseOperationsPercent(experience, rates.experiencePercent) || !ParseOperationsPercent(drops, rates.dropPercent)) return false;
    rates.initialized = true;
    char body[180] = {};
    const int size = sprintf_s(body, "{\"ProcessId\":%lu,\"ExperiencePercent\":%d,\"DropPercent\":%d}\n",
                              GetCurrentProcessId(), rates.experiencePercent, rates.dropPercent);
    HANDLE file = CreateFileW(temporary, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file != INVALID_HANDLE_VALUE && size > 0) {
        DWORD written = 0;
        const bool saved = WriteFile(file, body, size, &written, NULL) && written == size && FlushFileBuffers(file);
        CloseHandle(file);
        if (saved) rates.reported = MoveFileExW(temporary, report, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != FALSE;
        else DeleteFileW(temporary);
    }
    return true;
}
