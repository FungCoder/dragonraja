#ifndef DRAGON_MAGIC_PREPARATION_GATE_H
#define DRAGON_MAGIC_PREPARATION_GATE_H

#include <stdint.h>

namespace DragonGameplay
{
struct PreparationIdentity
{
    uint32_t agent;
    uint32_t user;
    uint32_t character;
    uint32_t map;

    bool operator==(const PreparationIdentity& other) const
    {
        return agent == other.agent && user == other.user &&
            character == other.character && map == other.map;
    }
};

// Uses a 64-bit monotonic clock; a 32-bit timer wrap cannot revive an old cast.
// The action handler still enforces the original finite validity window.
class MagicPreparationGate
{
public:
    MagicPreparationGate() : started_ms_(0), duration_ms_(0), skill_(0), valid_(false) {}

    void Reset() { valid_ = false; }

    void Begin(const PreparationIdentity& identity, uint16_t skill,
        uint64_t now_ms, uint32_t duration_ms)
    {
        identity_ = identity;
        skill_ = skill;
        started_ms_ = now_ms;
        duration_ms_ = duration_ms;
        valid_ = true;
    }

    bool CanExecute(const PreparationIdentity& identity, uint16_t skill,
        uint64_t now_ms) const
    {
        return valid_ && identity_ == identity && skill_ == skill &&
            now_ms >= started_ms_ && now_ms - started_ms_ >= duration_ms_;
    }
    bool CanExecuteWithin(const PreparationIdentity& identity, uint16_t skill,
        uint64_t now_ms, uint64_t active_ms) const
    {
        return CanExecute(identity, skill, now_ms) &&
            now_ms - started_ms_ - duration_ms_ < active_ms;
    }

private:
    PreparationIdentity identity_;
    uint64_t started_ms_;
    uint32_t duration_ms_;
    uint16_t skill_;
    bool valid_;
};
}

#endif
