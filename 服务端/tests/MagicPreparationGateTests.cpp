#include "../Library/Shared/MagicPreparationGate.h"
#include "../Library/Shared/AutoPlayProtocol.h"
#include <iostream>

namespace
{
int failures = 0;

void Expect(bool condition, const char* name)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << name << '\n';
        ++failures;
    }
}
}

int main()
{
    using namespace DragonGameplay;
    const PreparationIdentity owner = {1, 2, 3, 4};
    MagicPreparationGate gate;
    Expect(!gate.CanExecute(owner, 10, 1000), "execute without casting");

    gate.Begin(owner, 10, 1000, 300);
    Expect(!gate.CanExecute(owner, 10, 1299), "early execute");
    Expect(gate.CanExecute(owner, 10, 1300), "exact preparation boundary");
    Expect(gate.CanExecute(owner, 10, 1500), "continue after preparation");
    Expect(!gate.CanExecute(owner, 11, 1500), "different skill cannot reuse cast");

    PreparationIdentity changed = owner;
    ++changed.agent;
    Expect(!gate.CanExecute(changed, 10, 1500), "agent connection reuse");
    changed = owner;
    ++changed.user;
    Expect(!gate.CanExecute(changed, 10, 1500), "user identity reuse");
    changed = owner;
    ++changed.character;
    Expect(!gate.CanExecute(changed, 10, 1500), "different character");
    changed = owner;
    ++changed.map;
    Expect(!gate.CanExecute(changed, 10, 1500), "cross map identity");

    gate.Reset();
    Expect(!gate.CanExecute(owner, 10, 1500), "explicit cancellation");
    gate.Begin(owner, 11, 2000, 4000);
    Expect(!gate.CanExecute(owner, 10, 6500), "replacement removes old skill");
    Expect(!gate.CanExecute(owner, 11, 5999), "long cast still preparing");
    Expect(gate.CanExecute(owner, 11, 6000), "long cast ready");

    gate.Begin(owner, 12, UINT32_MAX - 100, 300);
    Expect(!gate.CanExecute(owner, 12, 198), "early across clock wrap");
    Expect(gate.CanExecute(owner, 12, 199), "ready across clock wrap");
    gate.Begin(owner, 13, 20, 65535);
    Expect(!gate.CanExecute(owner, 13, 65554), "maximum wire duration early");
    Expect(gate.CanExecute(owner, 13, 65555), "maximum wire duration ready");
    gate.Begin(owner, 14, 42, 0);
    Expect(gate.CanExecute(owner, 14, 42), "zero actual preparation duration");

    if (failures == 0) std::cout << "Magic preparation gate tests passed\n";
    return failures != 0;
}
