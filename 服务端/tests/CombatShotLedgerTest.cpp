#include "../Library/Shared/CombatShotLedger.h"
#include <cstdlib>
#include <iostream>

static void Require(bool condition, const char* scenario)
{
    if (!condition) { std::cerr << scenario << '\n'; std::exit(1); }
}
int main()
{
    using namespace DragonGameplay;
    using namespace DragonCombatShot;
    CombatShotLedger ledger;
    PreparationIdentity owner = {1, 2, 3, 4}, other = owner;
    ShotAction action, consumed;
    action.id = 1; action.launch_x = 50; action.attack_kind = 7;
    Require(!ledger.Issue(owner, action, 0), "cannot issue before handshake");
    Require(!ledger.Handshake(owner, 0), "zero session nonce");
    Require(ledger.Handshake(owner, 9), "handshake");
    Require(ledger.Issue(owner, action, 100), "issue actual action");
    Require(!ledger.Issue(owner, action, 100), "duplicate action id");
    Require(!ledger.Consume(owner, 9, 2, 100, consumed), "unknown id");
    Require(!ledger.Consume(owner, 8, 1, 100, consumed), "wrong nonce");
    ++other.user;
    Require(!ledger.Consume(other, 9, 1, 100, consumed), "other connection owner");
    other = owner; ++other.map;
    Require(!ledger.Consume(other, 9, 1, 100, consumed), "cross map replay");
    other = owner; ++other.character;
    Require(!ledger.Consume(other, 9, 1, 100, consumed), "other character");
    other = owner; ++other.agent;
    Require(!ledger.Consume(other, 9, 1, 100, consumed), "agent reuse");
    Require(ledger.Handshake(owner, 9), "retry handshake");
    Require(ledger.Consume(owner, 9, 1, 101, consumed), "retry preserves in-flight action");
    Require(consumed.launch_x == 50 && consumed.attack_kind == 7, "server snapshot retained");
    Require(!ledger.Consume(owner, 9, 1, 101, consumed), "duplicate result consumed once");
    // A failed legacy filter happens after Consume: the action remains spent.
    Require(!ledger.Consume(owner, 9, 1, 102, consumed), "failed-hit retry cannot consume again");
    action.id = 2;
    Require(ledger.Issue(owner, action, 500), "second action");
    Require(ledger.Consume(owner, 9, 2, 500 + LifetimeMs - 1, consumed), "last valid millisecond");
    action.id = 3;
    Require(ledger.Issue(owner, action, 500), "expiry action");
    Require(!ledger.Consume(owner, 9, 3, 500 + LifetimeMs, consumed), "exact expiry");
    action.id = 4;
    Require(ledger.Issue(owner, action, UINT32_MAX - 100), "issue across clock wrap");
    Require(ledger.Consume(owner, 9, 4, uint64_t(UINT32_MAX) + 201, consumed),
        "64-bit monotonic clock crosses 32-bit boundary");
    action.id = 6;
    Require(ledger.Issue(owner, action, 0), "long idle action");
    Require(!ledger.Consume(owner, 9, 6, uint64_t(UINT32_MAX) + 101, consumed),
        "49-day clock boundary must not resurrect expired authorization");
    action.id = 7;
    Require(ledger.Issue(owner, action, 100), "clock reversal case");
    Require(!ledger.Consume(owner, 9, 7, 99, consumed), "backward clock fails closed");
    action.id = 5;
    Require(ledger.Issue(owner, action, 0), "before reconnect");
    Require(ledger.Handshake(owner, 10), "new nonce clears old actions");
    Require(!ledger.Consume(owner, 10, 5, 1, consumed), "reconnect cannot reuse action");
    for (unsigned id = 1; id <= MaxOutstanding; ++id)
    {
        action.id = 100 + id;
        Require(ledger.Issue(owner, action, 1000), "bounded outstanding actions");
    }
    action.id = 1000;
    Require(!ledger.Issue(owner, action, 1000), "full ledger rejects without eviction");
    Require(ledger.Issue(owner, action, 1000 + LifetimeMs), "expired slot reused");
    ledger.ClearActions();
    Require(!ledger.Consume(owner, 10, 1000, 1000 + LifetimeMs, consumed), "death cancels in-flight shots");
    Require(ledger.Nonce(owner) == 10, "death preserves negotiated session for revival");
    Require(ledger.Issue(owner, action, 20000), "revival can issue new shot");
    ledger.Reset();
    Require(!ledger.Consume(owner, 10, 1000, 1000 + LifetimeMs, consumed), "death/disconnect clears actions");
    Require(ledger.Nonce(owner) == 0, "reset requires new handshake");
    CombatShotLedger burstLedger;
    Require(burstLedger.Handshake(owner, 20), "burst session");
    ShotAction burst;
    burst.id = 1000; burst.parts = 3;
    Require(burstLedger.Issue(owner, burst, 100), "one release authorizes three parts");
    Require(burstLedger.Consume(owner, 20, 1001, 101, consumed) && consumed.id == 1001,
        "out-of-order part is independent");
    Require(!burstLedger.Consume(owner, 20, 1001, 101, consumed), "same part cannot repeat");
    ShotAction overlap;
    overlap.id = 1002; overlap.parts = 3;
    Require(!burstLedger.Issue(owner, overlap, 102), "live part ranges cannot overlap");
    Require(burstLedger.Consume(owner, 20, 1000, 103, consumed), "first part after second");
    Require(burstLedger.Consume(owner, 20, 1002, 103, consumed), "third part after second");
    Require(!burstLedger.Consume(owner, 20, 1000, 104, consumed), "fully spent release cannot replay");
    burst.id = 2000; burst.parts = 5;
    Require(burstLedger.Issue(owner, burst, 200), "five-part release");
    for (unsigned part = 0; part < 5; ++part)
        Require(burstLedger.Consume(owner, 20, burst.id + part, 201, consumed), "all valid parts can hit one target");
    Require(!burstLedger.Consume(owner, 20, 2005, 202, consumed), "sixth invented part rejected");
    burst.id = UINT64_MAX - 1; burst.parts = 3;
    Require(!burstLedger.Issue(owner, burst, 300), "part ID overflow");
    burst.id = 3000; burst.parts = MaxBurstParts + 1;
    Require(!burstLedger.Issue(owner, burst, 300), "oversized burst");
    Require(BurstParts(MultiFireSkill, 1) == 3 && BurstParts(MultiFireSkill, 2) == 3 &&
        BurstParts(MultiFireSkill, 3) == 5 && BurstParts(IcingBlastSkill, 3) == 5,
        "production level-to-projectile rules");
    Require(!BurstParts(MultiFireSkill, 0) && !BurstParts(IcingBlastSkill, 4) &&
        !BurstParts(232, 3), "unknown skills and levels grant no burst count");
    std::cout << "CombatShotLedgerTest passed\n";
}
