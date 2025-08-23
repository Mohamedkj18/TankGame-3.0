#include <cassert>
#include <iostream>
#include "WorldView.h"
#include "Danger.h"

using namespace Algorithm_212788293_212497127;

int main()
{
    WorldView wv{12, 12};
    wv.setMask(5, 5, WorldView::SHELL);

    // Immediate + predictive: base=20, decay=6, horizon=2, mark_intermediate=false
    seedShellDanger(wv, 20);
    seedShellDangerPredictiveUnknown(wv, /*base=*/20, /*decay=*/6, /*horizon=*/2, /*mark_intermediate=*/false);

    auto at = [&](int dx, int dy, int dist)
    {
        std::size_t x = wv.wrapX(5 + dx * dist);
        std::size_t y = wv.wrapY(5 + dy * dist);
        return wv.getDanger(x, y);
    };

    // distance 0: current shell
    assert(wv.getDanger(5, 5) >= 20);

    // distance 2 (t=1) in 8 dirs should have >=20
    const int D[8][2] = {{+1, 0}, {+1, +1}, {0, +1}, {-1, +1}, {-1, 0}, {-1, -1}, {0, -1}, {+1, -1}};
    for (auto &d : D)
    {
        assert(at(d[0], d[1], 2) >= 20);
    }

    // distance 4 (t=2) in 8 dirs should have >=14 (20 - 6)
    for (auto &d : D)
    {
        assert(at(d[0], d[1], 4) >= 14);
    }

    // a far cell should remain zero
    assert(wv.getDanger(0, 0) == 0);

    std::cout << "Shell predictive (unknown dir): OK\n";
    return 0;
}
