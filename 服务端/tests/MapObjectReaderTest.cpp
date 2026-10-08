#include <windows.h>
#pragma pack(push, 1)
#include "../Mapserver/HigherLayers/Object.h"
struct PackingSentinel { char first; int second; };
static_assert(sizeof(PackingSentinel) == 5, "Object.h must restore incoming packing");
#pragma pack(pop)
#include "../Mapserver/HigherLayers/MapObjectReader.h"
#include <cassert>
#include <cwchar>
#include <string>

static_assert(sizeof(MAPOBJECT_old) == 124, "Old object ABI");
static_assert(sizeof(MAPOBJECT) == 132, "Object ABI");

template<class Value>
void Append(std::vector<unsigned char>& bytes, const Value& value)
{
    const auto* start = reinterpret_cast<const unsigned char*>(&value);
    bytes.insert(bytes.end(), start, start + sizeof(value));
}

FILE* Fixture(const std::vector<unsigned char>& bytes, std::size_t length)
{
    FILE* file = std::tmpfile();
    assert(file);
    assert(length == 0 || std::fwrite(bytes.data(), 1, length, file) == length);
    std::rewind(file);
    return file;
}

bool Read(FILE* file, unsigned short& images, std::vector<MAPOBJECT>& objects)
{
    return ReadMapObjectFile<MAPOBJECT, MAPOBJECT_old>(file, MAX_MAPOBJECT_, images, objects);
}

void CheckObjects(bool withSound)
{
    std::vector<unsigned char> bytes;
    if (withSound) Append(bytes, static_cast<unsigned short>(0xffff));
    Append(bytes, static_cast<unsigned short>(1));
    Append(bytes, static_cast<unsigned short>(9));
    Append(bytes, 0);
    MAPOBJECT object = {};
    object.id = 317; object.x = 64; object.y = 128;
    object.objectoritem = 3; object.soundno = 15;
    if (withSound) Append(bytes, object);
    else
    {
        MAPOBJECT_old old = {};
        std::memcpy(&old, &object, sizeof(old));
        Append(bytes, old);
    }
    for (std::size_t length = 0; length < bytes.size(); ++length)
    {
        FILE* file = Fixture(bytes, length);
        unsigned short images = 77;
        std::vector<MAPOBJECT> loaded(1); loaded[0].id = 999;
        assert(!Read(file, images, loaded));
        assert(images == 77 && loaded.size() == 1 && loaded[0].id == 999);
        std::fclose(file);
    }
    FILE* file = Fixture(bytes, bytes.size());
    unsigned short images = 0;
    std::vector<MAPOBJECT> loaded;
    assert(Read(file, images, loaded));
    assert(images == 9 && loaded.size() == 1 && loaded[0].id == 317);
    assert(loaded[0].x == 64 && loaded[0].y == 128);
    assert(loaded[0].soundno == (withSound ? 15 : 0));
    assert(withSound || loaded[0].sounddelay == 1);
    std::fclose(file);
}

void CheckDoorLine()
{
    std::vector<unsigned char> bytes;
    Append(bytes, 1);
    for (int i = 0; i < 20; ++i) Append(bytes, i == 0 ? 13317 : 0);
    for (int field = 0; field < 4; ++field)
        for (int frame = 0; frame < ITEM_FRAME_MAX_; ++frame)
            Append(bytes, static_cast<short>(field * 10 - 20));
    for (std::size_t length = 0; length < bytes.size(); ++length)
    {
        FILE* file = Fixture(bytes, length);
        short line[4] = {1, 2, 3, 4};
        assert(!ReadMapDoorLine(file, 13317, ITEM_FRAME_MAX_, line));
        assert(line[0] == 1 && line[3] == 4);
        std::fclose(file);
    }
    FILE* file = Fixture(bytes, bytes.size());
    short line[4] = {};
    assert(ReadMapDoorLine(file, 13317, ITEM_FRAME_MAX_, line));
    assert(line[0] == -20 && line[1] == -10 && line[2] == 0 && line[3] == 10);
    assert(!ReadMapDoorLine(file, 13000, ITEM_FRAME_MAX_, line));
    assert(!ReadMapDoorLine(file, 13317, 0, line));
    std::fclose(file);
}

int wmain(int argc, wchar_t** argv)
{
    if (argc > 1 && std::wcscmp(argv[1], L"--audit") == 0)
    {
        std::printf("File\tValid\tObjects\tServerItems\tDoors\tDoorFileValid\tMissingDoorLines\n");
        for (int index = 2; index < argc; ++index)
        {
            FILE* file = _wfopen(argv[index], L"rb");
            unsigned short images = 0;
            std::vector<MAPOBJECT> objects;
            const bool valid = Read(file, images, objects);
            if (file) std::fclose(file);
            int items = 0, doors = 0;
            std::wstring doorPath(argv[index]);
            if (doorPath.size() < 5 ||
                _wcsicmp(doorPath.c_str() + doorPath.size() - 5, L".toi2") != 0)
            {
                std::fprintf(stderr, "Audit requires .toi2 input paths\n");
                return 1;
            }
            doorPath.resize(doorPath.size() - 5);
            doorPath += L"_toi2.b";
            FILE* doorFile = _wfopen(doorPath.c_str(), L"rb");
            std::int32_t doorCount = 0;
            long doorRecordSize = 0;
            const bool doorValid = ReadMapDoorFileHeader(doorFile, ITEM_FRAME_MAX_, doorCount, doorRecordSize);
            int missingLines = 0;
            for (const MAPOBJECT& object : objects)
            {
                if (IsServerMapObject(object.objectoritem)) ++items;
                if (object.objectoritem == 3)
                {
                    ++doors;
                    short line[4] = {};
                    if (!ReadMapDoorLine(doorFile, object.id + 13000, ITEM_FRAME_MAX_, line))
                        ++missingLines;
                }
            }
            if (doorFile) std::fclose(doorFile);
            const wchar_t* name = std::wcsrchr(argv[index], L'\\');
            std::printf("%ls\t%d\t%zu\t%d\t%d\t%d\t%d\n", name ? name + 1 : argv[index],
                        valid ? 1 : 0, objects.size(), items, doors, doorValid ? 1 : 0, missingLines);
        }
        return 0;
    }
    CheckObjects(false);
    CheckObjects(true);
    CheckDoorLine();
    unsigned short images = 0;
    std::vector<MAPOBJECT> loaded;
    assert(!Read(nullptr, images, loaded));
    std::vector<unsigned char> bytes;
    Append(bytes, static_cast<unsigned short>(MAX_MAPOBJECT_ + 1));
    Append(bytes, static_cast<unsigned short>(0));
    FILE* file = Fixture(bytes, bytes.size());
    assert(!Read(file, images, loaded));
    std::fclose(file);
    bytes.clear();
    Append(bytes, static_cast<unsigned short>(0));
    Append(bytes, static_cast<unsigned short>(0));
    file = Fixture(bytes, bytes.size());
    assert(Read(file, images, loaded) && loaded.empty());
    std::fclose(file);
    assert(IsMapObjectItemSlotValid(0, 10000));
    assert(IsMapObjectItemSlotValid(9999, 10000));
    assert(!IsMapObjectItemSlotValid(-1, 10000));
    assert(!IsMapObjectItemSlotValid(10000, 10000));
    for (unsigned type = 0; type <= 100; ++type)
    {
        const bool skipped = type == 0 || type == 5 || type == 6 ||
            (type >= 71 && type <= 78) || (type >= 81 && type <= 86);
        assert(IsServerMapObject(type) == !skipped);
    }
    std::printf("Map objects: ABI, old/new format, all truncation boundaries, count, door lines and item slots passed\n");
    return 0;
}
