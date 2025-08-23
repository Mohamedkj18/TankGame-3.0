#include <cassert>
#include <iostream>
#include "roles/RolePolicy.h"
#include "WorldView.h"
#include "Danger.h"
#include "Types.h"

using namespace Algorithm_212788293_212497127;

int main()
{
    // Map with a wall line (gap at x=6) and one enemy behind it.
    WorldView wv{12, 7};
    for (std::size_t x = 2; x <= 9; ++x)
        if (x != 6)
            wv.setMask(x, 3, WorldView::WALL);
    wv.setMask(9, 3, WorldView::ENEMY);
    Cell me{1, 3};

    // Seed dangers (decaying LOS + shells none + proximity)
    seedEnemyLOSDanger(wv, 8, 1, 10, true);
    seedWallProximityDanger(wv, 1, 1);

    // Aggressor → approach near enemy (not on enemy) and passable
    {
        Cell tgt = targetForRole(RoleTag::Aggressor, wv, me);
        assert(!(tgt.x == 9 && tgt.y == 3)); // not the enemy cell
        assert(!wv.isBlocked(tgt.x, tgt.y)); // passable
    }
    // Anchor → central control
    {
        Cell tgt = targetForRole(RoleTag::Anchor, wv, me);
        assert(!wv.isBlocked(tgt.x, tgt.y));
        // near center (no strict numeric, just passability check here)
    }
    // Flanker → ring 1–2 around enemy if available
    {
        Cell tgt = targetForRole(RoleTag::Flanker, wv, me);
        assert(!wv.isBlocked(tgt.x, tgt.y));
        const std::size_t dE = wv.manhattanToroidal(tgt, Cell{9, 3});
        assert(dE >= 1); // not sitting on enemy
    }
    // Survivor → safe and passable
    {
        Cell tgt = targetForRole(RoleTag::Survivor, wv, me);
        assert(!wv.isBlocked(tgt.x, tgt.y));
    }
    std::cout << "role targets: OK\n";
    return 0;
}
