#include "../Mapserver/HigherLayers/GameplaySafety.h"
#include <cassert>
#include <cstdio>

struct NpcFixture { short MoveSx, MoveSy; int state; };
struct TradeFixture
{
    int id, target, type, state;
    int GetServerID() const { return id; }
    int GetExchangeTargetId() const { return target; }
    int GetExchangeType() const { return type; }
    int GetExchangeState() const { return state; }
};

int main()
{
    assert(IsGameplayIndexValid(0, 398) && IsGameplayIndexValid(397, 398));
    assert(!IsGameplayIndexValid(-1, 398) && !IsGameplayIndexValid(398, 398));
    int storeCounts[30] = {};
    storeCounts[29] = 500;
    assert(IsStoreListValid(0, 30, storeCounts, 500));
    assert(IsStoreListValid(29, 30, storeCounts, 500));
    assert(!IsStoreListValid(-1, 30, storeCounts, 500));
    assert(!IsStoreListValid(30, 30, storeCounts, 500));
    storeCounts[0] = -1;
    assert(!IsStoreListValid(0, 30, storeCounts, 500));
    storeCounts[0] = 501;
    assert(!IsStoreListValid(0, 30, storeCounts, 500));
    assert(!IsStoreListValid(0, 30, nullptr, 500));
    for (int position = -1; position <= 256; ++position)
    {
        const bool expected = position >= 0 && position < 128 &&
            position / 32 < 3 && (position / 8) % 4 < 3;
        assert(IsInventoryPositionValid(position) == expected);
    }
    assert(IsRegularMonsterRespawnEligible(true, 0, 0, 0, false));
    assert(!IsRegularMonsterRespawnEligible(false, 0, 0, 0, false));
    assert(!IsRegularMonsterRespawnEligible(true, 1, 0, 0, false));
    assert(!IsRegularMonsterRespawnEligible(true, 0, 1, 0, false));
    assert(!IsRegularMonsterRespawnEligible(true, 0, 0, 1, false));
    assert(!IsRegularMonsterRespawnEligible(true, 0, 0, 0, true));
    // Exercise the actual countdown helpers with the existing removal threshold.
    for (int scenario = 0; scenario < 4; ++scenario)
    {
        const bool harvestable = (scenario % 2) != 0;
        const bool regularMonster = scenario < 2;
        int deathCount = regularMonster ? RegularMonsterDeathCount(harvestable, 5) :
            (harvestable ? 15 : 6);
        int ticks = 0;
        int clientRemovals = 0;
        while (deathCount > 1)
        {
            --deathCount;
            ++ticks;
            if (deathCount == 5)
            {
                ++clientRemovals;
                deathCount = HiddenMonsterDeathCount(regularMonster, deathCount);
            }
        }
        assert(clientRemovals == 1);
        assert(ticks == (regularMonster ? (harvestable ? 6 : 2) :
            (harvestable ? 14 : 5)));
    }
    assert(HiddenMonsterDeathCount(false, 5) == 5);
    assert(RegularMonsterDeathCount(true, 5) == 10);
    assert(RegularMonsterDeathCount(false, 5) == 6);
    int bossRow[] = {1, 1};
    int* bossRows[] = {nullptr, bossRow, nullptr};
    assert(IsBossPhaseValid(bossRows, 3, 1));
    assert(!IsBossPhaseValid(bossRows, 3, 0));
    assert(!IsBossPhaseValid(bossRows, 3, 2));
    assert(!IsBossPhaseValid(bossRows, 3, 3));
    assert(!IsBossPhaseValid(bossRows, 3, 23));
    bossRow[1] = 2;
    assert(!IsBossPhaseValid(bossRows, 3, 1));
    NpcFixture npc = {117, 61, 2};
    int total = 1;
    bool tileReleased = false;
    ClearRemovedNpcSlot(npc, total, [&](short& x, short& y) {
        assert(x == 117 && y == 61 && npc.state == 2);
        tileReleased = true;
    });
    assert(tileReleased && npc.MoveSx == 0 && npc.MoveSy == 0 && npc.state == 0 && total == 0);
    int generation = 1;
    assert(ReleaseGenerationCount(generation) && generation == 0);
    assert(!ReleaseGenerationCount(generation) && generation == 0);
    generation = -3;
    assert(!ReleaseGenerationCount(generation) && generation == 0);
    assert(IsTradeItemIndexValid(19, 20));
    assert(!IsTradeItemIndexValid(20, 20) && !IsTradeItemIndexValid(-1, 20));
    TradeFixture first = {2, 3, 1, 3}, second = {3, 2, 1, 3};
    assert(IsConfirmedTrade(&first, &second, 3));
    assert(IsActiveTrade(&first, &second, 0));
    second.state = 0;
    assert(!IsActiveTrade(&first, &second, 0));
    second.state = 3;
    int checked = 0;
    assert(ValidateTradeOffers(20, [&](int) { ++checked; return true; }));
    assert(checked == 20);
    checked = 0;
    assert(!ValidateTradeOffers(20, [&](int index) { ++checked; return index != 4; }));
    assert(checked == 5);
    second.state = 2;
    assert(!IsConfirmedTrade(&first, &second, 3));
    second.state = 3; second.target = 4;
    assert(!IsMutualTrade(&first, &second));
    assert(!IsMutualTrade(&first, &first));
    assert(!IsMutualTrade(&first, static_cast<TradeFixture*>(nullptr)));
    std::printf("NPC respawn/protected countdown, cleanup/counter and trade checks passed\n");
    return 0;
}
