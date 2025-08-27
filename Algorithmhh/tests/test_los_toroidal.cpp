#include <iostream>
#include <cassert>
#include "WorldView.h"
#include "WorldViewDebug.h"
#include "LOS.h"
#include "TeamState.h"

using namespace Algorithm_212788293_212497127;

int main()
{
    WorldView wv{8, 5};
    TeamState tv;
    tv.ensure(wv.w, wv.h);
    TankLocal me{1, 0, 0, 0, 0};

    // Me at (6,2), facing east. Enemy at (1,2): path is 7 -> 0 -> 1
    wv.setMask(6, 2, WorldView::FRIEND);
    me.x = 6;
    me.y = 2;
    me.facing_deg = 0;
    wv.setMask(1, 2, WorldView::ENEMY);

    std::cout << "\n=== LOS toroidal: no blocker ===\n";
    printWorldView(wv, std::cout, 'F', 'E');
    assert(firstHitIsEnemyFacing(wv, tv, me) && "Should see enemy via wrap");

    // Now block with a wall at (0,2) – comes before enemy (order: 7,0,1)
    wv.setMask(0, 2, WorldView::WALL);
    std::cout << "\n=== LOS toroidal: blocked at wrap ===\n";
    printWorldView(wv, std::cout, 'F', 'E');
    assert(!firstHitIsEnemyFacing(wv, tv, me) && "Wall on wrap blocks LOS");

    std::cout << "LOS toroidal: OK\n";
    return 0;
}
