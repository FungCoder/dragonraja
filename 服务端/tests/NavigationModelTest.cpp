#include "../Client/Debug/NavigationModel.h"
#include <cstdlib>
#include <iostream>
#include <vector>
struct Node { int x, y; Node* lpParent; };
static void Require(bool condition, const char* scenario)
{
    if (!condition) { std::cerr << scenario << '\n'; std::exit(1); }
}
int main()
{
    DragonNavigation::Tile result = {-1, -1};
    std::vector<Node> chain(81);
    for (int index = 0; index <= 80; ++index)
    {
        chain[index] = {index, 5, index ? &chain[index - 1] : nullptr};
    }
    Require(DragonNavigation::SelectSegment(&chain[80], 0, 5, 16, result), "long path accepted");
    Require(result.x == 16 && result.y == 5, "segment stays near actual origin, not destination");
    Require(DragonNavigation::SelectSegment(&chain[80], 0, 5, 49, result) && result.x == 49,
        "maximum 50 node contract includes actual origin");
    Require(DragonNavigation::SelectSegment(&chain[1], 0, 5, 16, result) && result.x == 1,
        "one step reaches exact destination");
    Require(DragonNavigation::SelectSegment(&chain[16], 0, 5, 16, result) && result.x == 16,
        "exact segment boundary");
    Require(!DragonNavigation::SelectSegment(&chain[0], 0, 5, 16, result), "zero movement rejected");
    Require(!DragonNavigation::SelectSegment(&chain[80], 1, 5, 16, result), "wrong origin rejected");
    Require(!DragonNavigation::SelectSegment(&chain[80], 0, 5, 50, result), "over capacity rejected");
    Require(!DragonNavigation::SelectSegment<Node>(nullptr, 0, 5, 16, result), "unreachable rejected");
    chain[40].x = 900;
    Require(!DragonNavigation::SelectSegment(&chain[80], 0, 5, 16, result), "disconnected chain rejected");
    chain[40].x = 40;
    chain[0].lpParent = &chain[1];
    Require(!DragonNavigation::SelectSegment(&chain[80], 0, 5, 16, result), "cycle bounded and rejected");
    chain[0].lpParent = nullptr;
    chain[1].x = 0;
    Require(!DragonNavigation::SelectSegment(&chain[80], 0, 5, 16, result), "duplicate tile rejected");
    Node bend[5] = {{2, 3, nullptr}, {3, 4, &bend[0]}, {4, 4, &bend[1]},
        {4, 3, &bend[2]}, {5, 2, &bend[3]}};
    Require(DragonNavigation::SelectSegment(&bend[4], 2, 3, 3, result) && result.x == 4 && result.y == 3,
        "bends and diagonal chain preserve traversal order");
    std::cout << "NavigationModelTest passed\n";
}
