#define NOMINMAX
#include <windows.h>
#include <iostream>
#include <cstdlib>
#pragma pack(push, 1)
#include "../Mapserver/HigherLayers/Network5.h"
#pragma pack(pop)
#include "../Mapserver/HigherLayers/ClientBattleIngress.h"

enum {DRAGON_CONNECTIONS_START = 1, DRAGON_MAX_CONNECTIONS = 8,
    CONNECT_JOIN = 3, Num_Of_Magic = 256};
struct CHARLIST
{
    bool npc;
    int serverId;
    unsigned total_id = 1;
    bool IsPlayer() const { return !npc; }
    bool IsNpc() const { return npc; }
    int GetServerID() const { return serverId; }
};
struct t_connection
{
    int state; bool OnMapMoving; CHARLIST chrlst;
    unsigned dwAgentConnectionIndex = 0, dwIDForAgent = 0;
};
static t_connection connections[DRAGON_MAX_CONNECTIONS] = {};
static int MapNumber = 1;
struct Header { unsigned short type; int size; };
struct t_packet
{
    struct { Header header; } h;
    union { t_magic magic; t_strike strike; t_combat combat; } u;
};
static struct { int magic_Type; } Magic_Ref[Num_Of_Magic] = {};
static CHARLIST monster = {true, 10000};
static CHARLIST* GetCharListPtr(unsigned id)
{
    if (id == 10000) return &monster;
    if (id >= 1 && id < DRAGON_MAX_CONNECTIONS) return &connections[id].chrlst;
    return nullptr;
}
static int dispatches;
static int HandleCommand(int, t_packet*) { ++dispatches; return 1; }
static bool allowLegacyShot = false;
static bool AllowLegacyThrowResult(int) { return allowLegacyShot; }
#include "../Mapserver/HigherLayers/PlayerSessionSafety.h"
#include "ClientBattleIngressProduction.inc"

static void Require(bool value, const char* scenario)
{
    if (!value) { std::cerr << scenario << '\n'; std::exit(1); }
}
int main()
{
    connections[1] = {CONNECT_JOIN, false, {false, 1}};
    connections[2] = {CONNECT_JOIN, false, {false, 2}};
    t_packet packet = {};
    packet.h.header = {CMD_ATTACK, sizeof(t_player_attack)};
    Require(HandleUserCommand(1, &packet) == 1 && dispatches == 1, "normal attack dispatch");
    packet.h.header.size = sizeof(t_player_attack) - 1;
    Require(HandleUserCommand(1, &packet) < 0 && dispatches == 1, "short packet no side effects");
    packet.h.header.size = sizeof(t_player_attack) + 1;
    Require(HandleUserCommand(1, &packet) < 0 && dispatches == 1, "trailing packet no side effects");
    packet.h.header = {CMD_NPC_ATTACK, sizeof(t_npc_attack)};
    Require(HandleUserCommand(1, &packet) < 0 && dispatches == 1, "cannot initiate NPC attack");
    packet.h.header = {CMD_NPC_MAGIC, sizeof(t_client_npc_magic)};
    packet.u.magic.client_npc_magic.bTimeDisable = true;
    Require(HandleUserCommand(1, &packet) < 0 && dispatches == 1, "cannot bypass NPC timing");
    packet.h.header = {CMD_MAGIC_RESULT_D, sizeof(t_magic_result_d)};
    packet.u.magic.magic_result_d = {10, 10000, 1, 32, 32};
    Magic_Ref[10].magic_Type = 50;
    Require(HandleUserCommand(1, &packet) == 1 && dispatches == 2, "own NPC collision");
    packet.u.magic.magic_result_d.idTarget = 2;
    Require(HandleUserCommand(1, &packet) < 0 && dispatches == 2, "cannot substitute victim");
    packet.u.magic.magic_result_d.idTarget = 1;
    packet.u.magic.magic_result_d.idCaster = 2;
    Require(HandleUserCommand(1, &packet) < 0 && dispatches == 2, "cannot substitute player attacker");
    packet.u.magic.magic_result_d.idCaster = 10001;
    Require(HandleUserCommand(1, &packet) < 0 && dispatches == 2, "missing NPC");
    packet.u.magic.magic_result_d.idCaster = 10000;
    Magic_Ref[10].magic_Type = 0;
    Require(HandleUserCommand(1, &packet) < 0 && dispatches == 2, "invalid battle dispatch group");
    packet.u.magic.magic_result_d.nMagic = 0;
    Require(HandleUserCommand(1, &packet) < 0 && dispatches == 2, "undefined magic");
    packet.h.header = {CMD_ATTACK, sizeof(t_player_attack)};
    connections[1].state = 0;
    Require(HandleUserCommand(1, &packet) < 0 && dispatches == 2, "not joined");
    connections[1].state = CONNECT_JOIN;
    connections[1].OnMapMoving = true;
    Require(HandleUserCommand(1, &packet) < 0 && dispatches == 2, "moving map");
    connections[1].OnMapMoving = false;
    connections[1].chrlst.serverId = 2;
    Require(HandleUserCommand(1, &packet) < 0 && dispatches == 2, "session identity mismatch");
    Require(HandleUserCommand(0, &packet) < 0, "zero connection");
    Require(HandleUserCommand(DRAGON_MAX_CONNECTIONS, &packet) < 0, "upper connection boundary");
    Require(HandleUserCommand(1, nullptr) < 0, "null packet");
    connections[1].chrlst.serverId = 1;
    packet.h.header = {CMD_THROW_RESULT, sizeof(t_client_throw_result)};
    Require(HandleUserCommand(1, &packet) < 0 && dispatches == 2,
        "ordinary shot cannot fall back to a result without an action");
    allowLegacyShot = true;
    Require(HandleUserCommand(1, &packet) == 1 && dispatches == 3,
        "separate legacy migration mode remains explicit");
    std::cout << "Production client battle dispatch tests passed\n";
}
