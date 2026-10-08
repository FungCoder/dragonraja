#pragma once
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

// 文件使用旧 Win32 结构布局。先完整验证，再允许调用方修改游戏状态。
template<class Object, class OldObject>
bool ReadMapObjectFile(FILE* file, unsigned capacity, unsigned short& imageCount,
                       std::vector<Object>& objects)
{
    if (!file) return false;
    std::uint16_t count = 0, images = 0;
    if (std::fread(&count, sizeof(count), 1, file) != 1) return false;
    const bool hasSound = count == 0xffff;
    if (hasSound && std::fread(&count, sizeof(count), 1, file) != 1) return false;
    if (count > capacity || std::fread(&images, sizeof(images), 1, file) != 1)
        return false;
    const long headerEnd = std::ftell(file);
    if (headerEnd < 0 || std::fseek(file, 0, SEEK_END) != 0) return false;
    const long length = std::ftell(file);
    const std::size_t indexBytes = static_cast<std::size_t>(count) * 4;
    const std::size_t recordBytes = hasSound ? sizeof(Object) : sizeof(OldObject);
    const std::size_t expected = static_cast<std::size_t>(headerEnd) + indexBytes +
        static_cast<std::size_t>(count) * recordBytes;
    if (length < 0 || static_cast<std::size_t>(length) < expected ||
        std::fseek(file, headerEnd + static_cast<long>(indexBytes), SEEK_SET) != 0)
        return false;
    std::vector<Object> loaded(count);
    for (unsigned i = 0; i < count; ++i)
    {
        if (hasSound)
        {
            if (std::fread(&loaded[i], sizeof(Object), 1, file) != 1) return false;
        }
        else
        {
            OldObject old = {};
            if (std::fread(&old, sizeof(old), 1, file) != 1) return false;
            std::memcpy(&loaded[i], &old, sizeof(old));
            loaded[i].soundno = 0;
            loaded[i].soundframe = 0;
            loaded[i].sounddelay = 1;
        }
    }
    imageCount = images;
    objects.swap(loaded);
    return true;
}

inline bool IsServerMapObject(unsigned type)
{
    return type != 0 && type != 5 && type != 6 &&
        !(type >= 71 && type <= 78) && !(type >= 81 && type <= 86);
}

inline bool IsMapObjectItemSlotValid(int slot, int capacity)
{
    return slot >= 0 && slot < capacity;
}

inline bool ReadMapDoorFileHeader(FILE* file, unsigned frames,
                                  std::int32_t& count, long& recordSize)
{
    if (!file || frames == 0 || frames > 10) return false;
    count = 0;
    if (std::fseek(file, 0, SEEK_SET) != 0 ||
        std::fread(&count, sizeof(count), 1, file) != 1 || count < 0 ||
        std::fseek(file, 0, SEEK_END) != 0) return false;
    const long length = std::ftell(file);
    recordSize = 80 + 8 * frames;
    if (length < 4 || count > (length - 4) / recordSize) return false;
    return true;
}

inline bool ReadMapDoorLine(FILE* file, int itemId, unsigned frames, short (&line)[4])
{
    std::int32_t count = 0;
    long recordSize = 0;
    if (!ReadMapDoorFileHeader(file, frames, count, recordSize)) return false;
    for (int index = 0; index < count; ++index)
    {
        const long start = 4 + index * recordSize;
        std::int32_t id = 0;
        if (std::fseek(file, start, SEEK_SET) != 0 ||
            std::fread(&id, sizeof(id), 1, file) != 1) return false;
        if (id != itemId) continue;
        short loaded[4] = {};
        for (int field = 0; field < 4; ++field)
            if (std::fseek(file, start + 80 + field * 2 * frames, SEEK_SET) != 0 ||
                std::fread(&loaded[field], sizeof(short), 1, file) != 1) return false;
        std::memcpy(line, loaded, sizeof(loaded));
        return true;
    }
    return false;
}
