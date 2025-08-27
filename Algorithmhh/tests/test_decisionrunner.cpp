#include <iostream>
#include <cassert>

#include "WorldView.h"
#include "WorldViewDebug.h"
#include "TeamState.h"
#include "DecisionRunner.h"
#include "Types.h"

// Only tests and .cpps include ActionRequest
#include "common/ActionRequest.h"

namespace Alg = Algorithm_212788293_212497127;
using namespace Algorithm_212788293_212497127;

static Alg::TankLocal me_at(std::size_t x, std::size_t y, int deg)
{
    Alg::TankLocal me{};
    me.x = x;
    me.y = y;
    me.facing_deg = deg;
    return me;
}

int main()
{
    // Case 1: SHOOT when enemy is first in LOS
    {
        WorldView wv{10, 6};
        TeamState tv;
        tv.ensure(wv.w, wv.h);
        wv.setMask(5, 3, WorldView::ENEMY);
        wv.setMask(2, 3, WorldView::FRIEND);
        BattleInfoLite bi{};
        bi.waypoint = Cell{9, 3};
        auto me = me_at(2, 3, 0); // facing east
        auto act = DecisionRunner::decide(bi, wv, tv, me);
        assert(act == ::ActionRequest::Shoot && "Should shoot enemy ahead");
    }

    // Case 2: ROTATE to align when waypoint is off to the south
    {
        WorldView wv{10, 6};
        TeamState tv;
        tv.ensure(wv.w, wv.h);
        wv.setMask(2, 2, WorldView::FRIEND);
        BattleInfoLite bi{};
        bi.waypoint = Cell{2, 5}; // below
        auto me = me_at(2, 2, 0); // currently facing east
        auto act = DecisionRunner::decide(bi, wv, tv, me);
        assert(act == ::ActionRequest::RotateRight45 || act == ::ActionRequest::RotateLeft45);
        // For our deterministic rule: facing east(0) to south(90) -> rotate right 45 first
        assert(act == ::ActionRequest::RotateRight45);
    }

    // Case 3: MOVE FORWARD when aligned and next cell free
    {
        WorldView wv{10, 6};
        TeamState tv;
        tv.ensure(wv.w, wv.h);
        wv.setMask(2, 2, WorldView::FRIEND);
        BattleInfoLite bi{};
        bi.waypoint = Cell{5, 2}; // straight east
        auto me = me_at(2, 2, 0);
        auto act1 = DecisionRunner::decide(bi, wv, tv, me);
        // currently aligned, next cell (3,2) free -> MoveForward
        assert(act1 == ::ActionRequest::MoveForward);
    }

    // Case 4: DO NOT step into reserved cell; rotate instead
    {
        WorldView wv{10, 6};
        TeamState tv;
        tv.ensure(wv.w, wv.h);
        wv.setMask(2, 2, WorldView::FRIEND);
        BattleInfoLite bi{};
        bi.waypoint = Cell{5, 2}; // straight east
        auto me = me_at(2, 2, 0);

        // reserve (3,2) for another friendly this step
        tv.reserveMove(3, 2, /*tank_id=*/0, /*ttl=*/3);

        auto act = DecisionRunner::decide(bi, wv, tv, me);
        assert(act != ::ActionRequest::MoveForward);
        // Our fallback is RotateLeft45
        assert(act == ::ActionRequest::RotateLeft45);
    }

    std::cout << "DecisionRunner basic: OK\n";
    return 0;
}
