#pragma once

// Weekly wall-clock window, including a window crossing midnight/Sunday.
inline unsigned long GetWeeklyWarRemainingSeconds(
    int applyDay, int startHour, int startMinute, int endHour, int endMinute,
    int currentDay, int currentHour, int currentMinute, int currentSecond)
{
    if (applyDay < 0 || applyDay > 6 || currentDay < 0 || currentDay > 6 ||
        startHour < 0 || startHour > 23 || endHour < 0 || endHour > 23 ||
        currentHour < 0 || currentHour > 23 || startMinute < 0 ||
        startMinute > 59 || endMinute < 0 || endMinute > 59 ||
        currentMinute < 0 || currentMinute > 59 || currentSecond < 0 ||
        currentSecond > 59)
    {
        return 0;
    }

    const int daySeconds = 24 * 60 * 60;
    const int weekSeconds = 7 * daySeconds;
    const int start = (startHour * 60 + startMinute) * 60;
    const int end = (endHour * 60 + endMinute) * 60;
    const int duration = (end - start + daySeconds) % daySeconds;
    const int now = currentDay * daySeconds + currentHour * 3600 +
                    currentMinute * 60 + currentSecond;
    const int elapsed = (now - (applyDay * daySeconds + start) +
                         weekSeconds) % weekSeconds;
    return elapsed < duration ? duration - elapsed : 0;
}

inline bool IsWarfieldNumberValid(int number, int oldCount, int newCount)
{
    return number >= 0 && number < oldCount + newCount;
}

inline bool IsScheduledWarDue(int applyDay, int applyHour, int lastStartedDate,
                            int currentDay, int currentHour, int currentDate)
{
    return applyDay == currentDay && applyHour == currentHour &&
           lastStartedDate != currentDate;
}
