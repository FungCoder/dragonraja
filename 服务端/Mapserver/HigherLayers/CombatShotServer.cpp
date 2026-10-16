#include "..\stdafx.h"
#include "CombatShotServer.h"
#include "BattleManager.h"
#include "Area.h"
#include "PlayerSessionSafety.h"
#include "../../Library/Shared/CombatShotLedger.h"
#include <map>
#include <mutex>
#include <cstddef>

static_assert(MULTIPLE_FIRE == DragonCombatShot::MultiFireSkill, "Multi-fire canonical ID");
static_assert(ICING_BLAST == DragonCombatShot::IcingBlastSkill, "Icing-blast canonical ID");
static_assert(FLEET_MISSILE == DragonCombatShot::FleetMissileSkill, "Fleet-missile canonical ID");
static_assert(CONVERTING_ARMOR == DragonCombatShot::ConvertingArmorSkill, "Converting-armor canonical ID");
static_assert(SHARK_MISSILE == DragonCombatShot::SharkMissileSkill, "Shark-missile canonical ID");
static_assert(FIRE_EXTREME == DragonCombatShot::FireExtremeSkill, "Fire-extreme canonical ID");
static_assert(ICE_EXTREME == DragonCombatShot::IceExtremeSkill, "Ice-extreme canonical ID");

namespace
{
using DragonGameplay::CombatShotLedger;
using DragonGameplay::PreparationIdentity;
using DragonGameplay::IsPlayerSession;
using DragonGameplay::GetPlayerSessionIdentity;
std::map<int, CombatShotLedger> ledgers;
std::map<int, DWORD> notices, helloTimes;
std::map<int, CombatShotLedger> legacyReleases;
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
    notices.erase(connection);
    helloTimes.erase(connection);
    legacyReleases.erase(connection);
}

void CancelCombatShots(int connection)
{
    const std::lock_guard<std::mutex> lock(ledgerMutex);
    const auto found = ledgers.find(connection);
    if (found != ledgers.end()) found->second.ClearActions();
    legacyReleases.erase(connection);
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

bool UsesVersionedThrow(int connection)
{
    if (!IsPlayerSession(connection)) return false;
    const CHARLIST& actor = connections[connection].chrlst;
    return DragonCombatShot::IsSkillShot(actor.GetActiveCombat()) ||
        (actor.GetActiveCombat() == 0 && actor.GetMagic() != FIRE_ARROW && actor.GetMagic() != ICE_ARROW);
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
            action.id = nextAction + 1;
            nextAction += count;
            if (!found->second.Issue(owner, action, GetTickCount64())) return false;
        }
    }
    if (!nonce) { UpgradeMessage(connection); return false; }
    t_packet packet = {};
    if (skill == 0)
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
    if (action.attack_kind == WR_LONG1 && !actor.IsArrow()) return;
    if (hitX < 0 || hitY < 0 || hitX / TILE_SIZE >= g_lpMapFile->wWidth ||
        hitY / TILE_SIZE >= g_lpMapFile->wHeight) return;
    // Ordinary shots can intercept another entity. Bound candidates to the
    // existing server visibility rectangle around the actual release position.
    const int64_t dx = int64_t(target->X) - action.launch_x;
    const int64_t dy = int64_t(target->Y) - action.launch_y;
    if (dx > REM_AREA_X || dx < -REM_AREA_X || dy > REM_AREA_Y || dy < -REM_AREA_Y) return;
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

bool RegisterLegacyThrowRelease(int connection)
{
    if (!IsPlayerSession(connection)) return false;
    const CHARLIST& actor = connections[connection].chrlst;
    // The remaining unversioned physical callback is the magic-arrow fallback.
    if (actor.GetActiveCombat() != 0 ||
        (actor.GetMagic() != FIRE_ARROW && actor.GetMagic() != ICE_ARROW)) return false;
    DragonGameplay::ShotAction action = MakeShotAction(actor, 1);
    const std::lock_guard<std::mutex> lock(ledgerMutex);
    if (nextAction == UINT64_MAX) return false;
    action.id = ++nextAction;
    const PreparationIdentity owner = GetPlayerSessionIdentity(connection);
    CombatShotLedger& ledger = legacyReleases[connection];
    ledger.Handshake(owner, 1);
    return ledger.Issue(owner, action, GetTickCount64());
}

bool AllowLegacyThrowResult(int connection)
{
    if (!IsPlayerSession(connection)) return false;
    const std::lock_guard<std::mutex> lock(ledgerMutex);
    const auto found = legacyReleases.find(connection);
    return found != legacyReleases.end() &&
        found->second.OldestPending(GetPlayerSessionIdentity(connection), GetTickCount64()) != 0;
}

void RecvLegacyCombatShotHit(int connection, const void* data, int size)
{
    if (!IsPlayerSession(connection) || !data || size != sizeof(t_client_throw_result)) return;
    if (static_cast<const unsigned char*>(data)[offsetof(t_client_throw_result, bWanted)] > 1) return;
    t_client_throw_result hit = {};
    memcpy(&hit, data, sizeof(hit));
    const PreparationIdentity owner = GetPlayerSessionIdentity(connection);
    DragonGameplay::ShotAction action;
    {
        const std::lock_guard<std::mutex> lock(ledgerMutex);
        const auto found = legacyReleases.find(connection);
        if (found == legacyReleases.end() ||
            !found->second.ConsumeOldest(owner, GetTickCount64(), action)) return;
    }
    SettleCombatShot(connection, owner, action, hit.idTarget, hit.nX, hit.nY, hit.bWanted);
}
