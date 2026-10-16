#include "../Library/Shared/BattleSkillSafety.h"
#include <iostream>
#include <cstdlib>

static void Require(bool condition, const char* scenario)
{
    if (!condition) { std::cerr << scenario << '\n'; std::exit(1); }
}

int main()
{
    unsigned char wizard[200] = {}, priest[64] = {};
    wizard[0] = priest[0] = 1;
    wizard[1] = wizard[199] = priest[1] = priest[63] = 1;
    using DragonGameplay::HasLearnedSpell;
    Require(!HasLearnedSpell(0, 0, wizard), "zero is not learned");
    Require(!HasLearnedSpell(-1, 0, wizard), "negative canonical");
    Require(HasLearnedSpell(1, 0, wizard), "first wizard spell");
    Require(HasLearnedSpell(199, 0, wizard), "last wizard slot");
    Require(!HasLearnedSpell(200, 0, wizard), "wizard boundary");
    Require(!HasLearnedSpell(255, 0, wizard), "forged wizard slot");
    Require(!HasLearnedSpell(1, 150, priest), "no negative priest index");
    Require(!HasLearnedSpell(150, 150, priest), "priest zero excluded");
    Require(HasLearnedSpell(151, 150, priest), "first priest spell");
    Require(HasLearnedSpell(213, 150, priest), "last priest slot");
    Require(!HasLearnedSpell(214, 150, priest), "priest boundary");
    Require(!HasLearnedSpell(152, 150, priest), "not learned");
    std::cout << "BattleSkillSafetyTest passed\n";
}
