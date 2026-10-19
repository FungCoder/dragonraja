#pragma once
#include "../../Library/Shared/CombatShotProtocol.h"

// Include after the existing packed Network5.h declarations. The forwarding
// boundary calls this policy; internal AI calls do not pass through it.
namespace DragonClientBattle
{
static_assert(sizeof(t_player_attack) == 3, "Unexpected player attack wire layout");
static_assert(sizeof(t_client_magic_result) == 8, "Unexpected magic result wire layout");
static_assert(sizeof(t_magic_result_d) == 9, "Unexpected NPC result wire layout");
static_assert(sizeof(t_client_throw_result) == 7, "Unexpected throw result wire layout");
static_assert(sizeof(t_client_effect_result) == 9, "Unexpected item effect wire layout");
enum PayloadRule { Unrelated = -1, ServerOnly = -2 };

inline int ExpectedPayloadSize(unsigned command)
{
    switch (command)
    {
    case DragonCombatShot::HelloCommand: return sizeof(DragonCombatShot::Hello);
    case DragonCombatShot::HitCommand: return sizeof(DragonCombatShot::Hit);
    case DragonCombatShot::AckCommand:
    case DragonCombatShot::IssuedCommand:
    case DragonCombatShot::ArrowIssuedCommand:
    case DragonCombatShot::MagicIssuedCommand:
    case DragonCombatShot::BurstIssuedCommand: return ServerOnly;
    case CMD_ATTACK: return sizeof(t_player_attack);
    case CMD_THROW_ATTACK: return sizeof(t_client_throw_attack);
    case CMD_THROW_RESULT: return ServerOnly;
    // 7615 is also named CMD_COMBAT_RESULT_C. Preserve its client effect use.
    case CMD_EFFECT_RESULT: return sizeof(t_client_effect_result);
    case CMD_MAGIC_SELECT: return sizeof(t_client_magic_select);
    case CMD_MAGIC_CASTING: return sizeof(t_client_magic_casting);
    case CMD_MAGIC_EXECUTE: return sizeof(t_client_magic_execute);
    case CMD_MAGIC_RESULT: return sizeof(t_client_magic_result);
    case CMD_MAGIC_RESULT_D: return sizeof(t_magic_result_d);
    case CMD_COMBAT_CLEAR: return sizeof(t_client_combat_clear);
    case CMD_COMBAT_OBTAIN: return sizeof(t_client_combat_obtain);
    case CMD_COMBAT_REQUEST: return sizeof(t_client_combat_request);
    case CMD_COMBAT_LEARN: return sizeof(t_client_combat_learn);
    case CMD_COMBAT_SELECT: return sizeof(t_client_combat_select);
    case CMD_COMBAT_ATTACK: return sizeof(t_client_combat_attack);
    case CMD_COMBAT_RESULT: return sizeof(t_client_combat_result);
    case CMD_COMBAT_RESET: return 0;
    case CMD_NPC_ATTACK:
    case CMD_NPC_THROW:
    case CMD_NPC_MAGIC:
    case CMD_ATTACK_RESULT:
    case CMD_ATTACK_RESULT_D:
    case CMD_THROW_RESULT_D:
    case CMD_MAGIC_RESULT_T:
    case CMD_MAGIC_RESULT_H:
    case CMD_MAGIC_RESULT_M:
    case CMD_COMBAT_RESULT_H:
    case CMD_COMBAT_RESULT_M:
    case CMD_COMBAT_RESULT_S:
    case CMD_COMBAT_RESULT_T:
    case CMD_COMBAT_INIT:
    case CMD_BATTLE_DAMAGE:
    case CMD_BATTLE_MESSAMGE: return ServerOnly;
    default: return Unrelated;
    }
}

inline bool IsPayloadAllowed(unsigned command, int size)
{
    const int expected = ExpectedPayloadSize(command);
    return size >= 0 && (expected == Unrelated ||
        (expected >= 0 && size == expected));
}

inline bool CanReportNpcHit(unsigned reporter, unsigned target,
    bool casterIsNpc, bool targetIsPlayer)
{
    return reporter != 0 && reporter == target && casterIsNpc && targetIsPlayer;
}
}
