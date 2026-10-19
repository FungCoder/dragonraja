#pragma once
#include "../../Library/Shared/CombatShotProtocol.h"
#include <map>

class CombatShotClientState
{
public:
    void Begin(uint32_t nonce)
    {
        nonce_ = nonce;
        ready_ = false;
        last_issued_ = 0;
        bursts_.clear();
        ForgetPending();
    }
    void Accept(const DragonCombatShot::Ack& ack)
    {
        if (nonce_ && ack.version == DragonCombatShot::Version && ack.nonce == nonce_)
            ready_ = true;
    }
    bool Ready() const { return ready_ && nonce_ != 0; }
    uint32_t Nonce() const { return nonce_; }
    bool CanAccept(const DragonCombatShot::Issued& shot, unsigned actor) const
    {
        return Ready() && shot.version == DragonCombatShot::Version &&
            shot.nonce == nonce_ && shot.action > last_issued_ && shot.caster == actor &&
            shot.lifetime_ms == DragonCombatShot::LifetimeMs;
    }
    void Record(const DragonCombatShot::Issued& shot, uint32_t now, unsigned magic = 0)
    {
        last_issued_ = pending_action_ = shot.action;
        target_ = shot.target;
        created_ms_ = now;
        pending_burst_ = false;
        pending_magic_ = magic;
    }
    bool CanAcceptBurst(const DragonCombatShot::BurstIssued& shot, unsigned actor, bool magicBurst = false) const
    {
        using namespace DragonCombatShot;
        if (!Ready() || shot.version != Version || shot.nonce != nonce_ || shot.caster != actor ||
            shot.count != (magicBurst ? (shot.level == 0 ? MagicBurstParts(shot.skill) : 0) : BurstParts(shot.skill, shot.level)) || !shot.count ||
            shot.lifetime_ms != LifetimeMs || shot.actions[0] <= last_issued_ ||
            shot.actions[0] > UINT64_MAX - (shot.count - 1)) return false;
        for (unsigned part = 0; part < MaxBurstParts; ++part)
            if (shot.actions[part] != (part < shot.count ? shot.actions[0] + part : 0)) return false;
        return true;
    }
    bool RecordBurst(const DragonCombatShot::BurstIssued& shot, unsigned actor, uint32_t now, bool magicBurst = false)
    {
        if (!CanAcceptBurst(shot, actor, magicBurst)) return false;
        for (auto entry = bursts_.begin(); entry != bursts_.end(); )
            if (Expired(entry->second, now)) entry = bursts_.erase(entry); else ++entry;
        if (bursts_.size() >= DragonCombatShot::MaxOutstanding) return false;
        BurstEntry entry;
        entry.shot = shot;
        entry.created_ms = now;
        bursts_[shot.actions[0]] = entry;
        last_issued_ = shot.actions[shot.count - 1];
        pending_action_ = shot.actions[0];
        target_ = shot.target;
        created_ms_ = now;
        pending_burst_ = true;
        pending_magic_ = 0;
        return true;
    }
    bool HasBurstCapacity(uint32_t now) const
    {
        unsigned live = 0;
        for (const auto& entry : bursts_)
            if (!Expired(entry.second, now)) ++live;
        return live < DragonCombatShot::MaxOutstanding;
    }
    void ForgetPending() { pending_action_ = 0; target_ = 0; pending_burst_ = false; pending_magic_ = 0; }
    uint64_t Take(unsigned target, uint32_t now, unsigned magic = 0)
    {
        if (!Ready() || !pending_action_ || pending_burst_ || target != target_ || magic != pending_magic_) return 0;
        const uint64_t action = uint32_t(now - created_ms_) < DragonCombatShot::LifetimeMs ?
            pending_action_ : 0;
        ForgetPending();
        return action;
    }
    uint64_t TakeBurst(unsigned target, unsigned skill, uint32_t now)
    {
        if (!Ready() || !pending_burst_ || !pending_action_ || target != target_) return 0;
        const auto entry = bursts_.find(pending_action_);
        if (entry == bursts_.end()) { ForgetPending(); return 0; }
        if (Expired(entry->second, now)) { bursts_.erase(entry); ForgetPending(); return 0; }
        if (entry->second.shot.skill != skill) return 0;
        const uint64_t batch = pending_action_;
        ForgetPending();
        return batch;
    }
    unsigned BurstSkill(uint64_t batch, uint32_t now) const
    {
        const auto entry = bursts_.find(batch);
        return entry != bursts_.end() && !Expired(entry->second, now) ? entry->second.shot.skill : 0;
    }
    unsigned BurstCount(uint64_t batch, unsigned skill, uint32_t now) const
    {
        const auto entry = bursts_.find(batch);
        return entry != bursts_.end() && !Expired(entry->second, now) && entry->second.shot.skill == skill ?
            entry->second.shot.count : 0;
    }
    uint64_t TakePart(uint64_t batch, unsigned part, unsigned count, uint32_t now)
    {
        const auto entry = bursts_.find(batch);
        if (entry == bursts_.end()) return 0;
        if (Expired(entry->second, now)) { bursts_.erase(entry); return 0; }
        if (entry->second.shot.count != count || part >= count) return 0;
        const unsigned mask = 1u << part;
        if (entry->second.used & mask) return 0;
        entry->second.used |= mask;
        const uint64_t action = entry->second.shot.actions[part];
        if (entry->second.used == (1u << count) - 1) bursts_.erase(entry);
        return action;
    }
private:
    struct BurstEntry
    {
        DragonCombatShot::BurstIssued shot = {};
        uint32_t created_ms = 0;
        unsigned used = 0;
    };
    static bool Expired(const BurstEntry& entry, uint32_t now)
    {
        return uint32_t(now - entry.created_ms) >= DragonCombatShot::LifetimeMs;
    }
    uint32_t nonce_ = 0, created_ms_ = 0;
    uint64_t last_issued_ = 0, pending_action_ = 0;
    unsigned target_ = 0;
    unsigned pending_magic_ = 0;
    bool ready_ = false;
    bool pending_burst_ = false;
    std::map<uint64_t, BurstEntry> bursts_;
};
