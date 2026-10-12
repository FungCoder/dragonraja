#include "../Mapserver/stdafx.h"
#include "../Mapserver/HigherLayers/WarStartInfo.h"
#include "../Mapserver/HigherLayers/WarSchedule.h"
#include "../Mapserver/HigherLayers/WarStartupReader.h"
#include <cassert>
#include <cstdio>

int g_wday = 1;
int g_hour = 20;
int g_min = 0;
int g_sec = 0;
CLottoSystem* CLottoSystem::m_pClass = NULL;

static void CheckTime(CWarStartInfo& info, int day, int hour, int minute,
                      int second, unsigned long expected)
{
    g_wday = day;
    g_hour = hour;
    g_min = minute;
    g_sec = second;
    info.Execute();
    assert(info.IsActive() == (expected != 0));
    assert(info.GetRemainTime() == expected);
}

int main()
{
    CWarStartInfo* info = CWarStartInfo::CreateWarStartInfo();
    info->Register(new START_UP_INFO{1, 1, 20, 0, 22, 0});
    CheckTime(*info, 1, 19, 59, 59, 0);
    CheckTime(*info, 1, 20, 1, 15, 7125); // missed startup minute / restart
    CheckTime(*info, 1, 21, 59, 59, 1);
    CheckTime(*info, 1, 22, 1, 0, 0); // missed shutdown minute
    CheckTime(*info, 2, 20, 0, 0, 0);
    CheckTime(*info, 1, 20, 0, 0, 7200); // next week's occurrence
    CheckTime(*info, 1, 20, 0, 0, 7200); // repeated call does not extend
    delete info;

    info = CWarStartInfo::CreateWarStartInfo();
    info->Register(new START_UP_INFO{2, 6, 23, 30, 1, 30});
    CheckTime(*info, 6, 23, 29, 59, 0);
    CheckTime(*info, 6, 23, 30, 0, 7200);
    CheckTime(*info, 0, 0, 0, 0, 5400);
    CheckTime(*info, 0, 1, 30, 0, 0);
    delete info;

    assert(GetWeeklyWarRemainingSeconds(-1, 20, 0, 22, 0, 1, 20, 0, 0) == 0);
    assert(GetWeeklyWarRemainingSeconds(1, 24, 0, 22, 0, 1, 20, 0, 0) == 0);
    assert(GetWeeklyWarRemainingSeconds(1, 20, 0, 20, 0, 1, 20, 0, 0) == 0);
    assert(GetWeeklyWarRemainingSeconds(1, 20, 0, 22, 0, 1, 20, 0, 60) == 0);
    assert(!IsWarfieldNumberValid(-1, 3, 1));
    assert(IsWarfieldNumberValid(0, 3, 1));
    assert(IsWarfieldNumberValid(3, 3, 1));
    assert(!IsWarfieldNumberValid(4, 3, 1));
    int lastStartedDate = -1;
    assert(!IsScheduledWarDue(6, 14, lastStartedDate, 6, 13, 20261017));
    assert(IsScheduledWarDue(6, 14, lastStartedDate, 6, 14, 20261017));
    // A disconnected target does not consume today's occurrence.
    assert(IsScheduledWarDue(6, 14, lastStartedDate, 6, 14, 20261017));
    lastStartedDate = 20261017; // committed only after a target is available
    assert(!IsScheduledWarDue(6, 14, lastStartedDate, 6, 14, 20261017));
    assert(IsScheduledWarDue(6, 14, lastStartedDate, 6, 14, 20261024));
    std::puts("Weekly war schedule and warfield bounds passed.");
    return 0;
}
