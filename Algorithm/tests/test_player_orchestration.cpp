#include <cassert>
#include <iostream>
#include "Player_212788293_212497127.h"
#include "test_fakes.h"

using namespace Algorithm_212788293_212497127;

int main()
{
    // Two friendly tanks intend to pass a choke; enemy ahead; shell placed to bias lanes.
    std::vector<std::string> grid = {
        "............",
        "............",
        "............",
        ".%####.##E..", // first requester at (1,3)
        "............",
        "............",
        "............",
    };

    FakeSatelliteView satA(grid); // For tank A (marks '%' at (1,3))
    Player_212788293_212497127 player(1, satA.W, satA.H, 200, 5);

    FakeTankAlgorithm tankA(1, 0);
    player.updateTankWithBattleInfo(tankA, satA);
    assert(tankA.got_update);

    // Now a second tank B starts next to A at (2,3). Make its satellite mark it as '%'
    auto gridB = grid;
    gridB[3][1] = '1'; // A becomes regular friend
    gridB[3][2] = '%'; // B is the requester
    // Put a shell near the choke to force danger-aware divergence
    gridB[2][6] = '*'; // above the gap at x=6
    FakeSatelliteView satB(gridB);

    FakeTankAlgorithm tankB(1, 1);
    player.updateTankWithBattleInfo(tankB, satB);
    assert(tankB.got_update);

    // Core orchestration check: B should not choose A's reserved hop
    // (We can’t access Player’s TeamState directly, but we ensured Player penalizes reserved cells.)
    assert(!(tankB.last_waypoint.x == tankA.last_waypoint.x &&
             tankB.last_waypoint.y == tankA.last_waypoint.y));

    std::cout << "player orchestration: OK\n";
    return 0;
}
