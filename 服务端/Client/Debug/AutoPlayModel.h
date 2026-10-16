#pragma once
#include "../../Library/Shared/AutoPlayProtocol.h"
#include <stdint.h>

namespace DragonAutoPlayClient
{
inline uint32_t RemainingMs(uint32_t now, uint32_t deadline)
{
    const int32_t remaining = static_cast<int32_t>(deadline - now);
    return remaining > 0 ? static_cast<uint32_t>(remaining) : 0;
}

struct QueryTicket
{
    DragonAutoPlay::Request request = {};
    uint32_t sentAt = 0;
    bool pending = false;

    void Begin(const DragonAutoPlay::Request& value, uint32_t now)
    {
        request = value;
        sentAt = now;
        pending = true;
    }
    bool Accept(const DragonAutoPlay::Response& response)
    {
        if (!pending || response.version != DragonAutoPlay::Version ||
            response.sequence != request.sequence || response.kind != request.kind ||
            response.skill != request.skill || response.target != request.target)
            return false;
        pending = false;
        return true;
    }
    bool Expired(uint32_t now) const
    {
        return pending && static_cast<uint32_t>(now - sentAt) >= 5000;
    }
};

inline bool IsIntervalValid(int seconds)
{
    return seconds >= 0 && seconds <= 86400;
}

inline uint32_t ConfiguredIntervalMs(int seconds)
{
    return seconds == 0 ? 250u : static_cast<uint32_t>(seconds) * 1000u;
}
}
