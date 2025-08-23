#include <iostream>
#include <vector>
#include "WorldView.h"
#include "Pathfinding.h"
#include "Types.h"

using namespace Algorithm_212788293_212497127;

int main()
{
    const std::size_t W = 10, H = 6;
    WorldView wv{W, H};

    // Optional: add some walls to force a detour
    // (uncomment the line below if you already added isBlocked() logic)
    // for (std::size_t x = 2; x <= 7; ++x) wv.addWall(x, 3);

    Cell start{0, 0};
    Cell goal{4, 2};

    auto path = aStar(wv, start, goal);

    std::cout << "W=" << W << " H=" << H << "\n";
    std::cout << "Start: (" << start.x << "," << start.y << ")  "
              << "Goal: (" << goal.x << "," << goal.y << ")\n";
    std::cout << "Path length: " << path.size() << "\n";
    for (std::size_t i = 0; i < path.size(); ++i)
    {
        std::cout << i << ": (" << path[i].x << "," << path[i].y << ")\n";
    }
    return 0;
}
