#define NOMINMAX
#include <windows.h>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <map>
#include "../Client/Debug/AutoPlayActions.h"
#include "../Library/Shared/AutoPlayProtocol.h"

// These fixtures capture existing item/action calls. The runner compiles the
// actual adapter and explicit-work wrapper below, rather than a copied policy.
enum { PLANT, MINERAL, HERB, COOK, POTION, TOOL, WEAPON };
enum { HAND, INV, EQUIP, QUICK, BANK };
enum { SPELL, WIZARD_SPELL = 0, PRIEST_SPELL = 1, WT_WEAPON = 0,
    USE_ITEM = 1, REMAIN_ITEM = 4, IK_POTION_BOX = 100, IK_POTION_BAG = 147,
    ACTIVE_COMBAT = 3, PASSIVE_COMBAT = 4, RECOVERY_COMBAT = 5,
    MAX_MAGIC_ = 200, MAX_SKILL_ = 150, iCSCovertSkillID = 115,
    SKILL_OFF = 0, SKILL_ON = 2, SKILL_ING = 3,
    SKILLTARGETTYPE_MAP_ = 30, SKILLTARGETTYPE_HERO_ = 40,
    TILE_SIZE = 32, VILLAGE_SIZE = 300 };
struct POS { int type, p1, p2, p3; };
struct ItemAttr { int item_no; DWORD attr[6]; };
struct Character { int x, y, nCurrentAction; bool moveFlag, dead; };
typedef Character* LPCHARACTER;
static Character player = {};
static LPCHARACTER Hero = &player;
static bool IsDead(LPCHARACTER hero) { return hero->dead; }
struct CharacterData { int nCharacterData[2], MagicId[200], SkillId[200]; };
static CharacterData SCharacterData = {};
struct SkillData { char Skill_Minute[31]; int Howto; };
static SkillData SkillTable[150] = {};
struct MagicData { int spell_Type, magic_Type; };
static MagicData g_lpMT[256] = {};
struct MagicName
{
    const char* GetHanName() const { return "spell"; }
    const char* GetName() const { return "spell"; }
};
static MagicName magic[200];
class CItem
{
public:
    int kind = 0, action = USE_ITEM;
    int GetItemKind() const { return kind; }
    int GetRButton() const { return action; }
};
class CItem_Plant : public CItem { public: int hp = 0, mp = 0, ap = 0; };
class CItem_Herb : public CItem { public: int hp = 0, mp = 0, ap = 0; };
class CItem_Cook : public CItem { public: int hp = 0, mp = 0, ap = 0; };
class CItem_Potion : public CItem { public: int hp = 0, mp = 0, ap = 0; };
static std::map<int, CItem*> catalog;
static CItem* ItemUnit(int id)
{
    const auto found = catalog.find(id);
    return found == catalog.end() ? nullptr : found->second;
}
static ItemAttr QuickItemAttr[7] = {}, EquipItemAttr[8] = {};
static ItemAttr InvItemAttr[3][3][8] = {}, HandItemAttr = {};
static int boxItem, boxCount, eatRequests, moveRequests;
static POS movedSource = {}, movedDestination = {};
static void GetItemByPotionBox(int& item, int& count) { item = boxItem; count = boxCount; }
static void GetItemDuration(ItemAttr item, WORD& current, WORD& maximum)
{
    current = LOWORD(item.attr[0]); maximum = HIWORD(item.attr[0]);
}
static void SetItemPos(int type, int index, POS* pos) { *pos = { type, 0, 0, index }; }
static void SetItemPos(int type, int a, int b, int c, POS* pos) { *pos = { type, a, b, c }; }
static void EatItem(ItemAttr, POS) { ++eatRequests; }
static void SendMoveItem(int, POS source, POS destination)
{
    ++moveRequests; movedSource = source; movedDestination = destination;
}
static int SkillStatus = SKILL_OFF, SkillNo = 0, MapNumber = 0;
static int g_EventLocalWarBegin = 0;
struct MapData { struct { int wWidth, wHeight; } file; };
static MapData g_Map = { {100, 100} };
static bool hasTool = true, allowWork = true;
static int workRequests, sentSkill, sentX, sentY;
static bool AmIEquitSkillTool(int) { return hasTool; }
static int SelectSkill(int no) { SkillNo = no; SkillStatus = SKILL_ON; return 0; }
static void SkillSettingClear() { SkillStatus = SKILL_OFF; }
static int CheckSkillAt(int x, int y, LPCHARACTER, void*, bool)
{
    if (allowWork)
    {
        ++workRequests; sentSkill = SkillNo; sentX = x; sentY = y;
        SkillStatus = SKILL_ING;
    }
    return allowWork;
}
#include "AutoPlayActionsProduction.inc"

int main()
{
    using namespace DragonAutoPlayClient;
    CItem_Potion hp, mp, bag;
    hp.hp = 50; mp.mp = 50; bag.hp = 50; bag.kind = IK_POTION_BAG;
    CItem box, weapon;
    box.kind = IK_POTION_BOX;
    catalog = { {4001, &hp}, {4002, &mp}, {4003, &bag}, {10001, &box}, {6001, &weapon} };
    QuickItemAttr[0].item_no = 4001;
    assert(TryQuickRestore(0, 0));
    assert(!TryQuickRestore(1, 0) && !TryQuickRestore(2, 0));
    assert(!TryQuickRestore(-1, 0) && !TryQuickRestore(0, 7));
    QuickItemAttr[6].item_no = 4001;
    assert(!TryQuickRestore(0, 6));
    QuickItemAttr[0].item_no = 4002;
    assert(!TryQuickRestore(0, 0) && TryQuickRestore(1, 0));
    QuickItemAttr[0].item_no = 10001;
    boxItem = 4001; boxCount = 0;
    assert(!TryQuickRestore(0, 0));
    boxCount = 2;
    assert(TryQuickRestore(0, 0) && !TryQuickRestore(1, 0));
    QuickItemAttr[0].item_no = 4003;
    assert(!TryQuickRestore(0, 0));
    QuickItemAttr[0].attr[0] = MAKELONG(1, 10);
    assert(TryQuickRestore(0, 0));
    bag.hp = -1;
    assert(!TryQuickRestore(0, 0));
    assert(eatRequests == 4);

    EquipItemAttr[0].item_no = 6001;
    EquipItemAttr[0].attr[0] = MAKELONG(5, 100);
    InvItemAttr[2][2][7].item_no = 6001;
    InvItemAttr[2][2][7].attr[0] = MAKELONG(100, 100);
    assert(!TrySwitchWornWeapon());
    EquipItemAttr[0].attr[0] = MAKELONG(4, 100);
    const ItemAttr equipped = EquipItemAttr[0], spare = InvItemAttr[2][2][7];
    assert(TrySwitchWornWeapon() && moveRequests == 1);
    assert(movedSource.type == INV && movedSource.p1 == 2 && movedSource.p2 == 2 && movedSource.p3 == 7);
    assert(movedDestination.type == EQUIP && movedDestination.p3 == WT_WEAPON);
    assert(!std::memcmp(&equipped, &EquipItemAttr[0], sizeof(ItemAttr)));
    assert(!std::memcmp(&spare, &InvItemAttr[2][2][7], sizeof(ItemAttr)));
    HandItemAttr.item_no = 4001;
    assert(!TrySwitchWornWeapon());
    HandItemAttr.item_no = 0;

    SCharacterData.nCharacterData[SPELL] = PRIEST_SPELL;
    SCharacterData.MagicId[1] = 1;
    assert(IsLearned(DragonAutoPlay::Magic, 151));
    assert(!IsLearned(DragonAutoPlay::Magic, 1));
    SCharacterData.SkillId[115] = SCharacterData.SkillId[150] = 1;
    g_lpMT[215].spell_Type = RECOVERY_COMBAT;
    g_lpMT[151].magic_Type = 40;
    bool cpFound = false, curseFound = false, finalCombatFound = false;
    for (const ActionOption& option : GetActionOptions())
    {
        if (option.kind == DragonAutoPlay::Combat && option.skill == 215)
            cpFound = (option.classification & CombatRecovery) != 0;
        if (option.kind == DragonAutoPlay::Combat && option.skill == 250)
            finalCombatFound = true;
        if (option.kind == DragonAutoPlay::Magic && option.skill == 151)
            curseFound = (option.classification & DebuffAction) != 0 && !(option.classification & BuffAction);
    }
    assert(cpFound && curseFound && finalCombatFound);
    const unsigned categories[] = { 0, RestorationAction, BuffAction,
        BuffAction, DebuffAction, AttackAction };
    for (int category = 1; category <= 5; ++category)
    {
        g_lpMT[151].magic_Type = category * 10;
        bool found = false;
        for (const ActionOption& option : GetActionOptions())
            if (option.kind == DragonAutoPlay::Magic && option.skill == 151)
            {
                assert(option.classification == categories[category]);
                found = true;
            }
        assert(found);
    }

    SkillTable[32].Howto = SKILLTARGETTYPE_MAP_;
    SCharacterData.SkillId[32] = 1;
    player.x = player.y = 100;
    assert(!TryWork(32, -1, 3, false) && !TryWork(32, 100, 3, false));
    assert(!TryWork(44, 3, 3, false));
    hasTool = false;
    assert(!TryWork(32, 3, 3, false));
    hasTool = true;
    SkillNo = 26; SkillStatus = SKILL_ON;
    assert(!TryWork(32, 3, 3, false) && SkillNo == 26);
    SkillStatus = SKILL_OFF;
    assert(TryWork(32, 3, 3, false));
    assert(sentSkill == 32 && sentX == 112 && sentY == 112 && workRequests == 1);
    assert(!TryWork(32, 3, 3, false));
    SkillStatus = SKILL_OFF; SkillNo = 0; allowWork = false;
    assert(!TryWork(32, 3, 3, false) && SkillNo == 0 && SkillStatus == SKILL_OFF);
    std::puts("Production adapters: restoration types, weapon state, canonical skills and work checks passed");
}
