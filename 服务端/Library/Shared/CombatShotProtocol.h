#pragma once
#include <stdint.h>
#include <cstring>
#include <type_traits>

namespace DragonCombatShot
{
enum { Version = 2, HelloCommand = 31410, AckCommand = 31411,
    IssuedCommand = 31412, HitCommand = 31413, BurstIssuedCommand = 31414 };
// New protocol policy, not a claimed timeout from the reference client.
enum { LifetimeMs = 15000, MaxOutstanding = 64, MaxBurstParts = 8,
    MultiFireSkill = 228, IcingBlastSkill = 239 };
inline bool IsBurstSkill(unsigned skill)
{
    return skill == MultiFireSkill || skill == IcingBlastSkill;
}
inline unsigned BurstParts(unsigned skill, unsigned level)
{
    if (!IsBurstSkill(skill) || level < 1 || level > 3) return 0;
    return level == 3 ? 5 : 3;
}
#pragma pack(push, 1)
struct Hello { uint16_t version; uint32_t nonce; };
struct Ack { uint16_t version; uint32_t nonce; };
struct Issued
{
    uint16_t version;
    uint32_t nonce;
    uint64_t action;
    uint16_t caster;
    uint16_t target;
    int16_t x;
    int16_t y;
    uint32_t lifetime_ms;
};
struct Hit
{
    uint16_t version;
    uint32_t nonce;
    uint64_t action;
    uint16_t target;
    int16_t x;
    int16_t y;
    uint8_t wanted;
};
struct BurstIssued
{
    uint16_t version;
    uint32_t nonce;
    uint16_t caster;
    uint16_t target;
    int16_t x;
    int16_t y;
    uint16_t skill;
    uint8_t level;
    uint8_t count;
    uint64_t actions[MaxBurstParts];
    uint32_t lifetime_ms;
};
#pragma pack(pop)
static_assert(sizeof(Hello) == 6 && sizeof(Ack) == 6, "Shot handshake layout");
static_assert(sizeof(Issued) == 26 && sizeof(Hit) == 21, "Shot action layout");
static_assert(sizeof(BurstIssued) == 86, "Burst issue layout");

template<class Packet, class Payload>
void WritePacket(Packet& packet, uint16_t command, const Payload& payload)
{
    static_assert(std::is_trivially_copyable<Payload>::value, "Wire payload must be a value type");
    static_assert(sizeof(payload) <= sizeof(packet.u.data), "Shot payload exceeds packet capacity");
    packet.h.header.type = command;
    packet.h.header.size = sizeof(payload);
    std::memcpy(packet.u.data, &payload, sizeof(payload));
}
}
