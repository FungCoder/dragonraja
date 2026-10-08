#pragma once
#include <cstddef>

inline bool IsEventIndexValid(int index, int count)
{
    return index >= 0 && index < count;
}

template<std::size_t Capacity>
int FindEventGroupIndex(const int (&groups)[Capacity], int count, int groupNumber)
{
    if (count < 0 || static_cast<std::size_t>(count) > Capacity) return -1;
    for (int index = 0; index < count; ++index)
        if (groups[index] == groupNumber) return index;
    return -1;
}
