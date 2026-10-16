#pragma once
#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <vector>

namespace DragonNavigation
{
struct Tile { int x, y; };
// PathFind returns the destination followed by parent links back to the origin.
// Reject disconnected/cyclic/truncated chains before selecting a near-origin segment.
template<class Node>
bool SelectSegment(const Node* destination, int startX, int startY,
                   std::size_t maxSteps, Tile& result)
{
    if (!destination || maxSteps == 0 || maxSteps >= 50) return false;
    std::vector<Tile> reverse;
    const Node* current = destination;
    for (std::size_t count = 0; current && count <= 1001; ++count)
    {
        if (!reverse.empty())
        {
            const Tile& previous = reverse.back();
            const int dx = std::abs(current->x - previous.x);
            const int dy = std::abs(current->y - previous.y);
            if (dx > 1 || dy > 1 || (dx == 0 && dy == 0)) return false;
        }
        reverse.push_back({current->x, current->y});
        if (!current->lpParent)
        {
            if (current->x != startX || current->y != startY || reverse.size() < 2)
                return false;
            const std::size_t steps = (std::min)(maxSteps, reverse.size() - 1);
            result = reverse[reverse.size() - 1 - steps];
            return true;
        }
        current = current->lpParent;
    }
    return false;
}
}
