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

static void applyRotation(int &facing, ::ActionRequest ar)
{
    if (ar == ::ActionRequest::RotateLeft45)
        facing = (facing + 7) % 8;
    if (ar == ::ActionRequest::RotateRight45)
        facing = (facing + 1) % 8;
}

int main()
{
    // 3x3 room: center is me, all 8 neighbors are walls -> truly trapped
    WorldView wv{3, 3};
    for (std::size_t y = 0; y < 3; ++y)
        for (std::size_t x = 0; x < 3; ++x)
            if (!(x == 1 && y == 1))
                wv.setMask(x, y, WorldView::WALL);

    TeamState tv;
    tv.ensure(3, 3);

    TankLocal me{};
    me.x = 1;
    me.y = 1;
    me.facing_deg = 0; // facing East initially

    BattleInfoLite bi{};
    bi.tag = RoleTag::Anchor;       // irrelevant in trapped mode
    bi.waypoint = Cell{me.x, me.y}; // no movement

    printBoardWithOverlay(wv, std::cout, "Breacher BEFORE", &me, nullptr);

    bool shot = false;
    Cell hit{};
    for (int steps = 0; steps < 8; ++steps)
    {
        ::ActionRequest ar = DecisionRunner::decide(bi, wv, tv, me);
        if (ar == ::ActionRequest::Shoot)
        {
            if (firstWallAhead(wv, me, hit))
                shot = true;
            break;
        }
        // While trapped by walls, we should rotate toward a wall—never move forward.
        assert(ar == ::ActionRequest::RotateLeft45 || ar == ::ActionRequest::RotateRight45);
        applyRotation(me.facing_deg, ar);
    }
    assert(shot && "Breacher should rotate until aligned, then shoot to carve an exit");
    printBoardWithOverlay(wv, std::cout, "Breacher AFTER (X = targeted wall)", &me, &hit);

    std::cout << "trapped-breacher: OK\n";
    return 0;
}
