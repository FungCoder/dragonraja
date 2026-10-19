#include "..\stdafx.h"
#include "CombatShotServer.h"
#include "BattleManager.h"
#include "Area.h"
#include "PlayerSessionSafety.h"
#include "../../Library/Shared/CombatShotLedger.h"
#include <map>
#include <mutex>

static_assert(MULTIPLE_FIRE == DragonCombatShot::MultiFireSkill, "Multi-fire canonical ID");
static_assert(ICING_BLAST == DragonCombatShot::IcingBlastSkill, "Icing-blast canonical ID");
static_assert(FLEET_MISSILE == DragonCombatShot::FleetMissileSkill, "Fleet-missile canonical ID");
static_assert(CONVERTING_ARMOR == DragonCombatShot::ConvertingArmorSkill, "Converting-armor canonical ID");
static_assert(SHARK_MISSILE == DragonCombatShot::SharkMissileSkill, "Shark-missile canonical ID");
static_assert(FIRE_EXTREME == DragonCombatShot::FireExtremeSkill, "Fire-extreme canonical ID");
static_assert(ICE_EXTREME == DragonCombatShot::IceExtremeSkill, "Ice-extreme canonical ID");
static_assert(FIRE_ARROW == DragonCombatShot::FireArrowMagic &&
    ICE_ARROW == DragonCombatShot::IceArrowMagic, "Magic-arrow canonical IDs");
static_assert(FIRE_BALL == DragonCombatShot::FireBallMagic &&
    ICE_BALL == DragonCombatShot::IceBallMagic, "Single-cast canonical IDs");
static_assert(FLYING_SPARK == DragonCombatShot::FlyingSparkMagic &&
    BLITZ_SPEAR == DragonCombatShot::BlitzSpearMagic, "Multi-projectile canonical IDs");
static_assert(DEATH_FIRE == DragonCombatShot::DeathFireMagic &&
    SHADOW_OF_EVIL == DragonCombatShot::ShadowOfEvilMagic &&
    FLAME_ARROW == DragonCombatShot::FlameArrowMagic, "Fan-out canonical IDs");

namespace
{
using DragonGameplay::CombatShotLedger;
using DragonGameplay::PreparationIdentity;
using DragonGameplay::IsPlayerSession;
using DragonGameplay::GetPlayerSessionIdentity;
std::map<int, CombatShotLedger> ledgers;
struct MagicExecutionTime { PreparationIdentity owner; uint64_t accepted_ms; };
std::map<int, MagicExecutionTime> magicExecutions;
std::map<int, DWORD> notices, helloTimes;
uint64_t nextAction = 0;
std::mutex ledgerMutex;

DragonGameplay::ShotAction MakeShotAction(const CHARLIST& actor, unsigned parts)
{
    DragonGameplay::ShotAction action;
    action.parts = uint16_t(parts);
    action.launch_x = actor.X;
    action.launch_y = actor.Y;
    action.attack_kind = actor.GetTacticsKind() == TACTICS_Archery ? WR_LONG1 :
        (actor.GetWeaponKind() == IK_THROW_EVENT ? WR_LONG3 : WR_LONG2);
    return action;
}
void UpgradeMessage(int connection)
{
    {
        const std::lock_guard<std::mutex> lock(ledgerMutex);
        const DWORD now = timeGetTime();
        const auto previous = notices.find(connection);
        if (previous != notices.end() && DWORD(now - previous->second) < 5000) return;
        notices[connection] = now;
    }
    t_packet packet = {};
    const char text[] = "Secure ranged combat requires matching client and server updates.";
    static_assert(sizeof(text) <= sizeof(packet.u.kein.default_msg), "Upgrade notice capacity");
    packet.h.header.type = CMD_SCRIPT_MESSAGE_BOX;
    packet.h.header.size = sizeof(text);
    memcpy(packet.u.kein.default_msg, text, sizeof(text));
    QueuePacket(connections, connection, &packet, 1);
}
}

void ResetCombatShotSession(int connection)
{
    const std::lock_guard<std::mutex> lock(ledgerMutex);
    ledgers.erase(connection);
    magicExecutions.erase(connection);
    notices.erase(connection);
    helloTimes.erase(connection);
}

void CancelCombatShots(int connection)
{
    const std::lock_guard<std::mutex> lock(ledgerMutex);
    const auto found = ledgers.find(connection);
    if (found != ledgers.end()) found->second.ClearActions();
}

void RecvCombatShotHello(int connection, const void* data, int size)
{
    using namespace DragonCombatShot;
    if (!IsPlayerSession(connection) || !data || size != sizeof(Hello)) return;
    Hello hello = {};
    memcpy(&hello, data, sizeof(hello));
    if (hello.version != Version || !hello.nonce) { UpgradeMessage(connection); return; }
    {
        const std::lock_guard<std::mutex> lock(ledgerMutex);
        const DWORD now = timeGetTime();
        const auto previous = helloTimes.find(connection);
        if (previous != helloTimes.end() && DWORD(now - previous->second) < 200) return;
        helloTimes[connection] = now;
        ledgers[connection].Handshake(GetPlayerSessionIdentity(connection), hello.nonce);
    }
    Ack ack = {Version, hello.nonce};
    t_packet packet = {};
    DragonCombatShot::WritePacket(packet, AckCommand, ack);
    QueuePacket(connections, connection, &packet, 1);
}

bool IssueCombatShot(int connection, unsigned short target, short x, short y)
{
    using namespace DragonCombatShot;
    if (!IsPlayerSession(connection)) return false;
    CHARLIST& actor = connections[connection].chrlst;
    const unsigned skill = actor.GetActiveCombat();
    const unsigned level = DragonCombatShot::IsSkillShot(skill) ? actor.GetCombatLevel(skill) : 0;
    const unsigned count = skill == 0 ? 1 : BurstParts(skill, level);
    if (!count) return false;
    DragonGameplay::ShotAction action = MakeShotAction(actor, count);
    const PreparationIdentity owner = GetPlayerSessionIdentity(connection);
    uint32_t nonce = 0;
    {
        const std::lock_guard<std::mutex> lock(ledgerMutex);
        const auto found = ledgers.find(connection);
        if (found != ledgers.end()) nonce = found->second.Nonce(owner);
        if (nonce)
        {
            // Never wrap/reuse an action ID during this server process.
            if (nextAction > UINT64_MAX - count) return false;
            if (!found->second.HasCapacity(owner, GetTickCount64())) return false;
            // Capacity and payment are serialized with issue/consume. No mana is
            // spent for a full ledger, and no unpaid action becomes visible.
            if (skill == 0 && !g_pBattleManager->PrepareMagicArrowShot(
                connection, target, x, y, action.magic)) return false;
            action.id = nextAction + 1;
            nextAction += count;
            if (!found->second.Issue(owner, action, GetTickCount64())) return false;
        }
    }
    if (!nonce) { UpgradeMessage(connection); return false; }
    t_packet packet = {};
    if (action.magic)
    {
        const ArrowIssued issued = {{Version, nonce, action.id,
            uint16_t(connection), target, x, y, LifetimeMs}, action.magic};
        DragonCombatShot::WritePacket(packet, ArrowIssuedCommand, issued);
    }
    else if (skill == 0)
    {
        const Issued issued = {Version, nonce, action.id,
            uint16_t(connection), target, x, y, LifetimeMs};
        DragonCombatShot::WritePacket(packet, IssuedCommand, issued);
    }
    else
    {
        BurstIssued issued = {};
        issued.version = Version; issued.nonce = nonce;
        issued.caster = uint16_t(connection); issued.target = target;
        issued.x = x; issued.y = y;
        issued.skill = uint16_t(skill); issued.level = uint8_t(level);
        issued.count = uint8_t(count); issued.lifetime_ms = LifetimeMs;
        for (unsigned part = 0; part < count; ++part) issued.actions[part] = action.id + part;
        DragonCombatShot::WritePacket(packet, BurstIssuedCommand, issued);
    }
    QueuePacket(connections, connection, &packet, 1);
    return true;
}

namespace
{
void SettleCombatShot(int connection, const PreparationIdentity& owner,
    const DragonGameplay::ShotAction& action, unsigned hitTarget, int hitX, int hitY, bool wanted)
{
    if (!IsPlayerSession(connection) || !(GetPlayerSessionIdentity(connection) == owner)) return;
    CHARLIST& actor = connections[connection].chrlst;
    CHARLIST* target = GetCharListPtr(hitTarget);
    if (actor.IsDead() || actor.bAlive != ALIVE_ || !target || target == &actor ||
        target->IsDead() || target->bAlive != ALIVE_) return;
    if ((!action.magic || DragonCombatShot::IsMagicArrow(action.magic)) &&
        action.attack_kind == WR_LONG1 && !actor.IsArrow()) return;
    if (hitX < 0 || hitY < 0 || hitX / TILE_SIZE >= g_lpMapFile->wWidth ||
        hitY / TILE_SIZE >= g_lpMapFile->wHeight) return;
    // Ordinary shots can intercept another entity. Bound candidates to the
    // existing server visibility rectangle around the actual release position.
    const int64_t dx = int64_t(target->X) - action.launch_x;
    const int64_t dy = int64_t(target->Y) - action.launch_y;
    if (dx > REM_AREA_X || dx < -REM_AREA_X || dy > REM_AREA_Y || dy < -REM_AREA_Y) return;
    if (action.magic)
    {
        t_client_magic_result result = {};
        result.nMagic = BYTE(action.magic); result.idTarget = hitTarget;
        result.nX = hitX; result.nY = hitY; result.bWanted = wanted;
        g_pBattleManager->RecvAuthorizedMagicResult(connection, &result);
        return;
    }
    t_client_throw_result result = {};
    result.idTarget = hitTarget;
    result.nX = hitX;
    result.nY = hitY;
    result.bWanted = wanted != 0;
    g_pBattleManager->RecvAuthorizedThrowResult(connection, &result, action.attack_kind);
}
}

void RecvCombatShotHit(int connection, const void* data, int size)
{
    using namespace DragonCombatShot;
    if (!IsPlayerSession(connection) || !data || size != sizeof(Hit)) return;
    Hit hit = {};
    memcpy(&hit, data, sizeof(hit));
    if (hit.version != Version || hit.wanted > 1) return;
    DragonGameplay::ShotAction action;
    const PreparationIdentity owner = GetPlayerSessionIdentity(connection);
    {
        const std::lock_guard<std::mutex> lock(ledgerMutex);
        const auto found = ledgers.find(connection);
        // Spend before any stateful legacy filters, curses, damage or durability.
        if (found == ledgers.end() || !found->second.Consume(owner, hit.nonce,
            hit.action, GetTickCount64(), action)) return;
    }
    SettleCombatShot(connection, owner, action, hit.target, hit.x, hit.y, hit.wanted != 0);
}

bool IssueProjectileMagic(int connection, const void* data, int size)
{
    using namespace DragonCombatShot;
    if (!IsPlayerSession(connection) || !data || size != sizeof(t_client_magic_execute)) return false;
    t_client_magic_execute request = {};
    memcpy(&request, data, sizeof(request));
    if (!IsProjectileCastMagic(request.nMagic)) return false;
    const unsigned count = IsSingleCastMagic(request.nMagic) ? 1 : MagicBurstParts(request.nMagic);
    const PreparationIdentity owner = GetPlayerSessionIdentity(connection);
    DragonGameplay::ShotAction action = MakeShotAction(connections[connection].chrlst, count);
    action.magic = request.nMagic;
    uint32_t nonce = 0;
    {
        const std::lock_guard<std::mutex> lock(ledgerMutex);
        const auto found = ledgers.find(connection);
        if (found != ledgers.end()) nonce = found->second.Nonce(owner);
        if (nonce)
        {
            if (nextAction > UINT64_MAX - count || !found->second.HasCapacity(owner, GetTickCount64())) return false;
            const uint64_t now = GetTickCount64();
            const auto previous = magicExecutions.find(connection);
            const uint64_t interval = uint64_t(MagicRepeatIntervalMs) *
                ((connections[connection].chrlst.GetState() & CON_FROZEN) ? 2 : 1);
            if (previous != magicExecutions.end() && previous->second.owner == owner &&
                (now < previous->second.accepted_ms || now - previous->second.accepted_ms < interval)) return false;
            if (!g_pBattleManager->PrepareProjectileMagicExecute(connection, &request)) return false;
            action.id = nextAction + 1;
            nextAction += count;
            if (!found->second.Issue(owner, action, GetTickCount64())) return false;
            magicExecutions[connection] = {owner, now};
        }
    }
    if (!nonce) { UpgradeMessage(connection); return false; }
    const ArrowIssued issued = {{Version, nonce, action.id, uint16_t(connection),
        request.idTarget, request.nX, request.nY, LifetimeMs}, request.nMagic};
    t_packet packet = {};
    if (count == 1) DragonCombatShot::WritePacket(packet, MagicIssuedCommand, issued);
    else
    {
        BurstIssued burst = {};
        burst.version = Version; burst.nonce = nonce;
        burst.caster = uint16_t(connection); burst.target = request.idTarget;
        burst.x = request.nX; burst.y = request.nY;
        burst.skill = request.nMagic; burst.count = uint8_t(count);
        burst.lifetime_ms = LifetimeMs;
        for (unsigned part = 0; part < count; ++part) burst.actions[part] = action.id + part;
        DragonCombatShot::WritePacket(packet, MagicBurstIssuedCommand, burst);
    }
    QueuePacket(connections, connection, &packet, 1);
    return true;
}
