#include "../Client/Debug/CombatShotClientState.h"
#include <iostream>
#include <cstdlib>
static void Require(bool ok, const char* scenario)
{
    if (!ok) { std::cerr << scenario << '\n'; std::exit(1); }
}
int main()
{
    using namespace DragonCombatShot;
    CombatShotClientState state;
    state.Begin(7);
    state.Accept({Version, 6});
    Require(!state.Ready(), "stale handshake");
    state.Accept({Version + 1, 7});
    Require(!state.Ready(), "wrong protocol");
    state.Accept({Version, 7});
    Require(state.Ready(), "matching handshake");
    Issued shot = {Version, 7, 100, 1, 2, 10, 20, LifetimeMs};
    Require(!state.CanAccept(shot, 2), "wrong actor");
    Require(state.CanAccept(shot, 1), "issued action");
    state.Record(shot, 50);
    Require(!state.CanAccept(shot, 1), "duplicate issue");
    Require(state.Take(3, 100) == 0, "unrelated effect cannot steal pending action");
    Require(state.Take(2, 100) == 100, "bind action to the actual projectile");
    Require(state.Take(2, 101) == 0, "second visual cannot duplicate action");
    ++shot.action;
    state.Record(shot, 50);
    Require(state.Take(2, 50 + LifetimeMs) == 0, "expired pending animation");
    ++shot.action;
    state.Record(shot, UINT32_MAX - 100);
    Require(state.Take(2, 200) == shot.action, "animation across clock wrap");
    state.Record(shot, 10);
    state.ForgetPending();
    Require(state.Take(2, 11) == 0, "legacy animation replaces pending issue");
    state.Begin(8);
    Require(!state.CanAccept(shot, 1) && !state.Ready(), "map/session reset");
    state.Accept({Version, 7});
    Require(!state.Ready(), "old reply cannot negotiate new session");
    state.Begin(0);
    state.Accept({Version, 0});
    Require(!state.Ready(), "disconnected");
    state.Begin(9); state.Accept({Version, 9});
    BurstIssued burst = {};
    burst.version = Version; burst.nonce = 9; burst.caster = 1; burst.target = 2;
    burst.skill = MultiFireSkill; burst.level = 1; burst.count = 3; burst.lifetime_ms = LifetimeMs;
    for (unsigned part = 0; part < burst.count; ++part) burst.actions[part] = 500 + part;
    Require(state.CanAcceptBurst(burst, 1), "server burst metadata");
    burst.count = 4;
    Require(!state.CanAcceptBurst(burst, 1), "client cannot increase projectile count");
    burst.count = 3; burst.actions[2] = 501;
    Require(!state.CanAcceptBurst(burst, 1), "duplicate child ID rejected");
    burst.actions[2] = 502; burst.actions[7] = 900;
    Require(!state.CanAcceptBurst(burst, 1), "unused child slots must be empty");
    burst.actions[7] = 0;
    Require(state.RecordBurst(burst, 1, 100), "record parent and children");
    Require(!state.CanAcceptBurst(burst, 1), "duplicate burst issue");
    Require(state.Take(2, 101) == 0, "ordinary arrow cannot steal burst parent");
    Require(state.TakeBurst(2, IcingBlastSkill, 101) == 0, "other skill cannot steal parent");
    const uint64_t parent = state.TakeBurst(2, MultiFireSkill, 101);
    Require(parent == 500 && !state.TakeBurst(2, MultiFireSkill, 101), "bind parent once");
    Require(state.BurstCount(parent, MultiFireSkill, 101) == 3, "count uses server snapshot");
    Require(!state.TakePart(parent, 0, 5, 101), "local level change cannot increase count");
    Require(!state.TakePart(parent, 3, 3, 101), "fourth part rejected");
    Require(state.TakePart(parent, 0, 3, 101) == 500, "first projectile");
    Require(!state.TakePart(parent, 0, 3, 101), "same projectile cannot bind twice");
    Require(state.TakePart(parent, 2, 3, 101) == 502, "out of order child binding");
    Require(state.TakePart(parent, 1, 3, 101) == 501, "last child binding");
    Require(!state.BurstSkill(parent, 101) && !state.TakePart(parent, 1, 3, 101), "completed parent is retired");
    burst.level = 3; burst.count = 5;
    for (unsigned part = 0; part < burst.count; ++part) burst.actions[part] = 600 + part;
    Require(state.RecordBurst(burst, 1, 500), "five projectile issue");
    const uint64_t five = state.TakeBurst(2, MultiFireSkill, 501);
    state.ForgetPending();
    Require(state.BurstCount(five, MultiFireSkill, 502) == 5, "new animation does not clear a released burst");
    Require(!state.BurstCount(five, MultiFireSkill, 500 + LifetimeMs), "burst expiry");
    Require(!state.TakePart(five, 0, 5, 500 + LifetimeMs), "expired children cannot bind");
    for (unsigned skill : {unsigned(FleetMissileSkill), unsigned(ConvertingArmorSkill),
        unsigned(SharkMissileSkill), unsigned(FireExtremeSkill), unsigned(IceExtremeSkill)})
    {
        state.Begin(10); state.Accept({Version, 10});
        burst.nonce = 10; burst.skill = uint16_t(skill); burst.level = 3;
        burst.count = uint8_t(skill == FireExtremeSkill || skill == IceExtremeSkill ? 8 : 1);
        for (unsigned part = 0; part < MaxBurstParts; ++part)
            burst.actions[part] = part < burst.count ? 1000 + part : 0;
        const unsigned count = burst.count;
        burst.count = 9;
        Require(!state.CanAcceptBurst(burst, 1), "oversized skill envelope rejected before array access");
        burst.count = uint8_t(count);
        Require(state.RecordBurst(burst, 1, 100), "one and eight-part skill envelopes");
        Require(state.TakeBurst(2, skill, 100) == 1000, "one/eight-part root binds once");
        for (unsigned part = 0; part < count; ++part)
            Require(state.TakePart(1000, part, count, 100) == 1000 + part, "one/eight-part consumption");
        Require(!state.TakePart(1000, 0, count, 100), "one/eight-part rebind rejected");
    }
    state.Begin(11); state.Accept({Version, 11});
    shot.nonce = 11; shot.action = 2000;
    state.Record(shot, 100, FireArrowMagic);
    Require(!state.Take(2, 100) && !state.Take(2, 100, IceArrowMagic), "paid arrow cannot be rebound as physical/ice");
    Require(state.Take(2, 100, FireArrowMagic) == 2000 && !state.Take(2, 100, FireArrowMagic),
        "paid arrow binds once to the matching mode");
    ++shot.action; state.Record(shot, 100);
    Require(!state.Take(2, 100, FireArrowMagic) && state.Take(2, 100) == shot.action,
        "ordinary fallback cannot be rebound as magic");
    std::cout << "CombatShotClientStateTest passed\n";
}
