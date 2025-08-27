#include <cassert>
#include <iostream>
#include "WorldView.h"
#include "WorldViewDebug.h"
#include "Danger.h"

using namespace Algorithm_212788293_212497127;

int main()
{
    WorldView wv{10, 5};

    // Enemy at (2,2), wall at (5,2) should stop east ray; west ray wraps until wall or full cycle.
    wv.setMask(2, 2, WorldView::ENEMY);
    wv.setMask(5, 2, WorldView::WALL);

    // Shells at (1,1) and (9,4)
    wv.setMask(1, 1, WorldView::SHELL);
    wv.setMask(9, 4, WorldView::SHELL);

    printWorldView(wv, std::cout, 'F', 'E');
    seedEnemyLOSDanger(wv, /*enemyLOS=*/8);
    seedShellDanger(wv, /*shellK=*/20);
    seedWallProximityDanger(wv, /*k=*/1, /*radius=*/1);

    // east of (2,2) until wall gets LOS=8; (4,2) is also adjacent to the wall → 8+1
    assert(wv.getDanger(3, 2) >= 8);
    assert(wv.getDanger(4, 2) >= 9); // 8 (LOS) + 1 (proximity)
    assert(wv.getDanger(5, 2) == 0); // wall, not marked with danger
    assert(wv.getDanger(6, 2) >= 9); // proximity only (+1)

    // Shell cells carry 20 (plus maybe proximity if near a wall)
    assert(wv.getDanger(1, 1) >= 20);
    assert(wv.getDanger(9, 4) >= 20);

    std::cout << "Danger basic: OK\n";
    return 0;
}
