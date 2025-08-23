#include <cassert>
#include <iostream>
#include "fake_satellite.h"

#include "Player_212788293_212497127.h"
#include "TankAlgorithm_212788293_212497127.h"

using namespace Algorithm_212788293_212497127;

int main()
{
    const std::size_t W = 12, H = 7;
    FakeSatelliteView sat(W, H);
    sat.clear('.');

    // One friendly (our tank) at (1,3). One enemy straight east at (9,3).
    // Open corridor, no walls/mines.
    sat.set(1, 3, '1'); // our team id is 1
    sat.set(9, 3, '2'); // enemy

    Player_212788293_212497127 player(/*player_idx=*/1, W, H, /*max_steps=*/200, /*num_shells=*/10);
    TankAlgorithm_212788293_212497127 tank(/*player_idx=*/1, /*tank_idx=*/0);

    // --- Tick 0: tank asks BI (GM would call Player)
    sat.setRequestingTank(1, 3);
    player.updateTankWithBattleInfo(tank, sat);

    // --- Ticks: run getAction repeatedly WITHOUT updating BI again.
    // We should see rotations (if any) then a run of MoveForward driven by the short script.
    int move_count = 0;
    int shoot_count = 0;
    int rotate_count = 0;

    for (int t = 0; t < 6; ++t)
    { // up to script length + a few
        ::ActionRequest ar = tank.getAction();
        if (ar == ::ActionRequest::MoveForward)
            ++move_count;
        else if (ar == ::ActionRequest::Shoot)
            ++shoot_count;
        else if (ar == ::ActionRequest::RotateLeft45 || ar == ::ActionRequest::RotateRight45)
            ++rotate_count;
        else
        {
            // After script is consumed, our Tank returns RotateLeft45 (since we didn't implement GetBattleInfo enum).
            // Treat that as "no BI" marker and break.
            break;
        }
    }

    // Since tank constructor starts facing east in our motor (index 0),
    // first actions should be MoveForward steps (no rotation), and no shots in open corridor.
    assert(move_count >= 1 && move_count <= 4);
    assert(shoot_count == 0);

    std::cout << "integration-flow: moves=" << move_count
              << " rotates=" << rotate_count
              << " shoots=" << shoot_count << " OK\n";
    return 0;
}
