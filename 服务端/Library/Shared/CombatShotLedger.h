#pragma once
#include "CombatShotProtocol.h"
#include "MagicPreparationGate.h"
#include <array>

namespace DragonGameplay
{
struct ShotAction
{
    uint64_t id = 0;
    uint64_t created_ms = 0;
    int launch_x = 0;
    int launch_y = 0;
    int attack_kind = 0;
    uint16_t parts = 1;
    uint16_t used_parts = 0;
};

// One ledger per authenticated player session. Failed hits also spend an action.
// No skill switch operation touches this ledger: released shots remain in flight.
class CombatShotLedger
{
public:
    void Reset()
    {
        initialized_ = false;
        nonce_ = 0;
        ClearActions();
    }
    void ClearActions()
    {
        for (ShotAction& action : actions_) action.id = 0;
    }
    bool Handshake(const PreparationIdentity& identity, uint32_t nonce)
    {
        if (!nonce) return false;
        // A retry must not erase legitimate in-flight projectiles.
        if (Matches(identity, nonce)) return true;
        Reset();
        identity_ = identity;
        nonce_ = nonce;
        initialized_ = true;
        return true;
    }
    bool Matches(const PreparationIdentity& identity, uint32_t nonce) const
    {
        return initialized_ && identity_ == identity && nonce_ == nonce;
    }
    uint32_t Nonce(const PreparationIdentity& identity) const
    {
        return initialized_ && identity_ == identity ? nonce_ : 0;
    }
    bool Issue(const PreparationIdentity& identity, const ShotAction& action,
        uint64_t now)
    {
        if (!Nonce(identity) || !action.id || !action.parts ||
            action.parts > DragonCombatShot::MaxBurstParts ||
            action.id > UINT64_MAX - (action.parts - 1)) return false;
        for (ShotAction& current : actions_)
            if (current.id && action.id <= current.id + current.parts - 1 &&
                current.id <= action.id + action.parts - 1) return false;
        for (ShotAction& current : actions_)
        {
            if (!current.id || Expired(current, now))
            {
                current = action;
                current.created_ms = now;
                current.used_parts = 0;
                return true;
            }
        }
        return false;
    }
    bool Consume(const PreparationIdentity& identity, uint32_t nonce,
        uint64_t id, uint64_t now, ShotAction& result)
    {
        if (!Matches(identity, nonce) || !id) return false;
        for (ShotAction& action : actions_)
        {
            if (!action.id || id < action.id || id - action.id >= action.parts) continue;
            if (Expired(action, now)) { action.id = 0; return false; }
            const uint16_t mask = uint16_t(1u << unsigned(id - action.id));
            if (action.used_parts & mask) return false;
            action.used_parts |= mask;
            result = action;
            result.id = id;
            if (action.used_parts == (1u << action.parts) - 1) action.id = 0;
            return true;
        }
        return false;
    }
    uint64_t OldestPending(const PreparationIdentity& identity, uint64_t now) const
    {
        if (!Nonce(identity)) return 0;
        uint64_t oldest = 0;
        for (const ShotAction& action : actions_)
        {
            if (!action.id || Expired(action, now)) continue;
            for (unsigned part = 0; part < action.parts; ++part)
                if (!(action.used_parts & (1u << part)))
                {
                    const uint64_t candidate = action.id + part;
                    if (!oldest || candidate < oldest) oldest = candidate;
                    break;
                }
        }
        return oldest;
    }
    // Used only for explicitly retained legacy callbacks without a wire ID.
    bool ConsumeOldest(const PreparationIdentity& identity, uint64_t now, ShotAction& result)
    {
        return Consume(identity, Nonce(identity), OldestPending(identity, now), now, result);
    }
private:
    static bool Expired(const ShotAction& action, uint64_t now)
    {
        return now < action.created_ms ||
            now - action.created_ms >= DragonCombatShot::LifetimeMs;
    }
    PreparationIdentity identity_ = {};
    uint32_t nonce_ = 0;
    bool initialized_ = false;
    std::array<ShotAction, DragonCombatShot::MaxOutstanding> actions_ = {};
};
}
