#pragma once

// Map-to-map results keep the historical sum-of-nation-codes format.
inline bool IsLocalWarWinningNation(int result, int nation)
{
    switch (nation)
    {
    case 3: return result == 3 || result == 7 || result == 9 || result == 13;
    case 4: return result == 4 || result == 7 || result == 10 || result == 13;
    case 6: return result == 6 || result == 9 || result == 10 || result == 13;
    default: return false;
    }
}

inline bool IsValidLocalWarNation(int nation)
{
    return nation == 3 || nation == 4 || nation == 6;
}

inline bool IsValidLocalWarMap(int map)
{
    return map >= 0 && map < 7;
}

inline int LocalWarExperienceGrade(int level, int offset)
{
    if (level <= offset) return 0;
    if (level - offset > 110) return 110;
    return level - offset;
}
