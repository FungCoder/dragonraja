#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <map>
#include <mutex>
#include <atomic>
#include <thread>
#include <vector>
#include <iostream>
#include <cstdlib>
#pragma pack(push, 1)
#include "../Mapserver/HigherLayers/Network5.h"
#pragma pack(pop)
#include "../Library/Shared/CombatShotLedger.h"
#include "../Mapserver/HigherLayers/CombatShotServer.h"

enum { DRAGON_CONNECTIONS_START = 1, DRAGON_MAX_CONNECTIONS = 8,
    CONNECT_JOIN = 3, FIRE_ARROW = 40, ICE_ARROW = 41, TACTICS_Archery = 50, TACTICS_Hurl = 51,
    IK_THROW_EVENT = 60, WR_LONG1 = 70, WR_LONG2 = 71, WR_LONG3 = 72,
    CMD_SCRIPT_MESSAGE_BOX = 10273, TILE_SIZE = 32,
    REM_AREA_X = 21 * 32, REM_AREA_Y = 17 * 32, ALIVE_ = 1 };
static int MapNumber = 5;
static uint64_t nowMs = 100;
static DWORD timeGetTime() { return DWORD(nowMs); }
static ULONGLONG TestTickCount64() { return nowMs; }
#define GetTickCount64 TestTickCount64
struct CHARLIST
{
    int X = 100, Y = 100, Hp = 100, serverId = 1;
    int active = 0, magic = 0, tactics = TACTICS_Archery, weapon = 0;
    unsigned level = 1;
    unsigned total_id = 123;
    bool player = true;
    int bAlive = ALIVE_;
    bool hasArrow = true;
    bool IsPlayer() const { return player; }
    bool IsDead() const { return Hp <= 0; }
    bool IsArrow() const { return hasArrow; }
    int GetServerID() const { return serverId; }
    int GetActiveCombat() const { return active; }
    int GetMagic() const { return magic; }
    int GetTacticsKind() const { return tactics; }
    int GetWeaponKind() const { return weapon; }
    unsigned GetCombatLevel(unsigned) const { return level; }
};
struct t_connection
{
    int state = CONNECT_JOIN;
    bool OnMapMoving = false;
    unsigned dwAgentConnectionIndex = 1, dwIDForAgent = 2;
    CHARLIST chrlst;
};
static t_connection connections[DRAGON_MAX_CONNECTIONS];
static CHARLIST* GetCharListPtr(unsigned id)
{
    return id >= 1 && id < DRAGON_MAX_CONNECTIONS ? &connections[id].chrlst : nullptr;
}
static struct { int wWidth = 100, wHeight = 100; } mapFile;
static auto g_lpMapFile = &mapFile;
struct t_packet
{
    struct { struct { unsigned short type = 0, size = 0; } header; } h;
    union { char data[512]; struct { char default_msg[512]; } kein; } u;
};
static std::vector<t_packet> sent;
static void QueuePacket(t_connection*, int, t_packet* packet, int) { sent.push_back(*packet); }
static std::atomic<unsigned> settlements(0);
static std::atomic<int> settledKind(0);
struct BattleFixture
{
    void RecvAuthorizedThrowResult(int, t_client_throw_result*, int kind)
    {
        settledKind = kind;
        ++settlements;
    }
};
static BattleFixture battle;
static auto g_pBattleManager = &battle;
#include "../Mapserver/HigherLayers/PlayerSessionSafety.h"
#include "CombatShotServerProduction.inc"

static void Require(bool ok, const char* scenario)
{
    if (!ok) { std::cerr << scenario << '\n'; std::exit(1); }
}
static DragonCombatShot::Issued Issue()
{
    Require(IssueCombatShot(1, 2, 100, 100), "server accepted normal release");
    Require(sent.back().h.header.type == DragonCombatShot::IssuedCommand, "versioned release packet");
    DragonCombatShot::Issued issued;
    memcpy(&issued, sent.back().u.data, sizeof(issued));
    return issued;
}
static DragonCombatShot::Hit HitFor(const DragonCombatShot::Issued& shot)
{
    return {DragonCombatShot::Version, shot.nonce, shot.action, 2, 100, 100, 0};
}
static void Submit(const DragonCombatShot::Hit& hit, int connection = 1)
{
    RecvCombatShotHit(connection, &hit, sizeof(hit));
}
static DragonCombatShot::BurstIssued IssueBurstFor(unsigned skill, unsigned level)
{
    using namespace DragonCombatShot;
    connections[1].chrlst.active = int(skill);
    connections[1].chrlst.level = level;
    connections[1].chrlst.tactics = skill == IcingBlastSkill ? TACTICS_Hurl : TACTICS_Archery;
    Require(UsesVersionedThrow(1), "migrated burst selector");
    Require(IssueCombatShot(1, 2, 100, 100), "server accepted burst release");
    Require(sent.back().h.header.type == BurstIssuedCommand, "burst release envelope");
    BurstIssued burst;
    memcpy(&burst, sent.back().u.data, sizeof(burst));
    return burst;
}
static DragonCombatShot::Hit BurstHitFor(const DragonCombatShot::BurstIssued& burst, unsigned part)
{
    return {DragonCombatShot::Version, burst.nonce, burst.actions[part], 2, 100, 100, 0};
}
int main()
{
    using namespace DragonCombatShot;
    for (int id = 1; id < DRAGON_MAX_CONNECTIONS; ++id)
    {
        connections[id].chrlst.serverId = id;
        connections[id].chrlst.total_id += id;
        connections[id].dwIDForAgent += id;
    }
    Require(!IssueCombatShot(1, 2, 100, 100), "no silent legacy fallback before handshake");
    const Hello hello = {Version, 99};
    RecvCombatShotHello(1, &hello, sizeof(hello));
    Require(sent.back().h.header.type == AckCommand, "capability reply");
    auto issued = Issue();
    auto hit = HitFor(issued);
    Submit(hit, 2);
    Require(settlements == 0, "cannot consume another player's action");
    ++hit.nonce; Submit(hit); --hit.nonce;
    Require(settlements == 0, "wrong session nonce");
    Submit(hit); Submit(hit);
    Require(settlements == 1 && settledKind == WR_LONG1, "normal result consumes exactly once");

    issued = Issue(); hit = HitFor(issued);
    hit.target = 3; Submit(hit);
    Require(settlements == 2, "legitimate interception can hit a different nearby entity");
    issued = Issue(); hit = HitFor(issued);
    hit.target = 65000; Submit(hit); hit.target = 2; Submit(hit);
    Require(settlements == 2, "invalid candidate spends the action before retry");
    issued = Issue(); hit = HitFor(issued);
    hit.x = -1; Submit(hit); hit.x = 100; Submit(hit);
    Require(settlements == 2, "invalid coordinates cannot retry same action");
    issued = Issue(); hit = HitFor(issued);
    connections[2].chrlst.X = 100 + REM_AREA_X + 1;
    Submit(hit); connections[2].chrlst.X = 100; Submit(hit);
    Require(settlements == 2, "out-of-area target cannot retry");
    issued = Issue(); hit = HitFor(issued);
    connections[1].chrlst.hasArrow = false;
    Submit(hit); connections[1].chrlst.hasArrow = true; Submit(hit);
    Require(settlements == 2, "missing ammunition rejects and spends the action");
    issued = Issue(); hit = HitFor(issued);
    nowMs += LifetimeMs; Submit(hit);
    Require(settlements == 2, "expired result");
    issued = Issue(); hit = HitFor(issued);
    connections[1].chrlst.active = 228;
    Submit(hit);
    Require(settlements == 3 && settledKind == WR_LONG1, "skill switch preserves released shot identity");
    connections[1].chrlst.active = 0;

    issued = Issue(); hit = HitFor(issued); CancelCombatShots(1); Submit(hit);
    Require(settlements == 3, "death clears in-flight actions");
    issued = Issue(); hit = HitFor(issued); Submit(hit);
    Require(settlements == 4, "revival uses negotiated session without relogin");
    issued = Issue(); hit = HitFor(issued);
    std::thread first([&] { Submit(hit); });
    std::thread duplicate([&] { Submit(hit); });
    first.join(); duplicate.join();
    Require(settlements == 5, "concurrent duplicate has one consumer");
    issued = Issue(); hit = HitFor(issued);
    ResetCombatShotSession(1); Submit(hit);
    Require(settlements == 5 && !IssueCombatShot(1, 2, 100, 100), "map/disconnect requires fresh handshake");
    Require(!AllowLegacyThrowResult(1), "normal action cannot use unversioned callback");
    connections[1].chrlst.active = 229;
    Require(!AllowLegacyThrowResult(1), "selecting a skill alone grants no legacy authority");
    RegisterLegacyThrowRelease(1);
    connections[1].chrlst.active = 0;
    Require(AllowLegacyThrowResult(1), "old advanced shot remains in flight after skill switch");
    CancelCombatShots(1);
    Require(!AllowLegacyThrowResult(1), "death cancels legacy migration authority");
    nowMs += 201;
    RecvCombatShotHello(1, &hello, sizeof(hello));
    unsigned expected = settlements.load();
    for (unsigned skill : {unsigned(MultiFireSkill), unsigned(IcingBlastSkill)})
    for (unsigned level = 1; level <= 3; ++level)
    {
        const auto burst = IssueBurstFor(skill, level);
        const unsigned count = level == 3 ? 5 : 3;
        Require(burst.skill == skill && burst.level == level && burst.count == count,
            "count comes from the server skill level");
        RegisterLegacyThrowRelease(1);
        Require(!AllowLegacyThrowResult(1), "migrated burst cannot open legacy callback authority");
        connections[1].chrlst.active = 0;
        connections[1].chrlst.level = 1;
        auto invented = BurstHitFor(burst, 0);
        invented.action = burst.actions[count - 1] + 1;
        Submit(invented);
        Require(settlements == expected, "invented extra projectile rejected");
        for (unsigned part = 0; part < count; ++part)
        {
            const auto hitPart = BurstHitFor(burst, part);
            Submit(hitPart); ++expected;
            Submit(hitPart);
            Require(settlements == expected, "each projectile settles once for the same target");
        }
        Require(settledKind == (skill == IcingBlastSkill ? WR_LONG2 : WR_LONG1),
            "released attack family survives skill switching");
    }
    auto burst = IssueBurstFor(MultiFireSkill, 3);
    auto firstHit = BurstHitFor(burst, 0);
    std::thread burstFirst([&] { Submit(firstHit); });
    std::thread burstDuplicate([&] { Submit(firstHit); });
    burstFirst.join(); burstDuplicate.join(); ++expected;
    Require(settlements == expected, "concurrent replay cannot consume another projectile");
    auto secondHit = BurstHitFor(burst, 1), thirdHit = BurstHitFor(burst, 2);
    std::thread secondPart([&] { Submit(secondHit); });
    std::thread thirdPart([&] { Submit(thirdHit); });
    secondPart.join(); thirdPart.join(); expected += 2;
    Require(settlements == expected, "distinct projectiles consume independently");
    CancelCombatShots(1);
    for (unsigned part = 0; part < burst.count; ++part) Submit(BurstHitFor(burst, part));
    Require(settlements == expected, "death cancels all remaining parts");
    burst = IssueBurstFor(IcingBlastSkill, 1);
    auto invalid = BurstHitFor(burst, 0); invalid.target = 65000;
    Submit(invalid); Submit(BurstHitFor(burst, 0));
    Require(settlements == expected, "failed first projectile remains spent");
    Submit(BurstHitFor(burst, 1)); ++expected;
    Require(settlements == expected, "failed candidate does not discard other projectiles");
    nowMs += LifetimeMs;
    Submit(BurstHitFor(burst, 2));
    Require(settlements == expected, "burst lifetime is shared by all parts");
    connections[1].chrlst.active = MultiFireSkill;
    connections[1].chrlst.level = 0;
    const size_t beforeInvalid = sent.size();
    Require(!IssueCombatShot(1, 2, 100, 100) && sent.size() == beforeInvalid,
        "unlearned burst cannot mint projectile IDs");
    std::cout << "Production combat shot server tests passed\n";
}
