#include <cassert>
#include <iostream>
#include <cstring>
#include "fake_satellite.h"

#include "Player_212788293_212497127.h"
#include "BattleInfoLite.h"
#include "common/TankAlgorithm.h"

using namespace Algorithm_212788293_212497127;

// Simple sink tank that stores the last BI it received (so we can inspect it)
struct MockTank : public ::TankAlgorithm
{
    BattleInfoLite last{};
    bool got{false};
    void updateBattleInfo(::BattleInfo &info) override
    {
        auto *p = dynamic_cast<BattleInfoLite *>(&info);
        assert(p != nullptr);
        last = *p;
        got = true;
    }
    ::ActionRequest getAction() override { return ::ActionRequest::DoNothing; }
};

int main()
{
    const std::size_t W = 12, H = 7;
    FakeSatelliteView sat(W, H);
    sat.clear('.');

    // Corridor on row y=3. Our team is '1'. Enemy at far east to induce eastward paths.
    sat.set(9, 3, '2');

    // Two friend tanks A, B in a line: B behind A.
    const std::size_t Ax = 2, Ay = 3;
    const std::size_t Bx = 1, By = 3;
    sat.set(Ax, Ay, '1');
    sat.set(Bx, By, '1');

    Player_212788293_212497127 player(/*player_idx=*/1, W, H, /*max_steps=*/200, /*num_shells=*/10);

    MockTank tankA;
    MockTank tankB;

    // --- Update A first (as if GM served A first)
    sat.setRequestingTank(Ax, Ay);
    player.updateTankWithBattleInfo(tankA, sat);
    assert(tankA.got && tankA.last.plan_len >= 1);

    // A's first hop destination in the script (if path exists)
    int Adx = tankA.last.plan_dx[0], Ady = tankA.last.plan_dy[0];

    // --- Update B second (same snapshot, GM serves B right after A)
    sat.setRequestingTank(Bx, By);
    player.updateTankWithBattleInfo(tankB, sat);
    assert(tankB.got && "Player should produce BI for B");

    // This is a smoke test: with reservations, B should NOT try to step into A's immediate next cell
    // when an alternative exists. We check that B's first step is not 'into A'.
    // Compute A's immediate next cell from Adx/Ady:
    std::size_t A_next_x = (Ax + (Adx > 0) - (Adx < 0) + W) % W;
    std::size_t A_next_y = (Ay + (Ady > 0) - (Ady < 0) + H) % H;

    // Reconstruct B's intended next cell:
    int Bdx = (tankB.last.plan_len > 0 ? tankB.last.plan_dx[0] : 0);
    int Bdy = (tankB.last.plan_len > 0 ? tankB.last.plan_dy[0] : 0);
    std::size_t B_next_x = (Bx + (Bdx > 0) - (Bdx < 0) + W) % W;
    std::size_t B_next_y = (By + (Bdy > 0) - (Bdy < 0) + H) % H;

    // Expect that B does not pick A's reserved cell when a corridor is open (may still share the lane later).
    bool avoids_reserved = !(B_next_x == A_next_x && B_next_y == A_next_y);
    assert(avoids_reserved && "B should avoid A's immediately reserved cell");

    std::cout << "teamstate-reservation: A_next=(" << A_next_x << "," << A_next_y << ") "
              << "B_next=(" << B_next_x << "," << B_next_y << ") OK\n";
    return 0;
}
