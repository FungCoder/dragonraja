#include "stdafx.h"
#include "CombatShotClient.h"
#include "CombatShotClientState.h"
#include "../../Library/Shared/CombatShotProtocol.h"
#include "Dragon.h"
#include "Char.h"
#include "CharDataTable.h"
#include "BattleMgr.h"
#include "Battle.h"
#include "Effect.h"
#include "network.h"
using namespace battle;
static_assert(MULTIPLE_FIRE == DragonCombatShot::MultiFireSkill, "Multi-fire canonical ID");
static_assert(ICING_BLAST == DragonCombatShot::IcingBlastSkill, "Icing-blast canonical ID");
static_assert(FLEET_MISSILE == DragonCombatShot::FleetMissileSkill, "Fleet-missile canonical ID");
static_assert(CONVERTING_ARMOR == DragonCombatShot::ConvertingArmorSkill, "Converting-armor canonical ID");
static_assert(SHARK_MISSILE == DragonCombatShot::SharkMissileSkill, "Shark-missile canonical ID");
static_assert(FIRE_EXTREME == DragonCombatShot::FireExtremeSkill, "Fire-extreme canonical ID");
static_assert(ICE_EXTREME == DragonCombatShot::IceExtremeSkill, "Ice-extreme canonical ID");
static_assert(FIRE_ARROW == DragonCombatShot::FireArrowMagic &&
    ICE_ARROW == DragonCombatShot::IceArrowMagic, "Magic-arrow canonical IDs");
static_assert(FLYING_SPARK == DragonCombatShot::FlyingSparkMagic &&
    BLITZ_SPEAR == DragonCombatShot::BlitzSpearMagic, "Multi-projectile canonical IDs");

namespace
{
uint32_t nonceSeed = 0;
uint32_t helloAt = 0;
unsigned helloAttempts = 0;
bool helloNoticeShown = false;
CombatShotClientState state;
void SendHello()
{
    using namespace DragonCombatShot;
    const Hello hello = {Version, state.Nonce()};
    t_packet packet = {};
    DragonCombatShot::WritePacket(packet, HelloCommand, hello);
    QueuePacket(&packet, 1);
    helloAt = g_ClientTime;
    if (helloAttempts < 5) ++helloAttempts;
}
}
void CombatShotForgetPending()
{
    state.ForgetPending();
}
void CombatShotReset()
{
    state.Begin(0);
    helloAttempts = 0;
    helloNoticeShown = false;
}
void CombatShotBegin()
{
    CombatShotReset();
    if (!Hero) return;
    if (++nonceSeed == 0) ++nonceSeed;
    state.Begin(nonceSeed);
    SendHello();
}
void CombatShotTick()
{
    if (!Hero || !state.Nonce() || state.Ready() || IsDead(Hero) ||
        uint32_t(g_ClientTime - helloAt) < 1000) return;
    if (helloAttempts < 5) SendHello();
    else
    {
        if (!helloNoticeShown)
        {
            helloNoticeShown = true;
            g_mgrBattle.Message(MK_WARNING, "Secure ranged combat is unavailable; update client/server together.");
        }
        // Slow map entry must be able to recover without a fresh login.
        if (uint32_t(g_ClientTime - helloAt) >= 30000) SendHello();
    }
}
bool CombatShotReady()
{
    return state.Ready();
}
bool CombatShotNeedsHandshake(int activeCombat, int bowType)
{
    (void)bowType;
    return DragonCombatShot::IsSkillShot(activeCombat) || activeCombat == 0;
}
void CombatShotReceive(unsigned command, const void* data, size_t size)
{
    using namespace DragonCombatShot;
    if (!data || !Hero || !state.Nonce()) return;
    if (command == AckCommand && size == sizeof(Ack))
    {
        Ack ack = {};
        memcpy(&ack, data, sizeof(ack));
        state.Accept(ack);
        return;
    }
    if (command == MagicIssuedCommand && size == sizeof(ArrowIssued))
    {
        ArrowIssued cast = {};
        memcpy(&cast, data, sizeof(cast));
        if (!IsSingleCastMagic(cast.magic) || !state.CanAccept(cast.shot, Hero->id)) return;
        state.Record(cast.shot, g_ClientTime, cast.magic);
        state.ForgetPending();
        t_server_magic_execute packet = {};
        packet.nMagic = BYTE(cast.magic); packet.idCaster = cast.shot.caster;
        packet.idTarget = cast.shot.target; packet.nX = cast.shot.x; packet.nY = cast.shot.y;
        g_mgrBattle.RecvMagicExecuteWithAction(&packet, cast.shot.action);
        return;
    }
    if (command == MagicBurstIssuedCommand && size == sizeof(BurstIssued))
    {
        BurstIssued cast = {};
        memcpy(&cast, data, sizeof(cast));
        if (!state.RecordBurst(cast, Hero->id, g_ClientTime, true)) return;
        state.ForgetPending();
        t_server_magic_execute packet = {};
        packet.nMagic = BYTE(cast.skill); packet.idCaster = cast.caster;
        packet.idTarget = cast.target; packet.nX = cast.x; packet.nY = cast.y;
        g_mgrBattle.RecvMagicExecuteWithAction(&packet, cast.actions[0]);
        return;
    }
    if (command == BurstIssuedCommand && size == sizeof(BurstIssued))
    {
        BurstIssued burst = {};
        memcpy(&burst, data, sizeof(burst));
        if (!state.CanAcceptBurst(burst, Hero->id) || !state.HasBurstCapacity(g_ClientTime)) return;
        t_server_throw_attack packet = {};
        packet.idCaster = burst.caster; packet.idTarget = burst.target;
        packet.nX = burst.x; packet.nY = burst.y; packet.nCombat = BYTE(burst.skill);
        // Shark Missile chooses its animation duration from this level.
        Hero->nActiveLevel = burst.level;
        g_mgrBattle.RecvThrowAttack(&packet);
        if (state.RecordBurst(burst, Hero->id, g_ClientTime))
        {
            Hero->excute_MagicNum = burst.skill;
        }
        return;
    }
    unsigned magic = 0;
    Issued shot = {};
    if (command == ArrowIssuedCommand && size == sizeof(ArrowIssued))
    {
        ArrowIssued arrow = {};
        memcpy(&arrow, data, sizeof(arrow));
        if (!IsMagicArrow(arrow.magic)) return;
        shot = arrow.shot; magic = arrow.magic;
    }
    else if (command == IssuedCommand && size == sizeof(Issued)) memcpy(&shot, data, sizeof(shot));
    else return;
    if (!state.CanAccept(shot, Hero->id)) return;
    t_server_throw_attack packet = {};
    packet.idCaster = shot.caster;
    packet.idTarget = shot.target;
    packet.nX = shot.x;
    packet.nY = shot.y;
    packet.nCombat = 0;
    Hero->bow_Type = magic == FireArrowMagic ? 1 : (magic == IceArrowMagic ? 2 : 0);
    g_mgrBattle.RecvThrowAttack(&packet);
    if (magic) Hero->excute_MagicNum = magic;
    state.Record(shot, g_ClientTime, magic);
}
uint64_t CombatShotTake(unsigned caster, unsigned target, int effect)
{
    if (!Hero || caster != Hero->id || !CombatShotReady()) return 0;
    if (DragonCombatShot::IsMagicArrow(effect)) return state.Take(target, g_ClientTime, unsigned(effect));
    switch (effect)
    {
    case THROW_ARROW: case THROW_KNIFE: case THROW_FIRE_ARROW: case THROW_ICE_KNIFE:
    case THROW_SNOW_BALL: case THROW_WATER_BALL: case THROW_BIG_GULP: break;
    default: return 0;
    }
    return state.Take(target, g_ClientTime);
}
uint64_t CombatShotTakeBurst(unsigned caster, unsigned target, unsigned skill)
{
    if (!Hero || caster != Hero->id || !CombatShotReady()) return 0;
    return state.TakeBurst(target, skill, g_ClientTime);
}
unsigned CombatShotBurstSkill(uint64_t batch)
{
    return Hero && CombatShotReady() ? state.BurstSkill(batch, g_ClientTime) : 0;
}
unsigned CombatShotBurstCount(uint64_t batch, unsigned skill)
{
    return Hero && CombatShotReady() ? state.BurstCount(batch, skill, g_ClientTime) : 0;
}
uint64_t CombatShotTakePart(uint64_t batch, unsigned part, unsigned count)
{
    return Hero && CombatShotReady() ? state.TakePart(batch, part, count, g_ClientTime) : 0;
}
void CombatShotSendHit(uint64_t action, unsigned target, int x, int y, bool wanted)
{
    using namespace DragonCombatShot;
    if (!CombatShotReady() || !action || target > UINT16_MAX ||
        x < 0 || y < 0 || x > INT16_MAX || y > INT16_MAX) return;
    const Hit hit = {Version, state.Nonce(), action, uint16_t(target),
        int16_t(x), int16_t(y), uint8_t(wanted ? 1 : 0)};
    t_packet packet = {};
    DragonCombatShot::WritePacket(packet, HitCommand, hit);
    QueuePacket(&packet, 1);
}
