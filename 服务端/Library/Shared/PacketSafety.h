#pragma once
#include <cstddef>

namespace DragonPacketSafety
{
inline bool IsPayloadSizeValid(int payloadSize, std::size_t capacity)
{
    return payloadSize >= 0 && static_cast<std::size_t>(payloadSize) <= capacity;
}

inline bool IsFrameSizeValid(std::size_t frameSize, std::size_t headerSize,
    std::size_t payloadCapacity)
{
    return frameSize >= headerSize && frameSize - headerSize <= payloadCapacity;
}

inline bool IsCompleteFrameValid(std::size_t frameSize, int payloadSize,
    std::size_t headerSize, std::size_t payloadCapacity)
{
    return IsPayloadSizeValid(payloadSize, payloadCapacity) &&
        IsFrameSizeValid(frameSize, headerSize, payloadCapacity) &&
        frameSize - headerSize == static_cast<std::size_t>(payloadSize);
}

inline bool IsReadWindowValid(int remaining, int offset, std::size_t capacity)
{
    return remaining > 0 && offset >= 0 &&
        static_cast<std::size_t>(offset) <= capacity &&
        static_cast<std::size_t>(remaining) <= capacity - offset;
}
}
