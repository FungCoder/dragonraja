#pragma once

#include <string>
#include <vector>

namespace DragonAutoPlayClient
{
enum ActionClassification
{
    AttackAction = 1,
    BuffAction = 2,
    CombatRecovery = 4,
    WorkAction = 8,
    DebuffAction = 16,
    RestorationAction = 32
};

struct ActionOption
{
    int kind;
    int skill;
    std::wstring label;
    unsigned classification;
};

// IDs are canonical server IDs, including the priest offset and combat 215+.
std::vector<ActionOption> GetActionOptions();
bool IsLearned(int kind, int skill);

// resourceKind: 0 = HP, 1 = mana/divine, 2 = hunger; quickSlot: 0..5.
// Slot 6 is reserved for crafting output and is never an auto-use source.
bool TryQuickRestore(int resourceKind, int quickSlot);
bool TrySwitchWornWeapon();

// Explicit map tile coordinates. Unsupported recipes/item targets return false.
bool IsWorkSupported(int skill);
bool TryWork(int skill, int tilex, int tiley, bool self);
}
