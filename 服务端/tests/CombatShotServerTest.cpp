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
    Num_Of_Magic = 256, REMOVE_CURSE = 78, MP = 1, MK_WARNING = 2, MK_NORMAL = 3,
    FIRE_BALL = 30, ICE_BALL = 39, CON_FROZEN = 2, FLYING_SPARK = 89, BLITZ_SPEAR = 100,
    DEATH_FIRE = 87, SHADOW_OF_EVIL = 98, FLAME_ARROW = 110 };
static int MapNumber = 5;
static uint64_t nowMs = 100;
static DWORD g_curr_time = 10;
static struct { int exhaust_MP = 10, magic_Type = 51, Excute_Type = 0, continue_Time = 30; } Magic_Ref[Num_Of_Magic];
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
    bool filterAllowed = true, battleAllowed = true, endAllowed = true;
    int preparedMagic = FIRE_ARROW;
    bool locked = true;
    BYTE state = 0;
    BYTE GetState() const { return state; }
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
    bool CheckDoing(BYTE skill, DWORD) const { return doing && (Magic_Ref[skill].Excute_Type == 0 || locked); }
    bool CheckEnd(BYTE selected, DWORD) { if (!endAllowed) return false; magic = selected; return true; }
    void ClearMagic() { magic = 0; }
    void ClearMagic(bool lock) { locked = lock; }
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
    union { char data[512]; t_magic magic; struct { char default_msg[512]; } kein; } u;
};
static std::vector<t_packet> sent;
static void QueuePacket(t_connection*, int, t_packet* packet, int) { sent.push_back(*packet); }
static std::vector<t_packet> observed;
static void CastMe2Other(int, t_packet* packet) { observed.push_back(*packet); }
static struct
{
    void SaveMagicExecute(int, CHARLIST*, CHARLIST*, int, int) {}
} logManager;
static auto g_pLogManager = &logManager;
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
static bool CanExecutePlayerMagic(int connection, BYTE magic)
{
    return connections[connection].chrlst.prepared &&
        connections[connection].chrlst.preparedMagic == magic &&
        connections[connection].chrlst.GetMagic() == magic;
}
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
    void RecvAuthorizedMagicResult(WORD, t_client_magic_result*);
    void RecvMagicResultImpl(WORD, t_client_magic_result*, bool);
    void RecvMagicExecute(WORD, t_client_magic_execute*);
    bool PrepareProjectileMagicExecute(WORD, t_client_magic_execute*);
    bool RecvMagicExecuteImpl(WORD, t_client_magic_execute*, bool);
    t_packet m_packet = {};
    bool IsMagic(int magic) const { return magic > 0 && magic < 200; }
    bool IsValid(BYTE, CHARLIST* actor) const { return actor->learned; }
    bool IsValid(BYTE, int x, int y) const { return x >= 0 && y >= 0 && x < 100 && y < 100; }
    bool IsEnable(BYTE, CHARLIST* actor) const { return actor->enabled; }
    bool IsBattle(CHARLIST* actor, CHARLIST*) const { return actor->battleAllowed; }
    bool FilterMagic(CHARLIST* actor, CHARLIST*, int, int)
    {
        ++prepareCalls;
        if (!actor->filterAllowed || !actor->learned ||
            actor->Mana < Magic_Ref[actor->GetMagic()].exhaust_MP) return false;
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
#include "MagicExecuteAuthorityProduction.inc"
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
    const Hello previousProtocol = {6, 99};
    RecvCombatShotHello(1, &previousProtocol, sizeof(previousProtocol));
    Require(!IssueCombatShot(1, 2, 100, 100), "published protocol-six handshake cannot authorize protocol-seven releases");
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
    for (unsigned magic : {unsigned(FireBallMagic), unsigned(IceBallMagic)})
    {
        CancelCombatShots(1);
        caster.active = 0; caster.magic = caster.preparedMagic = magic;
        caster.prepared = caster.doing = caster.locked = true;
        caster.Mana = 100; caster.learned = caster.filterAllowed = true;
        caster.tactics = TACTICS_Archery; caster.hasArrow = false;
        t_client_magic_execute request = {};
        request.nMagic = BYTE(magic); request.idTarget = 2; request.nX = request.nY = 100;
        const unsigned beforeSpell = magicSettlements;
        battle.RecvMagicExecute(1, &request);
        Require(sent.back().h.header.type == MagicIssuedCommand && caster.Mana == 90,
            "real execute handler pays once and publishes one spell instance");
        ArrowIssued cast; memcpy(&cast, sent.back().u.data, sizeof(cast));
        Require(cast.magic == magic && observed.back().h.header.type == CMD_MAGIC_EXECUTE,
            "owner receives instance while observer keeps original execution message");
        t_client_magic_result oldResult = {};
        oldResult.nMagic = BYTE(magic); oldResult.idTarget = 2; oldResult.nX = oldResult.nY = 100;
        battle.RecvMagicResult(1, &oldResult);
        Require(magicSettlements == beforeSpell, "old spell result cannot bypass instance");
        caster.magic = FIRE_ARROW;
        Submit(HitFor(cast.shot)); Submit(HitFor(cast.shot));
        Require(magicSettlements == beforeSpell + 1 && settledMagic == magic &&
            filteredMagic == magic && caster.magic == FIRE_ARROW && caster.Mana == 90,
            "cast settles once with snapshot type without requiring arrows or changing selection");
        caster.hasArrow = true; caster.magic = magic;
        Require(!IssueProjectileMagic(1, &request, sizeof(request)) && caster.Mana == 90,
            "rapid repeat is refused by server before payment");
        nowMs += MagicRepeatIntervalMs;
        Require(IssueProjectileMagic(1, &request, sizeof(request)), "type-zero paid repeat remains legal");
        ArrowIssued repeat; memcpy(&repeat, sent.back().u.data, sizeof(repeat));
        Require(repeat.shot.action != cast.shot.action && caster.Mana == 80,
            "a second accepted execute has a new paid instance");
        auto failedCandidate = HitFor(repeat.shot); failedCandidate.target = 65000;
        Submit(failedCandidate); Submit(HitFor(repeat.shot));
        Require(magicSettlements == beforeSpell + 1, "invalid candidate spends spell before retries");
        Magic_Ref[magic].Excute_Type = 1; caster.locked = true;
        nowMs += MagicRepeatIntervalMs;
        Require(IssueProjectileMagic(1, &request, sizeof(request)), "one-use cast accepted");
        ArrowIssued oneUse; memcpy(&oneUse, sent.back().u.data, sizeof(oneUse));
        nowMs += MagicRepeatIntervalMs;
        Require(!IssueProjectileMagic(1, &request, sizeof(request)) && caster.Mana == 70 &&
            sent.back().h.header.type == CMD_MAGIC_EXECUTE && sent.back().u.magic.server_magic_execute.bFailed,
            "one-use preparation refuses second execute before payment with original failure response");
        Submit(HitFor(oneUse.shot));
        Require(magicSettlements == beforeSpell + 2, "accepted in-flight cast survives later failed execute");
        Magic_Ref[magic].Excute_Type = 0;
        caster.magic = caster.preparedMagic = magic; caster.prepared = caster.doing = true;
        caster.Mana = 0;
        Require(!IssueProjectileMagic(1, &request, sizeof(request)) && caster.Mana == 0,
            "insufficient spell mana cannot mint instance");
        caster.Mana = 100; caster.magic = magic; caster.prepared = false;
        Require(!IssueProjectileMagic(1, &request, sizeof(request)) && caster.Mana == 100,
            "execute without actual preparation is refused");
        caster.prepared = true; caster.learned = false;
        Require(!IssueProjectileMagic(1, &request, sizeof(request)) && caster.Mana == 100,
            "unlearned spell cannot receive instance");
        caster.learned = true; caster.magic = magic; caster.prepared = true;
        request.nX = -1;
        Require(!IssueProjectileMagic(1, &request, sizeof(request)) && caster.Mana == 100,
            "invalid execution position is refused before payment");
        request.nX = 100;
        Require(IssueProjectileMagic(1, &request, sizeof(request)), "expiring spell accepted");
        memcpy(&cast, sent.back().u.data, sizeof(cast)); nowMs += LifetimeMs;
        Submit(HitFor(cast.shot));
        Require(magicSettlements == beforeSpell + 2, "expired spell cannot settle");
        CancelCombatShots(1);
        Require(IssueProjectileMagic(1, &request, sizeof(request)), "death cancellation spell accepted");
        memcpy(&cast, sent.back().u.data, sizeof(cast)); CancelCombatShots(1);
        Submit(HitFor(cast.shot));
        Require(magicSettlements == beforeSpell + 2, "death cancels paid spell");
        caster.Mana = 10000;
        nowMs += MagicRepeatIntervalMs;
        Require(IssueProjectileMagic(1, &request, sizeof(request)), "shared ledger first spell slot");
        caster.magic = 0;
        for (unsigned index = 1; index < MaxOutstanding; ++index)
            Require(IssueCombatShot(1, 2, 100, 100), "shared ledger remaining physical slots");
        caster.magic = magic;
        const int fullMana = caster.Mana;
        nowMs += MagicRepeatIntervalMs;
        Require(!IssueProjectileMagic(1, &request, sizeof(request)) && caster.Mana == fullMana,
            "physical and magic casts share capacity before any payment");
        CancelCombatShots(1);
        ResetCombatShotSession(1);
        Require(!IssueProjectileMagic(1, &request, sizeof(request)) && caster.Mana == fullMana,
            "unnegotiated spell does not charge mana");
        nowMs += 201; RecvCombatShotHello(1, &hello, sizeof(hello));
    }
    for (unsigned magic : {unsigned(FlyingSparkMagic), unsigned(BlitzSpearMagic),
        unsigned(DeathFireMagic), unsigned(ShadowOfEvilMagic), unsigned(FlameArrowMagic)})
    {
        ResetCombatShotSession(1); nowMs += 201;
        RecvCombatShotHello(1, &hello, sizeof(hello));
        caster.active = 0; caster.magic = caster.preparedMagic = magic;
        caster.prepared = caster.doing = caster.locked = caster.learned = caster.filterAllowed = true;
        caster.Mana = 10000; caster.hasArrow = false;
        t_client_magic_execute request = {};
        request.nMagic = BYTE(magic); request.idTarget = 1; request.nX = 120; request.nY = 140;
        const unsigned beforeMagic = magicSettlements, beforePrepare = prepareCalls;
        battle.RecvMagicExecute(1, &request);
        Require(sent.back().h.header.type == MagicBurstIssuedCommand &&
            sent.back().h.header.size == sizeof(BurstIssued) && caster.Mana == 9990 &&
            prepareCalls == beforePrepare + 1, "multi-projectile spell pays once through original execute handler");
        BurstIssued cast = {}; memcpy(&cast, sent.back().u.data, sizeof(cast));
        Require(cast.skill == magic && cast.count == MagicBurstParts(magic) && cast.level == 0 &&
            cast.target == 1 && cast.x == 120 && cast.y == 140,
            "server fixes 3/5 child budget and preserves ground-aim execution");
        for (unsigned part = 0; part < MaxBurstParts; ++part)
            Require(cast.actions[part] == (part < cast.count ? cast.actions[0] + part : 0),
                "contiguous unique child IDs with zero unused slots");
        t_client_magic_result legacy = {};
        legacy.nMagic = BYTE(magic); legacy.idTarget = 2; legacy.nX = legacy.nY = 100;
        battle.RecvMagicResult(1, &legacy);
        Require(magicSettlements == beforeMagic, "legacy magic result cannot bypass child budget");
        caster.magic = FireArrowMagic;
        auto first = BurstHitFor(cast, 0);
        std::thread firstHit([&] { Submit(first); }), replay([&] { Submit(first); });
        firstHit.join(); replay.join();
        auto invalid = BurstHitFor(cast, 1); invalid.target = 65000;
        Submit(invalid); Submit(BurstHitFor(cast, 1));
        for (unsigned part = 2; part < cast.count; ++part)
        { Submit(BurstHitFor(cast, part)); Submit(BurstHitFor(cast, part)); }
        Require(magicSettlements == beforeMagic + cast.count - 1 && caster.Mana == 9990 &&
            settledMagic == magic && filteredMagic == magic && caster.magic == FireArrowMagic,
            "each valid child settles once with snapshot type; invalid child spent independently");
        auto extra = first; extra.action = cast.actions[0] + cast.count;
        Submit(extra);
        Require(magicSettlements == beforeMagic + cast.count - 1, "unissued extra child is refused");
        caster.magic = magic;
        Require(!IssueProjectileMagic(1, &request, sizeof(request)) && caster.Mana == 9990,
            "multi-projectile execution shares server cadence before payment");
        nowMs += MagicRepeatIntervalMs;
        Require(IssueProjectileMagic(1, &request, sizeof(request)), "next batch accepted after cadence");
        BurstIssued expired = {}; memcpy(&expired, sent.back().u.data, sizeof(expired));
        Require(expired.actions[0] > cast.actions[cast.count - 1], "new batch IDs never overlap old children");
        nowMs += LifetimeMs;
        for (unsigned part = 0; part < expired.count; ++part) Submit(BurstHitFor(expired, part));
        Require(magicSettlements == beforeMagic + cast.count - 1, "entire expired batch cannot settle");
        Require(IssueProjectileMagic(1, &request, sizeof(request)), "cancellable batch accepted");
        memcpy(&expired, sent.back().u.data, sizeof(expired)); CancelCombatShots(1);
        for (unsigned part = 0; part < expired.count; ++part) Submit(BurstHitFor(expired, part));
        Require(magicSettlements == beforeMagic + cast.count - 1, "death cancellation removes all children");
        nowMs += MagicRepeatIntervalMs; caster.Mana = 0;
        Require(!IssueProjectileMagic(1, &request, sizeof(request)), "unpaid batch refused");
        caster.Mana = 10000; caster.magic = magic; caster.prepared = true;
        Require(IssueProjectileMagic(1, &request, sizeof(request)), "batch occupies one shared ledger slot");
        caster.magic = 0; caster.hasArrow = true;
        for (unsigned index = 1; index < MaxOutstanding; ++index)
            Require(IssueCombatShot(1, 2, 100, 100), "physical releases fill remaining shared slots");
        caster.magic = magic; nowMs += MagicRepeatIntervalMs;
        const int fullMana = caster.Mana;
        Require(!IssueProjectileMagic(1, &request, sizeof(request)) && caster.Mana == fullMana,
            "magic batch shares 64 release slots without charging rejected batch");
        CancelCombatShots(1);
    }
    ResetCombatShotSession(1); nowMs += 201; RecvCombatShotHello(1, &hello, sizeof(hello));
    caster.magic = caster.preparedMagic = FireBallMagic; caster.prepared = true;
    caster.Mana = 100; caster.state = 0;
    t_client_magic_execute timed = {};
    timed.nMagic = FireBallMagic; timed.idTarget = 2; timed.nX = timed.nY = 100;
    Require(IssueProjectileMagic(1, &timed, sizeof(timed)), "cadence session initial cast");
    nowMs += 201;
    const Hello renewed = {Version, 100};
    RecvCombatShotHello(1, &renewed, sizeof(renewed));
    Require(!IssueProjectileMagic(1, &timed, sizeof(timed)) && caster.Mana == 90,
        "nonce replacement cannot reset same-character execution cadence");
    nowMs += MagicRepeatIntervalMs - 201;
    Require(IssueProjectileMagic(1, &timed, sizeof(timed)) && caster.Mana == 80,
        "exact execution interval boundary is legal");
    caster.state = CON_FROZEN; nowMs += MagicRepeatIntervalMs;
    Require(!IssueProjectileMagic(1, &timed, sizeof(timed)) && caster.Mana == 80,
        "frozen state doubles existing execution interval");
    nowMs += MagicRepeatIntervalMs;
    Require(IssueProjectileMagic(1, &timed, sizeof(timed)) && caster.Mana == 70,
        "frozen execution interval boundary is legal");
    {
        DragonGameplay::WallExecutionGate gate;
        const DragonGameplay::PreparationIdentity owner = {1,2,3,4};
        auto other = owner; ++other.character;
        Require(!gate.CanReport(owner, DragonGameplay::FireWallMagic, 0), "selection is not a paid wall execute");
        Require(gate.Record(owner, DragonGameplay::FireWallMagic, 500, 0), "zero-second legacy wall keeps its deadline second");
        Require(!gate.CanReport(owner, DragonGameplay::FireWallMagic, 499) &&
            gate.CanReport(owner, DragonGameplay::FireWallMagic, 1499) &&
            !gate.CanReport(owner, DragonGameplay::FireWallMagic, 1500), "monotonic wall proof and whole-second boundary");
        Require(gate.Record(owner, DragonGameplay::FireWallMagic, UINT64_C(1)<<32, 30), "wall proof beyond old clock wrap");
        Require(gate.Record(owner, DragonGameplay::IceWallMagic, (UINT64_C(1)<<32) + 1, 60), "independent ice wall payment");
        Require(!gate.Record(owner, DragonGameplay::FireWallMagic, 0, -1) &&
            !gate.Record(owner, FireBallMagic, 0, 30), "negative configuration and unrelated spells cannot create proof");
        Require(gate.CanReport(owner, DragonGameplay::FireWallMagic, (UINT64_C(1)<<32) + 1000) &&
            gate.CanReport(owner, DragonGameplay::IceWallMagic, (UINT64_C(1)<<32) + 1000) &&
            !gate.CanReport(other, DragonGameplay::FireWallMagic, (UINT64_C(1)<<32) + 1000), "paid walls retain separate identity-scoped windows");
        Require(!gate.CanReport(owner, DragonGameplay::FireWallMagic, UINT64_C(2)<<32), "clock wrap cannot revive old paid wall");
    }
    for (unsigned magic : {unsigned(DragonGameplay::FireWallMagic), unsigned(DragonGameplay::IceWallMagic)})
    {
        ResetCombatShotSession(1); caster.state = 0; caster.Hp = 100; caster.bAlive = ALIVE_;
        caster.magic = caster.preparedMagic = magic;
        caster.prepared = caster.doing = caster.locked = caster.learned = caster.filterAllowed = caster.battleAllowed = true;
        caster.Mana = 100;
        t_client_magic_execute execute = {};
        execute.nMagic = BYTE(magic); execute.idTarget = 1; execute.nX = execute.nY = 100;
        t_client_magic_result result = {};
        result.nMagic = BYTE(magic); result.idTarget = 2; result.nX = result.nY = 100;
        const unsigned beforeWall = magicSettlements;
        battle.RecvMagicResult(1, &result);
        Require(magicSettlements == beforeWall && !CanReportPaidWallMagic(1, magic), "prepared-only wall cannot damage before paid execute");
        caster.filterAllowed = false;
        battle.RecvMagicExecute(1, &execute);
        caster.filterAllowed = true; caster.magic = magic; caster.prepared = true;
        Require(!CanReportPaidWallMagic(1, magic) && caster.Mana == 100, "failed payment cannot authorize wall results");
        battle.RecvMagicResult(1, &result);
        Require(magicSettlements == beforeWall, "failed execute has no result fallback");
        const unsigned payments = prepareCalls;
        Magic_Ref[magic].continue_Time = -1;
        battle.RecvMagicExecute(1, &execute);
        Require(caster.Mana == 100 && prepareCalls == payments && !CanReportPaidWallMagic(1, magic), "invalid duration rejected before mana filter");
        Magic_Ref[magic].continue_Time = 30;
        caster.magic = magic; caster.prepared = true;
        battle.RecvMagicExecute(1, &execute);
        Require(CanReportPaidWallMagic(1, magic) && caster.Mana == 90 &&
            sent.back().h.header.type == CMD_MAGIC_EXECUTE && !sent.back().u.magic.server_magic_execute.bFailed,
            "successful original execute pays once and authorizes wall with original response");
        Require(!CanReportPaidWallMagic(1, magic == DragonGameplay::FireWallMagic ? DragonGameplay::IceWallMagic : DragonGameplay::FireWallMagic),
            "one wall payment does not authorize another wall type");
        caster.endAllowed = false;
        battle.RecvMagicResult(1, &result);
        Require(magicSettlements == beforeWall && CanReportPaidWallMagic(1, magic),
            "paid proof cannot bypass the original CheckEnd duration");
        caster.endAllowed = true;
        caster.magic = FireBallMagic;
        battle.RecvMagicResult(1, &result);
        battle.RecvMagicResult(1, &result);
        Require(magicSettlements == beforeWall + 2 && settledMagic == magic && filteredMagic == magic && caster.Mana == 90,
            "original wall can hit multiple times without extra mana or changed damage handler");
        caster.magic = magic; caster.prepared = true; caster.Mana = 0;
        battle.RecvMagicExecute(1, &execute);
        caster.Mana = 90; caster.magic = magic; caster.prepared = true;
        Require(CanReportPaidWallMagic(1, magic), "later failed execute cannot erase an already paid wall");
        ++connections[1].dwIDForAgent;
        battle.RecvMagicResult(1, &result);
        Require(magicSettlements == beforeWall + 2, "reused connection cannot inherit paid wall");
        --connections[1].dwIDForAgent;
        ++MapNumber;
        Require(!CanReportPaidWallMagic(1, magic), "wall proof is map-scoped");
        --MapNumber;
        nowMs += 31000;
        RecvCombatShotHello(1, &renewed, sizeof(renewed));
        battle.RecvMagicResult(1, &result);
        Require(!CanReportPaidWallMagic(1, magic) && magicSettlements == beforeWall + 2,
            "new handshake cannot renew expired paid wall");
        battle.RecvMagicExecute(1, &execute);
        Require(CanReportPaidWallMagic(1, magic) && caster.Mana == 80, "new paid execute creates new proof");
        caster.Hp = 0;
        Require(!CanReportPaidWallMagic(1, magic), "dead caster cannot report a wall");
        CancelCombatShots(1); caster.Hp = 100;
        Require(!CanReportPaidWallMagic(1, magic), "death cancellation prevents wall revival");
        caster.magic = magic; caster.prepared = true;
        battle.RecvMagicExecute(1, &execute);
        Require(CanReportPaidWallMagic(1, magic), "paid wall before disconnect");
        ResetCombatShotSession(1);
        Require(!CanReportPaidWallMagic(1, magic), "disconnect/map reset clears paid wall proof");
    }
    std::cout << "Production combat shot server tests passed\n";
}
