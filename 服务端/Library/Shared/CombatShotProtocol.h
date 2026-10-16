#pragma once
#include <stdint.h>
#include <cstring>
#include <type_traits>

namespace DragonCombatShot
{
enum { Version = 4, HelloCommand = 31410, AckCommand = 31411,
    IssuedCommand = 31412, HitCommand = 31413, BurstIssuedCommand = 31414,
    ArrowIssuedCommand = 31415 };
// New protocol policy, not a claimed timeout from the reference client.
enum { LifetimeMs = 15000, MaxOutstanding = 64, MaxBurstParts = 8,
    MultiFireSkill = 228, FleetMissileSkill = 229, ConvertingArmorSkill = 230,
    FireExtremeSkill = 232, IcingBlastSkill = 239, SharkMissileSkill = 241,
    IceExtremeSkill = 244, FireArrowMagic = 29, IceArrowMagic = 38 };
inline bool IsMagicArrow(unsigned magic)
{
    return magic == FireArrowMagic || magic == IceArrowMagic;
}
inline bool IsBurstSkill(unsigned skill)
{
    return skill == MultiFireSkill || skill == IcingBlastSkill ||
        skill == FireExtremeSkill || skill == IceExtremeSkill;
}
inline bool IsSkillShot(unsigned skill)
{
    return IsBurstSkill(skill) || skill == FleetMissileSkill ||
        skill == ConvertingArmorSkill || skill == SharkMissileSkill;
}
inline unsigned BurstParts(unsigned skill, unsigned level)
{
    // Skill releases share a batch envelope, including single projectiles.
    if (!IsSkillShot(skill) || level < 1 || level > 3) return 0;
    if (skill == FireExtremeSkill || skill == IceExtremeSkill) return 8;
    if (!IsBurstSkill(skill)) return 1;
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
struct ArrowIssued
{
    Issued shot;
    uint16_t magic;
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
static_assert(sizeof(ArrowIssued) == 28, "Magic arrow issue layout");

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
