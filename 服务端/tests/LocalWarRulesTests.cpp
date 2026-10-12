#include "../Mapserver/HigherLayers/LocalWarRules.h"

int main()
{
    const int nations[] = {3, 4, 6};
    const int results[] = {0, 3, 4, 6, 7, 9, 10, 13};
    const bool expected[][3] = {
        {false, false, false}, {true, false, false},
        {false, true, false}, {false, false, true},
        {true, true, false}, {true, false, true},
        {false, true, true}, {true, true, true}
    };
    for (int result = 0; result < 8; ++result)
        for (int nation = 0; nation < 3; ++nation)
            if (IsLocalWarWinningNation(results[result], nations[nation]) !=
                expected[result][nation]) return 1;
    if (IsLocalWarWinningNation(7, 0) || IsLocalWarWinningNation(8, 3)) return 2;
    if (IsValidLocalWarMap(-1) || IsValidLocalWarMap(7) ||
        !IsValidLocalWarMap(0) || !IsValidLocalWarMap(6)) return 3;
    if (IsValidLocalWarNation(0) || IsValidLocalWarNation(5) ||
        !IsValidLocalWarNation(6)) return 4;
    if (LocalWarExperienceGrade(1, 4) != 0 ||
        LocalWarExperienceGrade(4, 4) != 0 ||
        LocalWarExperienceGrade(30, 4) != 26 ||
        LocalWarExperienceGrade(200, 0) != 110) return 5;
    return 0;
}
