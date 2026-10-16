#ifndef DRAGON_AUTOPLAY_PROTOCOL_H
#define DRAGON_AUTOPLAY_PROTOCOL_H

#include <stdint.h>

namespace DragonAutoPlay
{
enum { Version = 1, RequestCommand = 31400, ResponseCommand = 31401 };
enum Kind { Attack = 0, Magic = 1, Combat = 2, Work = 3 };
enum Reason
{
    Ready = 0, InvalidRequest, Unsupported, NotLearned, Dead, Disabled,
    War, InvalidTarget, OutOfRange, Cooldown, InsufficientResource,
    AbilityRequired, EquipmentRequired
};
enum KnownFlags
{
    RulesKnown = 1, WaitKnown = 2, PrepareKnown = 4, RangeKnown = 8,
    ManaCostKnown = 16, EffectKnown = 32, EffectActive = 64,
    LearnedKnown = 128
};

#pragma pack(push, 1)
struct Request
{
    uint16_t version;
    uint8_t kind;
    uint16_t skill;
    uint16_t target;
    uint16_t sequence;
};

// Advisory snapshot only: existing action handlers remain authoritative.
// Missing known bits mean unknown, never zero cost/cooldown or missing effect.
// Expired-effect scheduling requires EffectKnown; EffectActive is meaningful
// only with EffectKnown. wait_ms gates a NEW action, not its execute phase.
// RulesKnown describes supplied table metadata and basic eligibility, not a
// reservation or permission to skip the original action's final checks.
struct Response
{
    uint16_t version;
    uint8_t kind;
    uint16_t skill;
    uint16_t target;
    uint16_t sequence;
    uint8_t allowed;
    uint8_t reason;
    uint16_t known;
    uint32_t wait_ms;
    uint32_t prepare_ms;
    uint32_t effect_remaining_ms;
    uint16_t range_pixels;
    uint16_t mana_cost;
    uint8_t skill_type;
    uint8_t target_type;
    uint8_t execute_type;
};
#pragma pack(pop)

static_assert(sizeof(Request) == 9, "AutoPlay request layout changed");
static_assert(sizeof(Response) == 32, "AutoPlay response layout changed");
}

#endif
