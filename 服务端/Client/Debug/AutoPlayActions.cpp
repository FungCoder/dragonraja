#include "stdafx.h"
#include "AutoPlayActions.h"
#include "../../Library/Shared/AutoPlayProtocol.h"
#include "Dragon.h"
#include "Char.h"
#include "CharDataTable.h"
#include "Menu.h"
#include "MenuSet.h"
#include "ItemTable.h"
#include "SmallMenu.h"
#include "Skill.h"
#include "skill_lsw.h"
#include "Effect.h"

extern void EatItem(const ItemAttr item, const POS position);

namespace DragonAutoPlayClient
{
namespace
{
std::wstring LocalLabel(const char* text)
{
    if (!text || !*text) return std::wstring();
    const int length = MultiByteToWideChar(CP_ACP, 0, text, -1, nullptr, 0);
    if (length <= 1) return std::wstring();
    std::wstring label(static_cast<size_t>(length), L'\0');
    if (!MultiByteToWideChar(CP_ACP, 0, text, -1, &label[0], length))
        return std::wstring();
    label.resize(static_cast<size_t>(length - 1));
    return label;
}

int LocalMagicId(int canonical)
{
    const int spell = SCharacterData.nCharacterData[SPELL];
    if (spell != WIZARD_SPELL && spell != PRIEST_SPELL) return -1;
    const int local = canonical - (spell == PRIEST_SPELL ? 150 : 0);
    return canonical > 0 && canonical < 215 && local > 0 &&
        local < MAX_MAGIC_ ? local : -1;
}

template<class Food>
bool Restores(const Food* food, int resource)
{
    return resource == 0 ? food->hp > 0 :
        (resource == 1 ? food->mp > 0 : food->ap > 0);
}

bool RestoresResource(int itemNo, CItem* item, int resource)
{
    if (!item) return false;
    switch (itemNo / 1000)
    {
    case PLANT: return Restores(static_cast<CItem_Plant*>(item), resource);
    case HERB: return Restores(static_cast<CItem_Herb*>(item), resource);
    case COOK: return Restores(static_cast<CItem_Cook*>(item), resource);
    case POTION: return Restores(static_cast<CItem_Potion*>(item), resource);
    default: return false;
    }
}
}

bool IsLearned(int kind, int skill)
{
    if (!Hero) return false;
    switch (kind)
    {
    case DragonAutoPlay::Attack: return skill == 0;
    case DragonAutoPlay::Magic:
        {
            const int local = LocalMagicId(skill);
            return local > 0 && SCharacterData.MagicId[local] != 0;
        }
    case DragonAutoPlay::Combat:
        {
            const int index = skill - 214;
            return index > 0 && index <= 36 &&
                SCharacterData.SkillId[iCSCovertSkillID + index - 1] != 0;
        }
    case DragonAutoPlay::Work:
        return skill > 0 && skill < MAX_SKILL_ &&
            SCharacterData.SkillId[skill] != 0;
    default: return false;
    }
}

std::vector<ActionOption> GetActionOptions()
{
    std::vector<ActionOption> options;
    if (!Hero) return options;
    options.push_back({ DragonAutoPlay::Attack, 0, L"\u5e73\u780d", AttackAction });
    const int offset = SCharacterData.nCharacterData[SPELL] == PRIEST_SPELL ? 150 : 0;
    for (int local = 1; local < MAX_MAGIC_; ++local)
    {
        const int canonical = local + offset;
        if (!IsLearned(DragonAutoPlay::Magic, canonical)) continue;
        unsigned classification = 0;
        const int category = g_lpMT[canonical].magic_Type / 10;
        if (category == 1) classification = RestorationAction;
        if (category == 2 || category == 3) classification = BuffAction;
        if (category == 4) classification = DebuffAction;
        if (category == 5) classification = AttackAction;
        std::wstring label = LocalLabel(magic[local].GetHanName());
        if (label.empty()) label = LocalLabel(magic[local].GetName());
        if (label.empty()) label = L"Magic " + std::to_wstring(canonical);
        options.push_back({ DragonAutoPlay::Magic, canonical, label, classification });
    }
    for (int canonical = 215; canonical <= 250; ++canonical)
    {
        if (!IsLearned(DragonAutoPlay::Combat, canonical)) continue;
        unsigned classification = 0;
        const int type = g_lpMT[canonical].spell_Type;
        if (type == RECOVERY_COMBAT) classification = CombatRecovery;
        else if (type == PASSIVE_COMBAT) classification = BuffAction;
        else if (type == ACTIVE_COMBAT) classification = AttackAction;
        const int skillIndex = iCSCovertSkillID + canonical - 215;
        std::wstring label = skillIndex < MAX_SKILL_ ?
            LocalLabel(SkillTable[skillIndex].Skill_Minute) : std::wstring();
        if (label.empty()) label = L"Combat " + std::to_wstring(canonical);
        options.push_back({ DragonAutoPlay::Combat, canonical, label, classification });
    }
    for (int skill = 1; skill < MAX_SKILL_; ++skill)
    {
        if (!IsLearned(DragonAutoPlay::Work, skill) || !IsWorkSupported(skill)) continue;
        std::wstring label = LocalLabel(SkillTable[skill].Skill_Minute);
        if (label.empty()) label = L"Work " + std::to_wstring(skill);
        options.push_back({ DragonAutoPlay::Work, skill, label, WorkAction });
    }
    return options;
}

bool TryQuickRestore(int resourceKind, int quickSlot)
{
    if (!Hero || IsDead(Hero) || resourceKind < 0 || resourceKind > 2 ||
        quickSlot < 0 || quickSlot >= 6) return false;
    const ItemAttr selected = QuickItemAttr[quickSlot];
    if (selected.item_no <= 0) return false;
    CItem* container = ItemUnit(selected.item_no);
    if (!container) return false;
    int actualItem = selected.item_no;
    if (container->GetItemKind() == IK_POTION_BOX)
    {
        int count = 0;
        GetItemByPotionBox(actualItem, count);
        if (count <= 0) return false;
    }
    else if (container->GetItemKind() == IK_POTION_BAG)
    {
        WORD current = 0, maximum = 0;
        GetItemDuration(selected, current, maximum);
        if (current == 0) return false;
    }
    const int action = container->GetRButton();
    if (action != USE_ITEM && action != REMAIN_ITEM) return false;
    if (!RestoresResource(actualItem, ItemUnit(actualItem), resourceKind)) return false;
    POS position = {};
    SetItemPos(QUICK, quickSlot, &position);
    EatItem(selected, position);
    return true;
}

bool TrySwitchWornWeapon()
{
    if (!Hero || IsDead(Hero) || HandItemAttr.item_no != 0) return false;
    const ItemAttr worn = EquipItemAttr[WT_WEAPON];
    if (worn.item_no <= 0 || !ItemUnit(worn.item_no)) return false;
    WORD current = 0, maximum = 0;
    GetItemDuration(worn, current, maximum);
    if (maximum == 0 || current >= 5) return false;
    for (int page = 0; page < 3; ++page)
    for (int row = 0; row < 3; ++row)
    for (int column = 0; column < 8; ++column)
    {
        const ItemAttr replacement = InvItemAttr[page][row][column];
        if (replacement.item_no != worn.item_no) continue;
        GetItemDuration(replacement, current, maximum);
        if (maximum == 0 || current < 5) continue;
        POS source = {}, destination = {};
        SetItemPos(INV, page, row, column, &source);
        SetItemPos(EQUIP, WT_WEAPON, &destination);
        SendMoveItem(replacement.item_no, source, destination);
        return true;
    }
    return false;
}
}
