#pragma once
#include <windows.h>
#include <cstring>

// Local mailbox wire format: magic, request UUID, UTC FILETIME, byte length, GBK.
// Only the account able to write this executable directory can submit requests.
typedef void (*ManagerNoticeBroadcast)(char*, int, DWORD*, DWORD*);

inline void ProcessManagerNotice(const BYTE* data, DWORD size, ULONGLONG now,
                                ManagerNoticeBroadcast broadcast, BYTE reply[32])
{
    memset(reply, 0, 32);
    memcpy(reply, "DRA1", 4);
    DWORD status = 1, targets = 0, failed = 0;
    if (size >= 32) {
        memcpy(reply + 4, data + 4, 16);
        ULONGLONG expires = 0;
        DWORD length = 0;
        memcpy(&expires, data + 20, 8);
        memcpy(&length, data + 28, 4);
        if (memcmp(data, "DRN1", 4) == 0 && length > 0 && length <= 258 && size == 32 + length) {
            status = 2;
            if (expires >= now && expires - now <= 300000000ULL) {
                char text[MAX_PATH] = {};
                memcpy(text, data + 32, length);
                bool valid = true;
                for (DWORD i = 0; i < length; ++i) {
                    const BYTE value = data[32 + i];
                    if (value == 0 || value == '%' || value == 127 || (value < 32 && value != 9 && value != 10 && value != 13)) valid = false;
                }
                WCHAR unicode[MAX_PATH] = {};
                if (!MultiByteToWideChar(936, MB_ERR_INVALID_CHARS, text, length, unicode, MAX_PATH)) valid = false;
                status = 1;
                if (valid) {
                    broadcast(text, length, &targets, &failed);
                    status = targets == 0 ? 3 : (failed ? 4 : 0);
                }
            }
        }
    }
    memcpy(reply + 20, &status, 4);
    memcpy(reply + 24, &targets, 4);
    memcpy(reply + 28, &failed, 4);
}

inline void PollManagerNotice(ManagerNoticeBroadcast broadcast)
{
    WCHAR base[MAX_PATH] = {};
    const DWORD count = GetModuleFileNameW(NULL, base, MAX_PATH);
    if (!count || count >= MAX_PATH) return;
    WCHAR* separator = wcsrchr(base, L'\\');
    if (!separator) return;
    separator[1] = 0;
    WCHAR request[MAX_PATH] = {}, response[MAX_PATH] = {}, temporary[MAX_PATH] = {};
    if (swprintf_s(request, L"%smanager_notice.req", base) < 0 ||
        swprintf_s(response, L"%smanager_notice.ack", base) < 0 ||
        swprintf_s(temporary, L"%smanager_notice.ack.tmp", base) < 0) return;
    HANDLE file = CreateFileW(request, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_DELETE,
                              NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) return;
    BYTE data[32 + MAX_PATH] = {}, reply[32] = {};
    DWORD read = 0;
    const DWORD size = GetFileSize(file, NULL);
    const bool readable = size <= sizeof(data) && ReadFile(file, data, size, &read, NULL) && read == size;
    CloseHandle(file);
    // Claim the request before sending. A write/delete failure cannot duplicate it.
    if (!DeleteFileW(request)) return;
    if (!readable) return;
    FILETIME clock;
    GetSystemTimeAsFileTime(&clock);
    ULONGLONG now = 0;
    memcpy(&now, &clock, 8);
    ProcessManagerNotice(data, read, now, broadcast, reply);
    file = CreateFileW(temporary, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) return;
    DWORD written = 0;
    const bool saved = WriteFile(file, reply, sizeof(reply), &written, NULL) && written == sizeof(reply) && FlushFileBuffers(file);
    CloseHandle(file);
    if (saved) MoveFileExW(temporary, response, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
    else DeleteFileW(temporary);
}
