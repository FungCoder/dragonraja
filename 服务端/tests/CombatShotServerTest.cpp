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
    CONNECT_JOIN = 3, FIRE_ARROW = 29, ICE_ARROW = 38, TACTICS_Archery = 50, TACTICS_Hurl = 51,
    IK_THROW_EVENT = 60, WR_LONG1 = 70, WR_LONG2 = 71, WR_LONG3 = 72,
    CMD_SCRIPT_MESSAGE_BOX = 10273, TILE_SIZE = 32,
    REM_AREA_X = 21 * 32, REM_AREA_Y = 17 * 32, ALIVE_ = 1,
    Num_Of_Magic = 256, REMOVE_CURSE = 78, MP = 1, MK_WARNING = 2 };
static int MapNumber = 5;
static uint64_t nowMs = 100;
static DWORD g_curr_time = 10;
static struct { int exhaust_MP = 10, magic_Type = 51; } Magic_Ref[Num_Of_Magic];
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
    int Mana = 1000, manaUpdates = 0;
    bool stone = false, prepared = true, doing = true, learned = true, enabled = true;
    bool filterAllowed = true, battleAllowed = true;
    int preparedMagic = FIRE_ARROW;
    bool IsPlayer() const { return player; }
    bool IsDead() const { return Hp <= 0; }
    bool IsArrow() const { return hasArrow; }
    int GetServerID() const { return serverId; }
    int GetActiveCombat() const { return active; }
    int GetMagic() const { return magic; }
    int GetTacticsKind() const { return tactics; }
    int GetWeaponKind() const { return weapon; }
    unsigned GetCombatLevel(unsigned) const { return level; }
    bool IsStone() const { return stone; }
    bool CheckDoing(BYTE, DWORD) const { return doing; }
    bool CheckEnd(BYTE selected, DWORD) { magic = selected; return true; }
    void ClearMagic() { magic = 0; }
    void SendCharInfoBasic(int, int) { ++manaUpdates; }
    void Message(int, int, int) {}
    void SetWanted(bool) {}
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
static std::atomic<unsigned> magicSettlements(0);
static unsigned settledMagic = 0, filteredMagic = 0, prepareCalls = 0;
static bool CanExecutePlayerMagicArrow(int connection, BYTE magic)
{
    return connections[connection].chrlst.prepared && connections[connection].chrlst.doing &&
        connections[connection].chrlst.preparedMagic == magic;
}
static void ResetAutoPlaySession(int connection) { connections[connection].chrlst.prepared = false; }
struct MagicBattleFixture
{
    void SetIndex(int index) { settledMagic = index; }
    void SetBothID(WORD, WORD) {}
    void SetBothPtr(CHARLIST*, CHARLIST*) {}
    void SetCurrentTime(DWORD) {}
    void SetContinueTime(int) {}
    void SetPosition(int, int) {}
    void Execute() { ++magicSettlements; }
};
static MagicBattleFixture magicBattle;
struct CBattleManager
{
    void RecvAuthorizedThrowResult(int, t_client_throw_result*, int kind)
    {
        settledKind = kind;
        ++settlements;
    }
    bool PrepareMagicArrowShot(WORD, WORD, int, int, unsigned short&);
    void RecvMagicResult(WORD, t_client_magic_result*);
    void RecvAuthorizedMagicArrowResult(WORD, t_client_magic_result*);
    void RecvMagicResultImpl(WORD, t_client_magic_result*, bool);
    bool IsMagic(int magic) const { return magic > 0 && magic < 200; }
    bool IsValid(BYTE, CHARLIST* actor) const { return actor->learned; }
    bool IsValid(BYTE, int x, int y) const { return x >= 0 && y >= 0 && x < 100 && y < 100; }
    bool IsEnable(BYTE, CHARLIST* actor) const { return actor->enabled; }
    bool IsBattle(CHARLIST* actor, CHARLIST*) const { return actor->battleAllowed; }
    bool FilterMagic(CHARLIST* actor, CHARLIST*, int, int)
    {
        ++prepareCalls;
        if (!actor->filterAllowed) return false;
        actor->Mana -= Magic_Ref[actor->GetMagic()].exhaust_MP;
        return true;
    }
    bool FilterPlayerMagicResult(BYTE magic, CHARLIST* actor, CHARLIST*)
    {
        filteredMagic = magic;
        return actor->learned && actor->filterAllowed;
    }
    bool FilterResult(CHARLIST* actor, CHARLIST* target)
    { return FilterPlayerMagicResult(BYTE(actor->GetMagic()), actor, target); }
    MagicBattleFixture* GetBattle(int) { return &magicBattle; }
};
static CBattleManager battle;
static auto g_pBattleManager = &battle;
#include "../Mapserver/HigherLayers/PlayerSessionSafety.h"
#include "MagicArrowAuthorityProduction.inc"
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
    connections[1].chrlst.active = 0;
    nowMs += 201;
    RecvCombatShotHello(1, &hello, sizeof(hello));
    unsigned expected = settlements.load();
    for (unsigned skill : {unsigned(MultiFireSkill), unsigned(IcingBlastSkill),
        unsigned(FleetMissileSkill), unsigned(ConvertingArmorSkill), unsigned(SharkMissileSkill),
        unsigned(FireExtremeSkill), unsigned(IceExtremeSkill)})
    for (unsigned level = 1; level <= 3; ++level)
    {
        const auto burst = IssueBurstFor(skill, level);
        const unsigned count = skill == MultiFireSkill || skill == IcingBlastSkill ?
            (level == 3 ? 5 : 3) : (skill == FireExtremeSkill || skill == IceExtremeSkill ? 8 : 1);
        Require(burst.skill == skill && burst.level == level && burst.count == count,
            "count comes from the server skill level");
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
    connections[1].chrlst.active = 0;
    auto& caster = connections[1].chrlst;
    caster.tactics = TACTICS_Archery; caster.active = 0;
    for (unsigned magic : {unsigned(FIRE_ARROW), unsigned(ICE_ARROW)})
    {
        caster.magic = caster.preparedMagic = magic;
        caster.Mana = 100; caster.prepared = caster.doing = true;
        const unsigned oldMagicHits = magicSettlements;
        const unsigned oldPhysicalHits = settlements;
        Require(IssueCombatShot(1, 2, 100, 100), "magic arrow accepted via same release ledger");
        Require(sent.back().h.header.type == ArrowIssuedCommand && caster.Mana == 90,
            "server pays once before publishing magic-arrow action");
        ArrowIssued arrow; memcpy(&arrow, sent.back().u.data, sizeof(arrow));
        Require(arrow.magic == magic, "wire mode is chosen by server");
        t_client_magic_result oldHit = {};
        oldHit.nMagic = BYTE(magic); oldHit.idTarget = 2; oldHit.nX = oldHit.nY = 100;
        battle.RecvMagicResult(1, &oldHit);
        Require(magicSettlements == oldMagicHits && settlements == oldPhysicalHits && caster.Mana == 90,
            "unversioned magic-arrow result cannot spend or duplicate the action");
        const auto arrowHit = HitFor(arrow.shot);
        caster.magic = 30; // Old in-flight arrow must not change the next selected spell.
        Submit(arrowHit); Submit(arrowHit);
        Require(magicSettlements == oldMagicHits + 1 && settlements == oldPhysicalHits &&
            settledMagic == magic && filteredMagic == magic && caster.magic == 30 && caster.Mana == 90,
            "one immutable arrow mode settles once without overwriting next selection");
        caster.magic = magic; caster.Mana = 0;
        const auto fallback = Issue();
        battle.RecvMagicResult(1, &oldHit);
        Submit(HitFor(fallback)); Submit(HitFor(fallback));
        Require(settlements == oldPhysicalHits + 1 && magicSettlements == oldMagicHits + 1 && caster.Mana == 0,
            "insufficient mana grants only physical damage from the same one-action model");
        caster.Mana = 100; caster.prepared = false;
        caster.magic = magic;
        auto normal = Issue(); Submit(HitFor(normal));
        Require(caster.Mana == 100, "unprepared arrows cannot pay for or claim magic damage");
        caster.prepared = true; caster.doing = false;
        normal = Issue(); Submit(HitFor(normal));
        Require(caster.Mana == 100, "expired cast falls back to ordinary release");
        caster.doing = true; caster.learned = false;
        Require(!IssueCombatShot(1, 2, 100, 100) && caster.Mana == 100, "unlearned mode refused without payment");
        caster.learned = true; caster.enabled = false;
        Require(!IssueCombatShot(1, 2, 100, 100) && caster.Mana == 100, "disabled magic mode refused");
        caster.enabled = true; caster.filterAllowed = false;
        Require(!IssueCombatShot(1, 2, 100, 100) && caster.Mana == 100, "failed magic release filter grants no action");
        caster.filterAllowed = true;
        connections[2].chrlst.stone = true;
        Require(!IssueCombatShot(1, 2, 100, 100) && caster.Mana == 100, "stone target cannot charge a magic release");
        connections[2].chrlst.stone = false;
        Require(IssueCombatShot(1, 2, 100, 100), "concurrent arrow release");
        memcpy(&arrow, sent.back().u.data, sizeof(arrow));
        const auto concurrent = HitFor(arrow.shot);
        const unsigned beforeConcurrent = magicSettlements;
        std::thread arrowFirst([&] { Submit(concurrent); }), arrowReplay([&] { Submit(concurrent); });
        arrowFirst.join(); arrowReplay.join();
        Require(magicSettlements == beforeConcurrent + 1 && caster.Mana == 90, "concurrent arrow replay spends once");
        Require(IssueCombatShot(1, 2, 100, 100), "invalid arrow candidate release");
        memcpy(&arrow, sent.back().u.data, sizeof(arrow));
        auto badCandidate = HitFor(arrow.shot); badCandidate.target = 65000;
        Submit(badCandidate); Submit(HitFor(arrow.shot));
        Require(magicSettlements == beforeConcurrent + 1 && caster.Mana == 80,
            "invalid candidate still spends paid arrow and cannot retry");
        Require(IssueCombatShot(1, 2, 100, 100), "expiring paid arrow");
        memcpy(&arrow, sent.back().u.data, sizeof(arrow)); nowMs += LifetimeMs;
        Submit(HitFor(arrow.shot));
        Require(magicSettlements == beforeConcurrent + 1, "expired magic arrow cannot settle");
        CancelCombatShots(1);
        caster.Mana = 10000;
        for (unsigned index = 0; index < MaxOutstanding; ++index)
            Require(IssueCombatShot(1, 2, 100, 100), "paid arrow capacity");
        const int fullMana = caster.Mana;
        Require(!IssueCombatShot(1, 2, 100, 100) && caster.Mana == fullMana,
            "full action ledger rejects before payment");
        CancelCombatShots(1);
    }
    std::cout << "Production combat shot server tests passed\n";
}
