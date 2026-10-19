#pragma once
#include "MagicPreparationGate.h"
#include <array>

namespace DragonGameplay
{
enum { FireWallMagic = 34, IceWallMagic = 42 };
inline bool IsWallMagic(unsigned magic)
{
    return magic == FireWallMagic || magic == IceWallMagic;
}

// Proof of a successful paid execute, separate from the legacy selection clock.
// The original CheckEnd still decides the wall's accepted lifetime and effects.
class WallExecutionGate
{
public:
    bool Record(const PreparationIdentity& identity, unsigned magic,
        uint64_t now_ms, int duration_seconds)
    {
        if (!IsWallMagic(magic) || duration_seconds < 0) return false;
        const unsigned slot = magic == FireWallMagic ? 0 : 1;
        // CheckEnd uses whole seconds and accepts its exact deadline second.
        // Include that quantization margin without extending CheckEnd itself.
        windows_[slot] = uint64_t(duration_seconds) * 1000 + 1000;
        gates_[slot].Begin(identity, uint16_t(magic), now_ms, 0);
        return true;
    }
    bool CanReport(const PreparationIdentity& identity, unsigned magic,
        uint64_t now_ms) const
    {
        if (!IsWallMagic(magic)) return false;
        const unsigned slot = magic == FireWallMagic ? 0 : 1;
        return gates_[slot].CanExecuteWithin(identity, uint16_t(magic),
            now_ms, windows_[slot]);
    }
private:
    std::array<MagicPreparationGate, 2> gates_;
    std::array<uint64_t, 2> windows_ = {};
};
}
