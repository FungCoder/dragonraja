#pragma once
#include <stddef.h>

namespace DragonGameplay
{
// Canonical IDs use a 150 offset for priests; zero is not a learned spell.
template<class Value, size_t Count>
bool HasLearnedSpell(int canonical, int offset, const Value (&learned)[Count])
{
    if (canonical <= offset) return false;
    const unsigned index = static_cast<unsigned>(canonical - offset);
    return index < Count && learned[index] != 0;
}
}
