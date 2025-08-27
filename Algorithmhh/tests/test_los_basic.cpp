#include <iostream>
#include <cassert>
#include "WorldView.h"
#include "WorldViewDebug.h"
#include "LOS.h"
#include "TeamState.h"

using namespace Algorithm_212788293_212497127;

static void placeMe(WorldView &wv, TankLocal &me, std::size_t x, std::size_t y, int facing)
{
    me.x = x;
    me.y = y;
    me.facing_deg = facing;
    wv.setMask(x, y, WorldView::FRIEND);
}

int main()
{
    WorldView wv{8, 5};
    TeamState tv;
    tv.ensure(wv.w, wv.h);
    TankLocal me{1, 0, 0, 0, 0};

    // Enemy to the east
    wv.setMask(5, 2, WorldView::ENEMY);

    // Case A: facing east (0°) → should see enemy first
    placeMe(wv, me, 2, 2, 0);
    std::cout << "\n=== LOS basic (east) ===\n";
    printWorldView(wv, std::cout, 'F', 'E');
    assert(firstHitIsEnemyFacing(wv, tv, me) && "Expected enemy first to the east");

    // Case B: facing south (90°) → no enemy in line
    placeMe(wv, me, 2, 2, 90);
    assert(!firstHitIsEnemyFacing(wv, tv, me) && "No enemy to the south");

    // Case C: diagonal NE (315°), enemy moved to (4,0)
    // reset enemy positions
    wv.clearMask(5, 2, WorldView::ENEMY);
    wv.setMask(4, 0, WorldView::ENEMY);
    placeMe(wv, me, 2, 2, 315);
    std::cout << "\n=== LOS basic (NE diagonal) ===\n";
    printWorldView(wv, std::cout, 'F', 'E');
    assert(firstHitIsEnemyFacing(wv, tv, me) && "Enemy on NE diagonal should be first hit");

    std::cout << "LOS basic: OK\n";
    return 0;
}
