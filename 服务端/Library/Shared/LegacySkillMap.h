#pragma once
#include <cstdint>

// Original 32-bit SKB disk layout; saved pointer fields are never dereferenced.
#pragma pack(push, 4)
struct LegacySkillMapRecord
{
    std::int32_t type, x, y, skillno, tileRange, probability, typeNum, subType;
    std::uint32_t unusedPrev, unusedNext;
};
#pragma pack(pop)
static_assert(sizeof(LegacySkillMapRecord) == 40, "Legacy SKB format changed");

inline bool IsValidLegacySkillMapRecord(const LegacySkillMapRecord& record,
                                       int width, int height)
{
    return width > 0 && height > 0 && record.x >= 0 && record.x < width &&
           record.y >= 0 && record.y < height && record.skillno >= 0;
}

template<class RuntimeRecord>
void DecodeLegacySkillMapRecord(const LegacySkillMapRecord& disk,
                               RuntimeRecord& record)
{
    record.type = disk.type;
    record.x = disk.x;
    record.y = disk.y;
    record.skillno = disk.skillno;
    record.tile_Range = disk.tileRange;
    record.probability = disk.probability;
    record.type_Num = disk.typeNum;
    record.subType = disk.subType;
    record.prev = nullptr;
    record.next = nullptr;
}
