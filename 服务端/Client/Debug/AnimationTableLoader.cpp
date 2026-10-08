#include "stdafx.h"
#include "AnimationTableLoader.h"
#include "directsound.h"

static_assert(sizeof(SOUNDLIST) == 20, "SOUNDLIST layout must match SOUNDLIST.BIN");

#include <cstdio>
#include <cstring>

static AnimationTableLoadResult ReportAnimationTableError(
    const char* file_name, AnimationTableLoadResult result)
{
    g_DBGLog.Log(LOG_LV1, "Animation table load failed: %s (reason=%d)",
                 file_name, result);
    return result;
}

AnimationTableLoadResult LoadAnimationTableBinary(
    const char* file_name, PCANIMATIONTABLE* destination, int capacity)
{
    if (file_name == NULL || destination == NULL || capacity <= 0)
    {
        return ANIMATION_TABLE_INVALID;
    }

    std::memset(destination, 0, sizeof(PCANIMATIONTABLE) * capacity);

    FILE* file = NULL;
    if (fopen_s(&file, file_name, "rb") != 0 || file == NULL)
    {
        return ReportAnimationTableError(file_name, ANIMATION_TABLE_MISSING);
    }

    const long header_size = 21;
    if (std::fseek(file, 0, SEEK_END) != 0)
    {
        std::fclose(file);
        return ReportAnimationTableError(file_name, ANIMATION_TABLE_INVALID);
    }

    const long file_size = std::ftell(file);
    const long payload_size = file_size - header_size;
    if (payload_size <= 0 ||
        payload_size % sizeof(PCANIMATIONTABLE) != 0 ||
        payload_size / sizeof(PCANIMATIONTABLE) > capacity ||
        std::fseek(file, 20, SEEK_SET) != 0)
    {
        std::fclose(file);
        return ReportAnimationTableError(file_name, ANIMATION_TABLE_INVALID);
    }

    unsigned char stored_checksum = 0;
    const size_t record_count = payload_size / sizeof(PCANIMATIONTABLE);
    if (std::fread(&stored_checksum, 1, 1, file) != 1 ||
        std::fread(destination, sizeof(PCANIMATIONTABLE), record_count, file)
            != record_count)
    {
        std::fclose(file);
        std::memset(destination, 0, sizeof(PCANIMATIONTABLE) * capacity);
        return ReportAnimationTableError(file_name, ANIMATION_TABLE_INVALID);
    }
    std::fclose(file);

    unsigned char calculated_checksum = 0;
    const unsigned char* bytes =
        reinterpret_cast<const unsigned char*>(destination);
    for (size_t index = 0;
         index < record_count * sizeof(PCANIMATIONTABLE); ++index)
    {
        calculated_checksum += bytes[index];
    }
    if (calculated_checksum != stored_checksum)
    {
        std::memset(destination, 0, sizeof(PCANIMATIONTABLE) * capacity);
        return ReportAnimationTableError(file_name, ANIMATION_TABLE_INVALID);
    }
    return ANIMATION_TABLE_OK;
}
