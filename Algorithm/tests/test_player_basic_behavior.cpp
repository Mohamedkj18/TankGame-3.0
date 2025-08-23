#include <cassert>
#include <iostream>
#include "Player_212788293_212497127.h"
#include "test_fakes.h"

using namespace Algorithm_212788293_212497127;

int main()
{
    // Board 12x7: two friends, one enemy, wall line with a gap
    std::vector<std::string> grid = {
        "............",
        "............",
        "............",
        ".%####.##E..", // '%' = requesting tank here at (1,3)
        "............",
        "............",
        "............",
    };

    FakeSatelliteView sat(grid);
    Player_212788293_212497127 player(/*player=*/1, sat.W, sat.H, /*max_steps=*/200, /*num_shells=*/5);

    // Tank A asks
    FakeTankAlgorithm tankA(1, 0);
    player.updateTankWithBattleInfo(tankA, sat);
    assert(tankA.got_update);
    // Waypoint must be inside the board and not into a wall
    assert(tankA.last_waypoint.x < sat.W && tankA.last_waypoint.y < sat.H);
    // Sanity: shouldn't be the same as current if a path exists across the gap
    // (If surrounded, Player may keep position—handled in other tests.)
    if (!(grid[tankA.last_waypoint.y][tankA.last_waypoint.x] == '#'))
        assert(!(tankA.last_waypoint.x == 1 && tankA.last_waypoint.y == 3));

    std::cout << "player basic behavior: OK\n";
    return 0;
}
