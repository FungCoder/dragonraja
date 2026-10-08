#pragma once
#include <cstring>

inline bool IsRegularMonsterRespawnEligible(bool ordinaryMapMonster,
    int eventNumber, int bossMarker, int bossPhase, bool tamed)
{
    return ordinaryMapMonster && eventNumber == 0 && bossMarker == 0 &&
        bossPhase == 0 && !tamed;
}

inline int RegularMonsterDeathCount(bool harvestable, int removeThreshold)
{
    // Five 5-second checks for harvesting; one check without meat or skin.
    return removeThreshold + (harvestable ? 5 : 1);
}

inline int HiddenMonsterDeathCount(bool regularMonster, int originalCount)
{
    // Keep one check between client removal and server slot reuse.
    return regularMonster ? 2 : originalCount;
}

template<class Counter>
bool ReleaseGenerationCount(Counter& count)
{
    if (count <= 0) { count = 0; return false; }
    --count;
    return true;
}

template<class Npc, class ReleaseTile>
void ClearRemovedNpcSlot(Npc& npc, int& activeCount, ReleaseTile releaseTile)
{
    releaseTile(npc.MoveSx, npc.MoveSy);
    std::memset(&npc, 0, sizeof(npc));
    if (activeCount > 0) --activeCount;
}

inline bool IsTradeItemIndexValid(int index, int capacity)
{
    return index >= 0 && index < capacity;
}

template<class Row>
bool IsBossPhaseValid(Row* const* rows, int capacity, int phase)
{
    return rows && phase > 0 && phase < capacity && rows[phase] && rows[phase][1] == phase;
}

template<class Player>
bool IsMutualTrade(const Player* first, const Player* second)
{
    return first && second && first != second &&
        first->GetExchangeTargetId() == second->GetServerID() &&
        second->GetExchangeTargetId() == first->GetServerID() &&
        first->GetExchangeType() == second->GetExchangeType();
}

template<class Player>
bool IsConfirmedTrade(const Player* first, const Player* second, int confirmedState)
{
    return IsMutualTrade(first, second) &&
        first->GetExchangeState() == confirmedState && second->GetExchangeState() == confirmedState;
}

template<class Player>
bool IsActiveTrade(const Player* first, const Player* second, int readyState)
{
    return IsMutualTrade(first, second) &&
        first->GetExchangeState() != readyState && second->GetExchangeState() != readyState;
}

template<class ValidateOffer>
bool ValidateTradeOffers(int capacity, ValidateOffer validate)
{
    for (int i = 0; i < capacity; ++i)
        if (!validate(i)) return false;
    return true;
}
