#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <iostream>
#include <cstdlib>
#include <vector>
#pragma pack(push, 1)
#include "../Client/Debug/Network5.h"
#pragma pack(pop)
#include "../Client/Debug/CombatShotClient.h"
#include "../Client/Debug/CombatShotClientState.h"
enum { THROW_ARROW = 194, THROW_KNIFE = 195, THROW_FIRE_ARROW = 665,
    THROW_ICE_KNIFE = 666, THROW_SNOW_BALL = 193, THROW_WATER_BALL = 661,
    THROW_BIG_GULP = 663, MK_WARNING = 1 };
static struct CharacterFixture
{
    unsigned id = 1;
    bool dead = false;
    int nActiveCombat = 0, nActiveLevel = 0, excute_MagicNum = 0;
    int direction = 0, height = 0, x = 100, y = 100;
} hero;
static auto Hero = &hero;
static bool IsDead(decltype(Hero) player) { return player->dead; }
static DWORD g_ClientTime = 100;
struct t_packet
{
    struct { struct { unsigned short type = 0, size = 0; } header; } h;
    union { char data[512]; } u;
};
static std::vector<t_packet> sent;
static void QueuePacket(t_packet* packet, int) { sent.push_back(*packet); }
static unsigned animations = 0, warnings = 0;
struct BattleFixture
{
    void RecvThrowAttack(t_server_throw_attack* packet)
    {
        // This hook is also present in the real battle handler.
        CombatShotForgetPending();
        Hero->nActiveCombat = packet->nCombat;
        ++animations;
    }
    void Message(int, const char*, ...) { ++warnings; }
};
static BattleFixture g_mgrBattle;
#include "CombatShotClientProduction.inc"
#include "CombatBurstGeneratorFixture.h"
static void Require(bool ok, const char* scenario)
{
    if (!ok) { std::cerr << scenario << '\n'; std::exit(1); }
}
int main()
{
    using namespace DragonCombatShot;
    CombatShotBegin();
    Require(CombatShotNeedsHandshake(0, 0) && CombatShotNeedsHandshake(0, 3),
        "ordinary bow and thrown weapon require secure capability");
    Require(CombatShotNeedsHandshake(MultiFireSkill, 0) && CombatShotNeedsHandshake(IcingBlastSkill, 3),
        "migrated burst skills require negotiated capability");
    Require(!CombatShotNeedsHandshake(229, 0) && !CombatShotNeedsHandshake(0, 1) &&
        !CombatShotNeedsHandshake(0, 2), "explicit advanced/magic-arrow migration scope");
    Require(sent.size() == 1 && sent.back().h.header.type == HelloCommand,
        "begin handshake after map ready");
    Hello hello;
    memcpy(&hello, sent.back().u.data, sizeof(hello));
    Ack ack = {Version, hello.nonce + 1};
    CombatShotReceive(AckCommand, &ack, sizeof(ack));
    Require(!CombatShotReady(), "stale acknowledgement");
    ack.nonce = hello.nonce;
    CombatShotReceive(AckCommand, &ack, sizeof(ack) - 1);
    Require(!CombatShotReady(), "short acknowledgement");
    CombatShotReceive(AckCommand, &ack, sizeof(ack));
    Require(CombatShotReady(), "matching server capability");
    Issued issued = {Version, hello.nonce, 100, 1, 2, 100, 100, LifetimeMs};
    CombatShotReceive(IssuedCommand, &issued, sizeof(issued) - 1);
    Require(animations == 0, "short issue cannot animate");
    issued.caster = 2;
    CombatShotReceive(IssuedCommand, &issued, sizeof(issued));
    Require(animations == 0, "wrong actor");
    issued.caster = 1;
    CombatShotReceive(IssuedCommand, &issued, sizeof(issued));
    CombatShotReceive(IssuedCommand, &issued, sizeof(issued));
    Require(animations == 1, "valid issue produces one animation");
    Require(CombatShotTake(1, 2, 500) == 0, "unrelated effect does not steal action");
    Require(CombatShotTake(2, 2, THROW_ARROW) == 0, "observer cannot steal action");
    const uint64_t action = CombatShotTake(1, 2, THROW_ARROW);
    Require(action == 100 && CombatShotTake(1, 2, THROW_ARROW) == 0,
        "bind once to the projectile, not later impact visuals");
    CombatShotSendHit(action, 3, 200, 200, true);
    Require(sent.back().h.header.type == HitCommand && sent.back().h.header.size == sizeof(Hit),
        "versioned callback without legacy fallback");
    Hit hit;
    memcpy(&hit, sent.back().u.data, sizeof(hit));
    Require(hit.action == action && hit.nonce == hello.nonce && hit.target == 3 && hit.wanted == 1,
        "action survives legitimate interception");
    BurstIssued burst = {};
    burst.version = Version; burst.nonce = hello.nonce;
    burst.caster = 1; burst.target = 2; burst.x = burst.y = 100;
    burst.skill = MultiFireSkill; burst.level = 1; burst.count = 3;
    burst.lifetime_ms = LifetimeMs;
    for (unsigned part = 0; part < burst.count; ++part) burst.actions[part] = 200 + part;
    const unsigned beforeInvalid = animations;
    CombatShotReceive(BurstIssuedCommand, &burst, sizeof(burst) - 1);
    auto invalid = burst; invalid.caster = 2;
    CombatShotReceive(BurstIssuedCommand, &invalid, sizeof(invalid));
    invalid = burst; invalid.count = 5;
    CombatShotReceive(BurstIssuedCommand, &invalid, sizeof(invalid));
    invalid = burst; invalid.level = 0;
    CombatShotReceive(BurstIssuedCommand, &invalid, sizeof(invalid));
    invalid = burst; invalid.actions[1] = invalid.actions[0];
    CombatShotReceive(BurstIssuedCommand, &invalid, sizeof(invalid));
    invalid = burst; invalid.actions[7] = 999;
    CombatShotReceive(BurstIssuedCommand, &invalid, sizeof(invalid));
    Require(animations == beforeInvalid, "malformed bursts never start attack animations");
    CombatShotReceive(BurstIssuedCommand, &burst, sizeof(burst));
    CombatShotReceive(BurstIssuedCommand, &burst, sizeof(burst));
    Require(animations == beforeInvalid + 1 && Hero->nActiveLevel == 1 &&
        Hero->excute_MagicNum == MultiFireSkill, "one animation with server skill and level");
    Require(CombatShotTake(1, 2, THROW_ARROW) == 0 &&
        CombatShotTakeBurst(2, 2, MultiFireSkill) == 0 &&
        CombatShotTakeBurst(1, 3, MultiFireSkill) == 0 &&
        CombatShotTakeBurst(1, 2, IcingBlastSkill) == 0, "unrelated generators cannot steal burst");
    const uint64_t batch = CombatShotTakeBurst(1, 2, MultiFireSkill);
    Require(batch == 200 && CombatShotTakeBurst(1, 2, MultiFireSkill) == 0,
        "receive handler restores pending state after original animation hook clears it");
    Hero->nActiveLevel = 3; Hero->nActiveCombat = IcingBlastSkill;
    Require(CombatShotBurstCount(batch, MultiFireSkill) == 3 &&
        CombatShotBurstSkill(batch) == MultiFireSkill, "released burst keeps server snapshot");
    // A new ordinary animation may clear pending state, but cannot erase a released root.
    issued.action = 250;
    CombatShotReceive(IssuedCommand, &issued, sizeof(issued));
    for (unsigned part = 0; part < 3; ++part)
    {
        const uint64_t child = CombatShotTakePart(batch, part, 3);
        Require(child == 200 + part && CombatShotTakePart(batch, part, 3) == 0,
            "each child is bound only once across later animations");
        CombatShotSendHit(child, 2, 100, 100, true);
        memcpy(&hit, sent.back().u.data, sizeof(hit));
        Require(hit.action == child && hit.nonce == hello.nonce, "child ID reaches real hit packet");
    }
    Require(CombatShotBurstCount(batch, MultiFireSkill) == 0, "fully bound batch is retired");
    uint64_t nextAction = 300;
    for (unsigned skill : {unsigned(MultiFireSkill), unsigned(IcingBlastSkill)})
        for (unsigned level = 1; level <= 3; ++level)
        {
            burst.skill = uint16_t(skill); burst.level = uint8_t(level);
            burst.count = uint8_t(BurstParts(skill, level));
            for (unsigned part = 0; part < MaxBurstParts; ++part)
                burst.actions[part] = part < burst.count ? nextAction + part : 0;
            const unsigned previousAnimations = animations;
            CombatShotReceive(BurstIssuedCommand, &burst, sizeof(burst));
            Require(animations == previousAnimations + 1 && Hero->nActiveLevel == level,
                "both skills accept every valid server level");
            Require(CombatShotTakeBurst(1, 2, skill) == nextAction, "matrix root binding");
            MAGICLIST root = {};
            root.lpChar_Own = Hero; root.lpChar_Target = Hero;
            root.magicSerial = int(skill); root.next_Effect = skill == MultiFireSkill ? 184 : 189;
            root.shot_batch = nextAction;
            Hero->excute_MagicNum = 999; Hero->nActiveLevel = 0;
            generated.clear();
            CBattle generator;
            if (skill == MultiFireSkill) generator.EffectShoot(&root, burst.count);
            else generator.EffectMulti(&root, burst.count);
            Require(generated.size() == burst.count, "real generators create server-authorized count");
            for (unsigned part = 0; part < burst.count; ++part)
                Require(generated[part].shot_action == nextAction + part &&
                    generated[part].shot_batch == 0 && generated[part].magicSerial == skill,
                    "real AddEffect preserves child ID and snapshot after skill changes");
            generator.EffectShoot(&root, burst.count);
            generator.EffectMulti(&root, burst.count);
            Require(generated.size() == burst.count, "repeated generators cannot create legacy children");
            nextAction += MaxBurstParts;
        }
    burst.skill = MultiFireSkill; burst.level = 1; burst.count = 3;
    for (unsigned part = 0; part < MaxBurstParts; ++part)
        burst.actions[part] = part < 3 ? nextAction + part : 0;
    CombatShotReceive(BurstIssuedCommand, &burst, sizeof(burst));
    Require(CombatShotTakeBurst(1, 2, MultiFireSkill) == nextAction, "expiring root released");
    g_ClientTime += LifetimeMs;
    Require(!CombatShotBurstCount(nextAction, MultiFireSkill) &&
        !CombatShotTakePart(nextAction, 0, 3), "expired root cannot create legacy children");
    MAGICLIST expired = {};
    expired.lpChar_Own = Hero; expired.lpChar_Target = Hero;
    expired.shot_batch = nextAction; expired.next_Effect = 184;
    generated.clear();
    CBattle generator;
    generator.EffectShoot(&expired, 3); generator.EffectMulti(&expired, 3);
    Require(generated.empty(), "expired production generators cannot fall back to action zero");
    expired.shot_batch = 0;
    generator.EffectShoot(&expired, 3); generator.EffectMulti(&expired, 5);
    Require(generated.size() == 8, "observer visual generators retain legacy counts");
    // Saturation must reject before animation so repeated packets cannot restart it.
    for (unsigned index = 0; index < MaxOutstanding; ++index)
    {
        nextAction += MaxBurstParts;
        for (unsigned part = 0; part < 3; ++part) burst.actions[part] = nextAction + part;
        CombatShotReceive(BurstIssuedCommand, &burst, sizeof(burst));
    }
    nextAction += MaxBurstParts;
    for (unsigned part = 0; part < 3; ++part) burst.actions[part] = nextAction + part;
    const unsigned fullAnimations = animations;
    CombatShotReceive(BurstIssuedCommand, &burst, sizeof(burst));
    CombatShotReceive(BurstIssuedCommand, &burst, sizeof(burst));
    Require(animations == fullAnimations, "full batch table refuses animation before mutation");
    g_ClientTime += LifetimeMs;
    CombatShotReceive(BurstIssuedCommand, &burst, sizeof(burst));
    Require(animations == fullAnimations + 1, "expired batch slots can be reused");
    const size_t before = sent.size();
    CombatShotReset();
    Require(!CombatShotBurstSkill(nextAction) && !CombatShotTakePart(nextAction, 0, 3),
        "map reset cancels all burst roots and children");
    CombatShotSendHit(action, 3, 200, 200, false);
    Require(sent.size() == before, "disconnected callback cannot send");
    CombatShotBegin();
    CombatShotReceive(AckCommand, &ack, sizeof(ack));
    Require(!CombatShotReady(), "old map acknowledgement cannot negotiate new map");
    const size_t initial = sent.size();
    for (unsigned second = 0; second < 10; ++second)
    { g_ClientTime += 1000; CombatShotTick(); }
    Require(sent.size() == initial + 4 && warnings == 1, "bounded handshake retries and mismatch notice");
    g_ClientTime += 30000;
    CombatShotTick();
    Require(sent.size() == initial + 5 && warnings == 1, "slow entry retries at a low rate without repeated notices");
    memcpy(&hello, sent.back().u.data, sizeof(hello));
    ack = {Version, hello.nonce};
    CombatShotReceive(AckCommand, &ack, sizeof(ack));
    Require(CombatShotReady(), "late server readiness recovers without relogin");
    g_ClientTime += 30000;
    CombatShotTick();
    Require(sent.size() == initial + 5, "ready session stops negotiation retries");
    std::cout << "Production combat shot client tests passed\n";
}
