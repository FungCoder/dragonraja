#include "../Mapserver/HigherLayers/GameplaySafety.h"
#include <cassert>
#include <cstdio>
#include <climits>

// The runner extracts these two production functions without modifying them.
// Lightweight collaborators let failures exercise actual reward control flow.
enum { HAND, INV, EQUIP, QUICK, BANK, BOX, MAKE, OTHER_CH, AUCTION_DB };
enum { DB_DEMON = 1, DRAGON_MAX_CONNECTIONS = 1000,
    SEALSTONE_YLLSE_NO = 1, SEALSTONE_VYSEUS_NO, SEALSTONE_ZYPERN_NO,
    N_YILSE = 3, N_VYSEUS = 4, N_ZYPERN = 6,
    DUAL_FAME_FIELD = 10, LDF_QUEST = 1, LOG_NORMAL = 0 };
struct POS { char type; short p1, p2, p3; };
struct ItemAttr { int item_no; };
struct CHARLIST
{
    ItemAttr inv[3][3][8], equip[8], quick[7], bank[5][3][6], handheld;
    bool JoinLocalWar;
    struct { int nation; } name_status;
    char Name[16];
    int GetServerID() const { return 2; }
};
struct table_requital_list
{
    int WarStatus, Npc_index, Sardonyx, LeafOfBlessed, ref_index;
    int Item_no, Item_Min, Item_Max, Item_rare_count, Item_rare_type;
    int Quest_no, Quest_step, Real_Fame, Dual_Fame;
};
class CQuestInMap
{
public:
    void GiveRequitalByCn(int cn, table_requital_list& table);
};
static CHARLIST player = {};
static int var[DRAGON_MAX_CONNECTIONS][20] = {};
static bool connected = true;
static int grantResult, grantCalls, grantedItems, fame, fameSaves;
static int successMessages, failureMessages;
static CHARLIST* CheckServerId(int cn)
{
    return connected && cn == 2 ? &player : nullptr;
}
static int DecideRequitalByNationDefensePoint(CHARLIST*) { return 1; }
static bool Requital_About_SealStone(int, int, int, int) { return true; }
static void OutMessage(CHARLIST*, int category, int)
{
    if (category == 4) ++successMessages;
    else ++failureMessages;
}
static void MyLog(int, const char*, ...) {}
static int SetRareItemInvPc(int, int, int, int, int, int, int)
{
    ++grantCalls;
    if (grantResult == 1) ++grantedItems;
    return grantResult;
}
static void FameUp2(int, int, int amount) { fame += amount; }
static void SaveChangeDualFame(CHARLIST*, int, int, int) { ++fameSaves; }
static ItemAttr* GetItemByPOS(CHARLIST*, POS);
#include "FeaturePortSafetyProduction.inc"

static void TestPositions()
{
    // Every neighbouring face, corner and valid slot of both storage arrays.
    const int capacities[2][3] = { {3, 3, 8}, {5, 3, 6} };
    for (int storage = 0; storage < 2; ++storage)
    {
        for (int page = -1; page <= capacities[storage][0]; ++page)
        for (int row = -1; row <= capacities[storage][1]; ++row)
        for (int column = -1; column <= capacities[storage][2]; ++column)
        {
            POS position = { static_cast<char>(storage == 0 ? INV : BANK),
                static_cast<short>(page), static_cast<short>(row),
                static_cast<short>(column) };
            const bool expected = page >= 0 && page < capacities[storage][0] &&
                row >= 0 && row < capacities[storage][1] &&
                column >= 0 && column < capacities[storage][2];
            ItemAttr* item = GetItemByPOS(&player, position);
            assert((item != nullptr) == expected);
            if (item)
                assert(item == (storage == 0 ? &player.inv[page][row][column] :
                    &player.bank[page][row][column]));
        }
    }
    for (int type = EQUIP; type <= QUICK; ++type)
    for (int slot = -1; slot <= 8; ++slot)
    {
        // p1/p2 are unused for EQUIP/QUICK; preserve that contract.
        POS position = { static_cast<char>(type), SHRT_MIN, SHRT_MAX,
            static_cast<short>(slot) };
        ItemAttr* item = GetItemByPOS(&player, position);
        const int capacity = type == EQUIP ? 8 : 7;
        assert((item != nullptr) == (slot >= 0 && slot < capacity));
    }
    POS hand = { HAND, SHRT_MIN, SHRT_MAX, SHRT_MAX };
    assert(GetItemByPOS(&player, hand) == &player.handheld);
    assert(!GetItemByPOS(static_cast<CHARLIST*>(nullptr), hand));
    assert(GetItemByPOS(2, hand) == &player.handheld);
    assert(!GetItemByPOS(65538, hand));
    assert(!GetItemByPOS(-1, hand));
    connected = false;
    assert(!GetItemByPOS(2, hand));
    connected = true;
    POS unsupported = { OTHER_CH, 0, 0, 0 };
    assert(!GetItemByPOS(&player, unsupported));
    unsupported.type = static_cast<char>(255);
    assert(!GetItemByPOS(&player, unsupported));
}

static void ResetRewards()
{
    grantCalls = grantedItems = fame = fameSaves = 0;
    successMessages = failureMessages = 0;
    var[2][5] = 3;
    var[2][DUAL_FAME_FIELD] = 11;
}

int main()
{
    TestPositions();
    CQuestInMap quest;
    table_requital_list reward = {};
    reward.Item_no = 1001;
    reward.Quest_no = 5;
    reward.Quest_step = 4;
    reward.Real_Fame = 7;
    reward.Dual_Fame = 2;
    const int failures[] = {0, -1, 2};
    for (int failure : failures)
    {
        ResetRewards();
        grantResult = failure;
        quest.GiveRequitalByCn(2, reward);
        assert(grantCalls == 1 && grantedItems == 0);
        assert(var[2][5] == 3 && var[2][DUAL_FAME_FIELD] == 11);
        assert(fame == 0 && fameSaves == 0);
        assert(successMessages == 0 && failureMessages == 1);
    }
    // A full inventory can be cleared and the same quest retried successfully.
    ResetRewards();
    grantResult = 0;
    quest.GiveRequitalByCn(2, reward);
    grantResult = 1;
    quest.GiveRequitalByCn(2, reward);
    assert(grantCalls == 2 && grantedItems == 1);
    assert(var[2][5] == 4 && var[2][DUAL_FAME_FIELD] == 13);
    assert(fame == 7 && fameSaves == 1);
    ResetRewards();
    reward.Item_no = 0;
    quest.GiveRequitalByCn(2, reward);
    assert(grantCalls == 0 && var[2][5] == 4 && fame == 7 && fameSaves == 1);
    ResetRewards();
    connected = false;
    quest.GiveRequitalByCn(2, reward);
    assert(grantCalls == 0 && var[2][5] == 3 && fameSaves == 0);
    std::puts("Production POS boundaries and quest reward failure/retry checks passed");
}
