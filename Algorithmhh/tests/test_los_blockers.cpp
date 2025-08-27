#include <iostream>
#include <cassert>
#include "WorldView.h"
#include "WorldViewDebug.h"
#include "LOS.h"
#include "TeamState.h"

using namespace Algorithm_212788293_212497127;

static void resetLine(WorldView &wv)
{
    // clear row y=2
    for (std::size_t x = 0; x < wv.w; ++x)
        wv.clearMask(x, 2, 0xFF);
}

int main()
{
    WorldView wv{10, 5};
    TeamState tv;
    tv.ensure(wv.w, wv.h);
    TankLocal me{1, 0, 0, 0, 0};

    // Enemy at (7,2); we stand at (2,2) facing east
    auto baseSetup = [&]()
    {
        resetLine(wv);
        wv.setMask(7, 2, WorldView::ENEMY);
        wv.setMask(2, 2, WorldView::FRIEND);
        me.x = 2;
        me.y = 2;
        me.facing_deg = 0;
    };

    // (1) Wall blocks
    baseSetup();
    wv.setMask(4, 2, WorldView::WALL);
    std::cout << "\n=== LOS blockers: wall ===\n";
    printWorldView(wv, std::cout, 'F', 'E');
    assert(!firstHitIsEnemyFacing(wv, tv, me));

    // (2) Friend blocks
    baseSetup();
    wv.setMask(4, 2, WorldView::FRIEND);
    assert(!firstHitIsEnemyFacing(wv, tv, me));

    // (3) Shell blocks (conservative policy)
    baseSetup();
    wv.setMask(4, 2, WorldView::SHELL);
    assert(!firstHitIsEnemyFacing(wv, tv, me));

    // (4) Mine blocks (policy)
    baseSetup();
    wv.setMask(4, 2, WorldView::MINE);
    assert(!firstHitIsEnemyFacing(wv, tv, me));

    std::cout << "LOS blockers: OK\n";
    return 0;
}
