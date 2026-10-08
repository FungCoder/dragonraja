#pragma once
#include <windows.h>
#include <string>

// The Chinese game wire format is CP936, independent of the Windows ACP.
inline bool EncodeNoticeGbk(const WCHAR* text, int length, std::string& encoded)
{
    encoded.clear();
    if (!text || length <= 0 || length > 4096) return false;
    BOOL substituted = FALSE;
    const int bytes = WideCharToMultiByte(936, WC_NO_BEST_FIT_CHARS, text, length,
                                          NULL, 0, NULL, &substituted);
    if (bytes <= 0 || bytes > 4096 || substituted) return false;
    encoded.resize(bytes);
    substituted = FALSE;
    if (!WideCharToMultiByte(936, WC_NO_BEST_FIT_CHARS, text, length,
                            &encoded[0], bytes, NULL, &substituted) || substituted) {
        encoded.clear();
        return false;
    }
    return true;
}
