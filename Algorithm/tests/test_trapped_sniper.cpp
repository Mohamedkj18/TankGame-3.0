#include <cassert>
#include <iostream>

#include "DecisionRunner.h"
#include "WorldView.h"
#include "TeamState.h"
#include "BattleInfoLite.h"
#include "Types.h"
#include "common/ActionRequest.h"
#include "print_utils.h"

using namespace Algorithm_212788293_212497127;

int main()
{
    // 5x5: me at (2,2), all 8 neighbors are mines, enemy two cells north.
    WorldView wv{5, 5};
    const Cell meC{2, 2};
    for (int dy = -1; dy <= 1; ++dy)
        for (int dx = -1; dx <= 1; ++dx)
            if (!(dx == 0 && dy == 0))
            {
                std::size_t nx = wv.wrapX(meC.x + dx);
                std::size_t ny = wv.wrapY(meC.y + dy);
                wv.setMask(nx, ny, WorldView::MINE);
            }
    // Enemy at (2,0): first hit north is a mine at (2,1), so LOS to enemy is blocked
    wv.setMask(2, 0, WorldView::ENEMY);

    TeamState tv;
    tv.ensure(5, 5);

    TankLocal me{};
    me.x = meC.x, me.y = meC.y;
    me.facing_deg = 0; // start facing East

    BattleInfoLite bi{};
    bi.tag = RoleTag::Survivor; // irrelevant in trapped mode
    bi.waypoint = meC;

    printBoardWithOverlay(wv, std::cout, "Sniper BEFORE", &me, nullptr);

    ::ActionRequest ar = DecisionRunner::decide(bi, wv, tv, me);
    // In "surrounded by mines", we must not move; we scan/rotate toward the enemy.
    assert(ar == ::ActionRequest::RotateLeft45 || ar == ::ActionRequest::RotateRight45 || ar == ::ActionRequest::Shoot); // Shoot only if LOS is clear (not here)
    // Since north lane is blocked by a mine, we expect rotation (determinstic left in our policy).
    assert(ar == ::ActionRequest::RotateLeft45);

    std::cout << "trapped-sniper: OK\n";
    return 0;
}
