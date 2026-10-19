#define NOMINMAX
#include <windows.h>
#include <iostream>
#include <cstdlib>
#pragma pack(push, 1)
#include "../Mapserver/HigherLayers/Network5.h"
#pragma pack(pop)
#include "../Mapserver/HigherLayers/ClientBattleIngress.h"

static void Require(bool value, const char* scenario)
{
    if (!value) { std::cerr << scenario << '\n'; std::exit(1); }
}

int main()
{
    using namespace DragonClientBattle;
    const unsigned clientCommands[] = {CMD_ATTACK, CMD_THROW_ATTACK,
        CMD_EFFECT_RESULT, CMD_MAGIC_SELECT, CMD_MAGIC_CASTING,
        CMD_MAGIC_EXECUTE, CMD_MAGIC_RESULT, CMD_MAGIC_RESULT_D, CMD_COMBAT_CLEAR,
        CMD_COMBAT_OBTAIN, CMD_COMBAT_REQUEST, CMD_COMBAT_LEARN, CMD_COMBAT_SELECT,
        CMD_COMBAT_ATTACK, CMD_COMBAT_RESULT, CMD_COMBAT_RESET,
        DragonCombatShot::HelloCommand, DragonCombatShot::HitCommand};
    for (unsigned command : clientCommands)
    {
        const int size = ExpectedPayloadSize(command);
        Require(size >= 0 && IsPayloadAllowed(command, size), "normal payload");
        Require(!IsPayloadAllowed(command, size - 1), "short payload");
        Require(!IsPayloadAllowed(command, size + 1), "trailing bytes");
        Require(!IsPayloadAllowed(command, size + 65536), "no word truncation");
    }
    const unsigned serverCommands[] = {CMD_NPC_ATTACK, CMD_NPC_THROW,
        CMD_NPC_MAGIC, CMD_ATTACK_RESULT, CMD_ATTACK_RESULT_D, CMD_THROW_RESULT_D,
        CMD_MAGIC_RESULT_H, CMD_MAGIC_RESULT_T, CMD_MAGIC_RESULT_M,
        CMD_COMBAT_RESULT_H, CMD_COMBAT_RESULT_M, CMD_COMBAT_RESULT_S,
        CMD_COMBAT_RESULT_T, CMD_COMBAT_INIT, CMD_BATTLE_DAMAGE, CMD_BATTLE_MESSAMGE,
        DragonCombatShot::AckCommand, DragonCombatShot::IssuedCommand,
        DragonCombatShot::BurstIssuedCommand, DragonCombatShot::ArrowIssuedCommand,
        DragonCombatShot::MagicIssuedCommand, CMD_THROW_RESULT};
    for (unsigned command : serverCommands)
    {
        Require(!IsPayloadAllowed(command, 0), "server command with empty payload");
        Require(!IsPayloadAllowed(command, 10), "server command with forged payload");
    }
    static_assert(CMD_EFFECT_RESULT == CMD_COMBAT_RESULT_C, "legacy alias");
    Require(IsPayloadAllowed(CMD_EFFECT_RESULT, sizeof(t_client_effect_result)),
        "legacy item effect alias remains usable");
    Require(IsPayloadAllowed(31400, 9), "unrelated protocol remains separate");
    Require(!IsPayloadAllowed(CMD_MAGIC_RESULT_D, sizeof(t_server_magic_execute)),
        "server payload cannot masquerade as NPC result");
    Require(CanReportNpcHit(10, 10, true, true), "own NPC hit report");
    Require(!CanReportNpcHit(10, 11, true, true), "cannot report another player");
    Require(!CanReportNpcHit(10, 10, false, true), "player cannot impersonate NPC");
    Require(!CanReportNpcHit(10, 10, true, false), "victim must be a player");
    Require(!CanReportNpcHit(0, 0, true, true), "invalid reporter");
    std::cout << "ClientBattleIngressTest passed\n";
}
