#include "../Library/Shared/LegacySkillMap.h"
#include <cassert>
#include <cstdio>

struct RuntimeSkillRecord
{
    int type, x, y, skillno, tile_Range, probability, type_Num, subType;
    RuntimeSkillRecord* prev;
    RuntimeSkillRecord* next;
};

int main(int argc, char** argv)
{
    LegacySkillMapRecord disk = {1, 179, 179, 6, 2, 1, 25, 0,
                                  0xffffffffu, 0xffffffffu};
    RuntimeSkillRecord runtime = {};
    DecodeLegacySkillMapRecord(disk, runtime);
    assert(runtime.type_Num == 25 && runtime.tile_Range == 2);
    assert(runtime.prev == nullptr && runtime.next == nullptr);
    assert(IsValidLegacySkillMapRecord(disk, 180, 180));
    disk.x = 180;
    assert(!IsValidLegacySkillMapRecord(disk, 180, 180));
    disk.x = -1;
    assert(!IsValidLegacySkillMapRecord(disk, 180, 180));
    assert(!IsValidLegacySkillMapRecord(disk, 0, 0));

    if (argc != 2) return 2;
    FILE* file = std::fopen(argv[1], "rb");
    if (!file) return 2;
    int records = 0, npcs = 0;
    for (;;)
    {
        const std::size_t count = std::fread(&disk, 1, sizeof(disk), file);
        if (count == 0 && std::feof(file)) break;
        if (count != sizeof(disk) || !IsValidLegacySkillMapRecord(disk, 180, 180))
        {
            std::fclose(file);
            return 1;
        }
        DecodeLegacySkillMapRecord(disk, runtime);
        assert(runtime.prev == nullptr && runtime.next == nullptr);
        ++records;
        if (runtime.skillno == 6) ++npcs;
    }
    std::fclose(file);
    std::printf("records=%d npc_positions=%d runtime_bytes=%zu\n",
                records, npcs, sizeof(runtime));
    return 0;
}
