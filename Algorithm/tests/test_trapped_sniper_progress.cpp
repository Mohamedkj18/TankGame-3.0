#include <cassert>
#include <iostream>
#include <string>

#include "DecisionRunner.h"
#include "WorldView.h"
#include "TeamState.h"
#include "BattleInfoLite.h"
#include "Types.h"
#include "common/ActionRequest.h"
#include "print_utils.h"

using namespace Algorithm_212788293_212497127;

static void applyRotation(int &facing_deg, ::ActionRequest ar)
{
    if (ar == ::ActionRequest::RotateLeft45)
        facing_deg = (facing_deg + 7) % 8;
    if (ar == ::ActionRequest::RotateRight45)
        facing_deg = (facing_deg + 1) % 8;
}

int main()
{
    // 7x5: tank at (3,2), all 8 neighbors are MINES (trapped).
    // Enemy to the NORTH at (3,0). Mine at (3,1) initially blocks LOS.
    WorldView wv{7, 5};
    const Cell meC{3, 2};

    // Surround with mines (8 neighbors)
    for (int dy = -1; dy <= 1; ++dy)
        for (int dx = -1; dx <= 1; ++dx)
            if (!(dx == 0 && dy == 0))
            {
                std::size_t nx = wv.wrapX(meC.x + dx);
                std::size_t ny = wv.wrapY(meC.y + dy);
                wv.setMask(nx, ny, WorldView::MINE);
            }

    // Enemy north
    wv.setMask(3, 0, WorldView::ENEMY);

    // (Optional) mark the tank cell as a friend purely for visualization (DecisionRunner uses TankLocal)
    wv.setMask(meC.x, meC.y, WorldView::FRIEND);

    TeamState tv;
    tv.ensure(wv.w, wv.h);

    TankLocal me{};
    me.x = meC.x, me.y = meC.y;
    me.facing_deg = 0; // start facing_deg East

    // sanity: verify we’re where we think we are
    assert(me.x == 3 && me.y == 2);

    BattleInfoLite bi{};
    bi.tag = RoleTag::Survivor; // role irrelevant in trapped mode
    bi.waypoint = meC;

    bool shot_before_clear = false;
    bool shot_after_clear = false;
    const Cell markBreached{3, 1};

    // --- Phase 1: BEFORE clearing the mine, run a few decision steps
    for (int step = 0; step < 4; ++step)
    {
        printBoardWithOverlay(wv, std::cout,
                              ("Sniper Progress - BEFORE clear (step " + std::to_string(step) + ")").c_str(),
                              &me, nullptr);

        ::ActionRequest ar = DecisionRunner::decide(bi, wv, tv, me);
        if (ar == ::ActionRequest::Shoot)
        {
            shot_before_clear = true;
            break;
        }

        // Expected: rotation toward north (no forward moves when trapped by mines)
        assert(ar == ::ActionRequest::RotateLeft45 || ar == ::ActionRequest::RotateRight45);
        applyRotation(me.facing_deg, ar);
        tv.age();
    }
    // Must NOT shoot while mine blocks (3,1)
    assert(!shot_before_clear && "Should not shoot before clearing the blocking mine");

    // --- Phase 2: CLEAR the north blocking mine (simulate ally action)
    {
        WorldView fresh{wv.w, wv.h};
        for (std::size_t y = 0; y < wv.h; ++y)
        {
            for (std::size_t x = 0; x < wv.w; ++x)
            {
                if (x == markBreached.x && y == markBreached.y)
                    continue; // leave empty
                if (wv.isWall(x, y))
                    fresh.setMask(x, y, WorldView::WALL);
                else if (wv.isMine(x, y))
                    fresh.setMask(x, y, WorldView::MINE);
                else if (wv.hasShell(x, y))
                    fresh.setMask(x, y, WorldView::SHELL);
                else if (wv.hasEnemy(x, y))
                    fresh.setMask(x, y, WorldView::ENEMY);
                else if (wv.hasFriend(x, y))
                    fresh.setMask(x, y, WorldView::FRIEND);
            }
        }
        wv = fresh;
    }

    printBoardWithOverlay(wv, std::cout,
                          "Sniper Progress - AFTER clear (X = removed mine)", &me, &markBreached);

    // --- Phase 3: AFTER clearing, allow up to 8 steps to align and shoot
    for (int step = 0; step < 8; ++step)
    {
        ::ActionRequest ar = DecisionRunner::decide(bi, wv, tv, me);
        if (ar == ::ActionRequest::Shoot)
        {
            shot_after_clear = true;
            break;
        }
        // Still trapped → only rotations expected until aligned
        assert(ar == ::ActionRequest::RotateLeft45 || ar == ::ActionRequest::RotateRight45);
        applyRotation(me.facing_deg, ar);
        tv.age();
    }

    assert(shot_after_clear && "After the mine is cleared, tank should align and Shoot the enemy");

    std::cout << "trapped-sniper-progress: OK\n";
    return 0;
}
