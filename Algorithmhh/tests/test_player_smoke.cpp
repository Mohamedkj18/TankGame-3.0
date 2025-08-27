#include <iostream>
#include <cassert>

#include "WorldView.h"
#include "WorldViewDebug.h"
#include "Pathfinding.h"
#include "TeamState.h"
#include "DecisionRunner.h"
#include "LOS.h"
#include "Types.h"

// Only .cpp/tests may include ActionRequest
#include "common/ActionRequest.h"

namespace Alg = Algorithm_212788293_212497127;

static Alg::Cell nearestEnemy(const Alg::WorldView &wv, Alg::Cell me)
{
    Alg::Cell best = me;
    std::size_t bestd = SIZE_MAX;
    for (std::size_t y = 0; y < wv.h; ++y)
        for (std::size_t x = 0; x < wv.w; ++x)
            if (wv.hasEnemy(x, y))
            {
                const auto d = wv.manhattanToroidal(me, Alg::Cell{x, y});
                if (d < bestd)
                {
                    bestd = d;
                    best = Alg::Cell{x, y};
                }
            }
    return best;
}

static Alg::Cell firstHop(const Alg::WorldView &wv, Alg::Cell from, Alg::Cell to)
{
    auto p = Alg::aStar(wv, from, to);
    return (p.size() >= 2) ? p[1] : from;
}

int main()
{
    // World with a small choke and two friends; one enemy ahead
    Alg::WorldView wv{12, 7};
    // walls (choke at y=3, gap at x=6)
    for (std::size_t x = 2; x <= 9; ++x)
        if (x != 6)
            wv.setMask(x, 3, Alg::WorldView::WALL);
    // enemy
    wv.setMask(10, 3, Alg::WorldView::ENEMY);
    // friends (two tanks)
    wv.setMask(1, 3, Alg::WorldView::FRIEND); // Tank A position
    wv.setMask(2, 3, Alg::WorldView::FRIEND); // Tank B position

    Alg::TeamState team;
    team.ensure(wv.w, wv.h);

    // ---- Tank A plans (first to ask) ----
    Alg::Cell a_pos{1, 3};
    Alg::Cell a_tgt = nearestEnemy(wv, a_pos);
    Alg::Cell a_hop = firstHop(wv, a_pos, a_tgt);
    bool a_ok = team.reserveMove(a_hop.x, a_hop.y, /*tank_id=*/0, /*ttl=*/5);
    assert(a_ok);

    // Decision for A: assume facing east; should either shoot (if first hit is enemy) or advance.
    Alg::TankLocal meA{/*player_idx*/ 1, /*tank_idx*/ 0, /*x*/ a_pos.x, /*y*/ a_pos.y, /*deg*/ 0};
    Alg::BattleInfoLite biA{};
    biA.waypoint = a_hop;

    auto actA = Alg::DecisionRunner::decide(biA, wv, team, meA);
    std::cout << "Tank A action: " << (int)actA << "\n";
    // We’re behind a wall line with gap at x=6; likely alignment/MoveForward
    assert(actA == ::ActionRequest::RotateRight45 || actA == ::ActionRequest::RotateLeft45 || actA == ::ActionRequest::MoveForward || actA == ::ActionRequest::Shoot);

    // ---- Tank B plans (second to ask) ----
    team.age(); // emulate on-demand aging between requests
    Alg::Cell b_pos{2, 3};
    Alg::Cell b_tgt = nearestEnemy(wv, b_pos);
    Alg::Cell b_hop = firstHop(wv, b_pos, b_tgt);

    // If B wants the same hop as A, TeamState should make B avoid stepping there now.
    int by = -1;
    bool reserved = team.isMoveReserved(b_hop.x, b_hop.y, by);

    Alg::TankLocal meB{/*player_idx*/ 1, /*tank_idx*/ 1, /*x*/ b_pos.x, /*y*/ b_pos.y, /*deg*/ 0};
    Alg::BattleInfoLite biB{};
    biB.waypoint = b_hop;

    auto actB = Alg::DecisionRunner::decide(biB, wv, team, meB);
    std::cout << "Tank B action: " << (int)actB << " (reserved=" << reserved << ", by=" << by << ")\n";

    if (reserved && by == 0)
    {
        // Then B must not MoveForward into that cell now; rotate fallback is expected.
        assert(actB != ::ActionRequest::MoveForward);
    }

    std::cout << "Player smoke: OK\n";
    return 0;
}
