#include "../stdafx.h"
#pragma pack(push, 4)
#include "../ItemTable.h"
#pragma pack(pop)

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::fprintf(stderr, "Usage: ItemTableProbe <itemtable0.bin>\n");
        return 2;
    }

    FILE* file = nullptr;
    if (fopen_s(&file, argv[1], "rb") != 0 || file == nullptr)
    {
        std::fprintf(stderr, "Cannot open item table.\n");
        return 2;
    }

    std::fseek(file, 0, SEEK_END);
    const long file_size = std::ftell(file);
    std::rewind(file);

    std::int32_t plain_count = 0;
    HSEL_INITIAL settings = {};
    std::int32_t encrypted_count = 0;
    std::int32_t stored_count_crc = 0;
    if (std::fread(&plain_count, sizeof(plain_count), 1, file) != 1 ||
        std::fread(&settings, sizeof(settings), 1, file) != 1 ||
        std::fread(&encrypted_count, sizeof(encrypted_count), 1, file) != 1 ||
        std::fread(&stored_count_crc, sizeof(stored_count_crc), 1, file) != 1)
    {
        std::fprintf(stderr, "Item table header is incomplete.\n");
        std::fclose(file);
        return 2;
    }

    CHSEL_STREAM cipher;
    const int initial_result = cipher.Initial(settings);
    const bool count_result = cipher.Decrypt(
        reinterpret_cast<char*>(&encrypted_count), sizeof(encrypted_count));
    std::printf("size=%ld settings_size=%zu plain_count=%d initial=%d "
                "decrypt_count=%d decrypted_count=%d count_crc=%d stored_crc=%d\n",
                file_size, sizeof(settings), plain_count, initial_result,
                count_result, encrypted_count, cipher.GetCRCConvertInt(),
                stored_count_crc);

    const long payload_size = file_size - static_cast<long>(sizeof(plain_count) +
        sizeof(settings) + sizeof(encrypted_count) + sizeof(stored_count_crc));
    if (plain_count <= 0 || payload_size <= 0 ||
        payload_size % plain_count != 0 || payload_size / plain_count <= 4)
    {
        std::fprintf(stderr, "Invalid record layout.\n");
        std::fclose(file);
        return 2;
    }
    const std::size_t record_size = payload_size / plain_count - 4;
    std::vector<char> record(record_size);
    int bad_records = 0;
    for (int record_index = 0; record_index < plain_count; ++record_index)
    {
        std::int32_t stored_record_crc = 0;
        if (std::fread(record.data(), record.size(), 1, file) != 1 ||
            std::fread(&stored_record_crc, sizeof(stored_record_crc), 1, file) != 1)
        {
            std::fprintf(stderr, "Record %d is incomplete.\n", record_index);
            std::fclose(file);
            return 2;
        }
        const bool decrypted = cipher.Decrypt(record.data(), record.size());
        if (!decrypted || cipher.GetCRCConvertInt() != stored_record_crc)
        {
            ++bad_records;
            if (bad_records == 1)
            {
                std::printf("first_bad_record=%d calculated_crc=%d stored_crc=%d\n",
                    record_index, cipher.GetCRCConvertInt(), stored_record_crc);
            }
        }
        if (decrypted && record_size == sizeof(CItem_Armor))
        {
            const CItem_Armor* armor = reinterpret_cast<const CItem_Armor*>(record.data());
            if (armor->Item_id == 38 || armor->Item_id == 103 || armor->Item_id == 120)
                std::printf("armor=%d wear=%d gender=%d str=%d con=%d dex=%d wis=%d int=%d ws=%d ps=%d level=%d classes=%d,%d,%d,%d,%d\n",
                    armor->Item_id, armor->wear_able, armor->Need3_gender,
                    armor->Need3_str, armor->Need3_con, armor->Need3_dex,
                    armor->Need3_wis, armor->Need3_int, armor->Need3_ws,
                    armor->Need3_ps, armor->Need3_lv, armor->Class_Warrior,
                    armor->Class_Thief, armor->Class_Archer,
                    armor->Class_Wizard, armor->Class_Cleric);
            if (armor->Item_id == 38 || armor->Item_id == 103 || armor->Item_id == 120)
                std::printf("armor=%d nationmask=%d lv=%d fame=%d dual=%d dates=%d,%d kind=%d\n",
                    armor->Item_id, armor->Imunity_Cure_4, armor->Imunity_Cure_5,
                    armor->Imunity_Cure_6, armor->Need2_max_age,
                    armor->Repair_Skill2_min, armor->Repair_Res1, armor->Item_kind);
        }
    }
    std::fclose(file);
    std::printf("record_size=%zu verified_records=%d bad_records=%d\n",
                record_size, plain_count, bad_records);
    std::printf("compiled_CItem_Plant_size=%zu\n", sizeof(CItem_Plant));
    std::printf("record_sizes=%zu,%zu,%zu,%zu,%zu,%zu,%zu,%zu,%zu,%zu,%zu\n",
                sizeof(CItem_Plant), sizeof(CItem_Mineral), sizeof(CItem_Herb),
                sizeof(CItem_Cook), sizeof(CItem_Potion), sizeof(CItem_Tool),
                sizeof(CItem_Weapon), sizeof(CItem_Disposable),
                sizeof(CItem_Armor), sizeof(CItem_Accessory), sizeof(CItem_Etc));
    return 0;
}
