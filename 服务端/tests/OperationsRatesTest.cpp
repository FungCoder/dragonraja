#include <cassert>
#include <limits>
#include "../Library/Shared/OperationsRates.h"

int main()
{
    int output = 0;
    assert(ParseOperationsPercent(L"100", output) && output == 100);
    assert(!ParseOperationsPercent(L"100junk", output));
    assert(!ParseOperationsPercent(L"99999999999999", output));
    assert(!ParseOperationsPercent(L"0", output));
    assert(!ParseOperationsPercent(L"1001", output));
    assert(ScaleOperationsExperience(123, 100, output) && output == 123);
    assert(ScaleOperationsExperience(123, 200, output) && output == 246);
    assert(ScaleOperationsExperience(11.5, 150, output) && output == 17);
    assert(!ScaleOperationsExperience(INT_MAX, 200, output));
    assert(!ScaleOperationsExperience(std::numeric_limits<double>::quiet_NaN(), 100, output));
    assert(!ScaleOperationsExperience(-1, 100, output));
    assert(ScaleOperationsDraws(3, 100, 0, output) && output == 3);
    assert(ScaleOperationsDraws(3, 200, 99, output) && output == 6);
    assert(ScaleOperationsDraws(0, 200, 0, output) && output == 0);
    int total = 0;
    for (int random = 0; random < 100; ++random) {
        assert(ScaleOperationsDraws(3, 150, random, output));
        total += output;
    }
    assert(total == 450); // Exactly 4.5 draws in expectation; never multiply weights.
    assert(!ScaleOperationsDraws(INT_MAX, 1000, 99, output));
    return 0;
}
