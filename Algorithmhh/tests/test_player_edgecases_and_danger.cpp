#include <cassert>
#include <iostream>
#include "Player_212788293_212497127.h"
#include "test_fakes.h"

using namespace Algorithm_212788293_212497127;

static void run_case(const std::vector<std::string> &grid, const char *label,
                     std::function<void(const FakeTankAlgorithm &)> check)
{
    FakeSatelliteView sat(grid);
    Player_212788293_212497127 player(1, sat.W, sat.H, 200, 5);
    FakeTankAlgorithm tank(1, 0);
    player.updateTankWithBattleInfo(tank, sat);
    assert(tank.got_update);
    check(tank);
    std::cout << label << ": OK\n";
}

int main()
{
    // 1) Surrounded by walls → must stay in place (fallback)
    run_case({
                 "#####",
                 "#%###",
                 "#####",
             },
             "surrounded-by-walls", [&](const FakeTankAlgorithm &t)
             { assert(t.last_waypoint.x == 1 && t.last_waypoint.y == 1); });

    // 2) Surrounded by mines → also treated as blocked; stay
    run_case({
                 "@@@@@",
                 "@%@@@",
                 "@@@@@",
             },
             "surrounded-by-mines", [&](const FakeTankAlgorithm &t)
             { assert(t.last_waypoint.x == 1 && t.last_waypoint.y == 1); });

    // 3) Shell advancing toward tank (direction-unknown projection) → step away, not into projected lane
    run_case({
                 "........",
                 ".%......", // me at (1,1)
                 "..*.....", // shell at (2,1) → projections at distance 2/4 in all 8 dirs
                 "........",
             },
             "shell-projection", [&](const FakeTankAlgorithm &t)
             {
        // Don’t step into (3,1) or (4,1) immediately if planning sees it as risky.
        // We only enforce: waypoint != (3,1) to reflect avoidance of the nearest projected lane.
        assert(!(t.last_waypoint.x == 3 && t.last_waypoint.y == 1)); });

    // 4) Two corridors: left safe, right dangerous via enemy LOS → pick left-leaning first hop
    run_case({
                 "############",
                 "#%....#...E#", // left corridor (columns 2..5), right corridor (8..10) but enemy at right
                 "#.....#....#",
                 "############",
             },
             "danger-vs-path", [&](const FakeTankAlgorithm &t)
             {
        // First hop should move east (toward column 2/3) but not toward the right corridor’s mouth near enemy.
        // Accept anything that stays inside the left corridor mouth: (2,1) or (2,2).
        const bool leftmouth =
            (t.last_waypoint.x == 2 && (t.last_waypoint.y == 1 || t.last_waypoint.y == 2));
        assert(leftmouth); });

    std::cout << "player edgecases & danger tendency: OK\n";
    return 0;
}
