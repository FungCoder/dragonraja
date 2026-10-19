#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <iostream>
#include <cstdlib>
#include <vector>
#pragma comment(lib, "user32.lib")
#pragma pack(push, 1)
#include "../Client/Debug/Network5.h"
#pragma pack(pop)
#include "../Client/Debug/CombatShotClient.h"
#include "../Client/Debug/CombatShotClientState.h"
enum { THROW_ARROW = 194, THROW_KNIFE = 195, THROW_FIRE_ARROW = 665,
    THROW_ICE_KNIFE = 666, THROW_SNOW_BALL = 193, THROW_WATER_BALL = 661,
    THROW_BIG_GULP = 663, MK_WARNING = 1, FIRE_ARROW = 29, ICE_ARROW = 38,
    SPRITETYPE_ITEM = 50, TILE_SIZE = 32, FIRE_BALL = 30, ICE_BALL = 39 };
static struct CharacterFixture
{
    unsigned id = 1;
    bool dead = false;
    int nActiveCombat = 0, nActiveLevel = 0, excute_MagicNum = 0;
    int direction = 0, height = 0, x = 100, y = 100;
    int bow_Type = 0;
    int nAttackedType = 0, nCurrentAction = 0, shoot_PointX = 0, shoot_PointY = 0;
    int accessory[3] = {};
    struct { int height[1] = {}; } AnimationTable[1];
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
static unsigned animations = 0, warnings = 0, animationLevel = 0;
static unsigned magicAnimations = 0;
static void CreateSingleCastRoot(t_server_magic_execute*, uint64_t);
struct BattleFixture
{
    void RecvThrowAttack(t_server_throw_attack* packet)
    {
        // This hook is also present in the real battle handler.
        CombatShotForgetPending();
        Hero->nActiveCombat = packet->nCombat;
        animationLevel = Hero->nActiveLevel;
        ++animations;
    }
    void Message(int, const char*, ...) { ++warnings; }
    void RecvMagicExecuteWithAction(t_server_magic_execute* packet, uint64_t action)
    {
        ++magicAnimations;
        Hero->excute_MagicNum = packet->nMagic;
        CreateSingleCastRoot(packet, action);
    }
};
static BattleFixture g_mgrBattle;
#include "CombatShotClientProduction.inc"
#include "CombatBurstGeneratorFixture.h"
static void CreateSingleCastRoot(t_server_magic_execute* packet, uint64_t action)
{
    static CharacterFixture target;
    target.id = 2;
    LPCHARACTER pCaster = Hero;
    LPCHARACTER pTarget = packet->idTarget == Hero->id ? Hero : &target;
    const int nMagic = packet->nMagic, nX = packet->nX, nY = packet->nY;
    const bool bFail = packet->bFailed, bPattern = false;
    switch (nMagic)
    {
#include "SingleCastRootCasesProduction.inc"
    default: std::abort();
    }
}
static void LinkSingleCastRoot(MAGICLIST* pEffect)
{
    LPCHARACTER pCaster = pEffect->lpChar_Own, pTarget = pEffect->lpChar_Target;
    const int nMagic = pEffect->magicSerial;
#include "SingleCastLinkProduction.inc"
}
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
    Require(!CombatShotNeedsHandshake(227, 0) && CombatShotNeedsHandshake(0, 1) &&
        CombatShotNeedsHandshake(0, 2), "magic arrows require the same negotiated capability");
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
    for (unsigned skill : {unsigned(MultiFireSkill), unsigned(IcingBlastSkill),
        unsigned(FleetMissileSkill), unsigned(ConvertingArmorSkill), unsigned(SharkMissileSkill),
        unsigned(FireExtremeSkill), unsigned(IceExtremeSkill)})
        for (unsigned level = 1; level <= 3; ++level)
        {
            burst.skill = uint16_t(skill); burst.level = uint8_t(level);
            burst.count = uint8_t(BurstParts(skill, level));
            for (unsigned part = 0; part < MaxBurstParts; ++part)
                burst.actions[part] = part < burst.count ? nextAction + part : 0;
            const unsigned previousAnimations = animations;
            CombatShotReceive(BurstIssuedCommand, &burst, sizeof(burst));
            Require(animations == previousAnimations + 1 && Hero->nActiveLevel == level &&
                animationLevel == level && CombatShotNeedsHandshake(skill, 0),
                "all skill animations use server level before playback");
            const int effect = skill == FleetMissileSkill ? 575 :
                (skill == ConvertingArmorSkill ? 29 :
                (skill == SharkMissileSkill ? 38 :
                (skill == MultiFireSkill || skill == FireExtremeSkill ? 513 : 514)));
            g_lpET[513].next_Effect = 184; g_lpET[514].next_Effect = 189;
            CharacterFixture target; target.id = 2;
            Hero->excute_MagicNum = 999; Hero->nActiveLevel = 0;
            generated.clear();
            InsertBurstRoot(Hero, &target, skill, effect, 100, 100);
            Require(generated.size() == 1 && generated[0].magicSerial == skill,
                "actual root helper and effect context preserve server skill snapshot");
            MAGICLIST root = generated[0];
            InsertBurstRoot(Hero, &target, skill, effect, 100, 100);
            Require(generated.size() == 1, "root cannot bind twice or fall back to legacy");
            CBattle generator;
            if (burst.count > 1)
            {
                generated.clear();
                if (skill == MultiFireSkill) generator.EffectShoot(&root, burst.count);
                else if (skill == IcingBlastSkill) generator.EffectMulti(&root, burst.count);
                else generator.EffectSpread(&root);
            }
            Require(generated.size() == burst.count, "real generators create server-authorized count");
            for (unsigned part = 0; part < burst.count; ++part)
                Require(generated[part].shot_action == nextAction + part &&
                    generated[part].shot_batch == 0 && generated[part].magicSerial == skill,
                    "real AddEffect preserves child ID and snapshot after skill changes");
            if (burst.count == 8) generator.EffectSpread(&root);
            else if (burst.count > 1)
            {
                generator.EffectShoot(&root, burst.count);
                generator.EffectMulti(&root, burst.count);
            }
            else Require(!CombatShotTakePart(nextAction, 0, 1), "single root consumes its sole child");
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
    for (unsigned magic : {unsigned(FireArrowMagic), unsigned(IceArrowMagic)})
    {
        nextAction += 100;
        ArrowIssued arrow = {{Version, hello.nonce, nextAction, 1, 2, 100, 100, LifetimeMs}, uint16_t(magic)};
        const unsigned beforeArrow = animations;
        CombatShotReceive(ArrowIssuedCommand, &arrow, sizeof(arrow) - 1);
        auto badArrow = arrow; badArrow.magic = 30;
        CombatShotReceive(ArrowIssuedCommand, &badArrow, sizeof(badArrow));
        badArrow = arrow; ++badArrow.shot.nonce;
        CombatShotReceive(ArrowIssuedCommand, &badArrow, sizeof(badArrow));
        Require(animations == beforeArrow, "short, stale and non-arrow modes cannot animate");
        CombatShotReceive(ArrowIssuedCommand, &arrow, sizeof(arrow));
        CombatShotReceive(ArrowIssuedCommand, &arrow, sizeof(arrow));
        Require(animations == beforeArrow + 1 && Hero->bow_Type == (magic == FireArrowMagic ? 1 : 2),
            "server mode starts one matching arrow animation");
        Require(!CombatShotTake(1, 2, THROW_ARROW) &&
            !CombatShotTake(1, 2, magic == FireArrowMagic ? ICE_ARROW : FIRE_ARROW),
            "physical or other magic effect cannot steal paid arrow action");
        CharacterFixture target; target.id = 2;
        generated.clear(); Hero->excute_MagicNum = 999;
        CStrikeSys shooter;
        shooter.Shoot(Hero, &target);
        Require(generated.size() == 1 && generated[0].shot_action == nextAction &&
            generated[0].magicSerial == magic, "real Shoot and effect context preserve paid action and mode");
        shooter.Shoot(Hero, &target);
        Require(generated.size() == 1, "repeated magic shoot has no legacy fallback");
        CombatShotSendHit(generated[0].shot_action, 2, 100, 100, false);
        memcpy(&hit, sent.back().u.data, sizeof(hit));
        Require(hit.action == nextAction && sent.back().h.header.type == HitCommand,
            "magic impact uses the same one-action hit format");
        ++nextAction;
        issued.action = nextAction;
        CombatShotReceive(IssuedCommand, &issued, sizeof(issued));
        Require(Hero->bow_Type == 0 && !CombatShotTake(1, 2, magic), "server fallback cannot claim magic mode");
        generated.clear(); shooter.Shoot(Hero, &target);
        Require(generated.size() == 1 && generated[0].shot_action == nextAction,
            "real Shoot creates one physical fallback projectile");
        ++nextAction; arrow.shot.action = nextAction;
        CombatShotReceive(ArrowIssuedCommand, &arrow, sizeof(arrow));
        g_ClientTime += LifetimeMs; generated.clear(); shooter.Shoot(Hero, &target);
        Require(generated.empty(), "expired paid arrow cannot become an unversioned projectile");
    }
    for (unsigned magic : {unsigned(FireBallMagic), unsigned(IceBallMagic)})
    for (unsigned target : {1u, 2u})
    {
        nextAction += 1000;
        ArrowIssued cast = {{Version, hello.nonce, nextAction, 1, uint16_t(target), 120, 140, LifetimeMs}, uint16_t(magic)};
        const unsigned beforeCast = magicAnimations;
        CombatShotReceive(MagicIssuedCommand, &cast, sizeof(cast) - 1);
        auto badCast = cast; badCast.magic = FireArrowMagic;
        CombatShotReceive(MagicIssuedCommand, &badCast, sizeof(badCast));
        badCast = cast; ++badCast.shot.nonce;
        CombatShotReceive(MagicIssuedCommand, &badCast, sizeof(badCast));
        Require(magicAnimations == beforeCast, "short, wrong-family and stale cast confirmations are refused");
        g_lpET[FireBallMagic].next_Effect = 156; g_lpET[IceBallMagic].next_Effect = 186;
        generated.clear();
        CombatShotReceive(MagicIssuedCommand, &cast, sizeof(cast));
        CombatShotReceive(MagicIssuedCommand, &cast, sizeof(cast));
        Require(magicAnimations == beforeCast + 1 && generated.size() == 1,
            "one confirmation creates exactly one real factory root");
        MAGICLIST root = generated[0];
        Require(root.magicSerial == magic && root.shot_action == nextAction &&
            root.magicSubNum == (target == 1 ? 1 : 0), "target and ground-aim roots retain action and original sub-mode");
        Require(!CombatShotTake(1, target, THROW_ARROW), "cast cannot be stolen by throw animation");
        Hero->excute_MagicNum = 999;
        generated.clear(); LinkSingleCastRoot(&root);
        Require(generated.size() == 1 && generated[0].shot_action == nextAction &&
            generated[0].magicSerial == magic && generated[0].magicNum == (magic == FireBallMagic ? 156 : 186),
            "actual child link carries immutable action and type after another selection");
        CombatShotSendHit(generated[0].shot_action, 2, 120, 140, false);
        memcpy(&hit, sent.back().u.data, sizeof(hit));
        Require(hit.action == nextAction && sent.back().h.header.type == HitCommand,
            "spell projectile uses shared result format");
    }
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
