#include <iostream>
#include <vector>
#include <cstdint>
#include <cstdlib>
#include "WorldView.h"
#include "Pathfinding.h"
#include "Types.h"

using namespace Algorithm_212788293_212497127;

static uint64_t checksum(const std::vector<Cell> &path)
{
    // simple order-sensitive hash (no randomness)
    uint64_t h = 1469598103934665603ull; // FNV offset
    for (auto c : path)
    {
        h ^= (static_cast<uint64_t>(c.x) << 32) ^ static_cast<uint64_t>(c.y);
        h *= 1099511628211ull; // FNV prime
    }
    return h;
}

static bool verifyAdjacencyToroid(const WorldView &wv, const std::vector<Cell> &p)
{
    if (p.empty())
        return true;
    for (size_t i = 1; i < p.size(); ++i)
    {
        // On a 4-connected torus, successive cells must be Manhattan distance 1 (with wrap)
        const auto dx = wv.toroidalDx(p[i - 1].x, p[i].x);
        const auto dy = wv.toroidalDy(p[i - 1].y, p[i].y);
        if (dx + dy != 1)
            return false;
    }
    return true;
}

static bool verifyNoBlocked(const WorldView &wv, const std::vector<Cell> &p)
{
    for (auto c : p)
    {
        if (wv.isBlocked(c.x, c.y))
            return false;
    }
    return true;
}

int main()
{
    const std::size_t W = 14, H = 8;
    WorldView wv{W, H};

    // Walls: a horizontal barrier on y=4 with a gap at x=10 (forces detour)
    for (std::size_t x = 1; x < W - 2; ++x)
    {
        if (x == 10)
            continue; // gap
        wv.setMask(x, 4, WorldView::WALL);
    }

    // Mines: sprinkle a few along the top and bottom edges
    wv.setMask(3, 0, WorldView::MINE);
    wv.setMask(4, 0, WorldView::MINE);
    wv.setMask(11, 7, WorldView::MINE);

    // Danger band: make row y=2 "costly" so A* prefers another lane if similar length
    for (std::size_t x = 0; x < W; ++x)
        wv.addDanger(x, 2, 5);

    // Start/Goal chosen so toroidal wrap can help if barrier blocks the middle
    Cell start{0, 0};
    Cell goal{12, 6};

    // Single run
    auto path1 = aStar(wv, start, goal);

    std::cout << "W=" << W << " H=" << H << "\n";
    std::cout << "Start: (" << start.x << "," << start.y << ")  "
              << "Goal: (" << goal.x << "," << goal.y << ")\n";
    std::cout << "Path length: " << path1.size() << "\n";
    for (std::size_t i = 0; i < path1.size(); ++i)
    {
        std::cout << i << ": (" << path1[i].x << "," << path1[i].y << ")\n";
    }

    if (path1.empty())
    {
        std::cerr << "ERROR: No path found\n";
        return 1;
    }
    if (!verifyAdjacencyToroid(wv, path1))
    {
        std::cerr << "ERROR: Non-adjacent steps found (toroidal check failed)\n";
        return 1;
    }
    if (!verifyNoBlocked(wv, path1))
    {
        std::cerr << "ERROR: Path stepped on a blocked (WALL/MINE) cell\n";
        return 1;
    }

    // Determinism test: run N times and compare checksum
    const int N = 8;
    const auto h1 = checksum(path1);
    for (int i = 0; i < N; ++i)
    {
        auto p = aStar(wv, start, goal);
        if (checksum(p) != h1 || p.size() != path1.size())
        {
            std::cerr << "ERROR: Non-deterministic path detected on run " << i << "\n";
            return 1;
        }
    }
    std::cout << "Determinism: OK (" << N << " repeat runs identical)\n";

    // Extra: ensure the path avoids the high-danger row if an equivalent-length route exists
    bool used_danger_row = false;
    for (auto c : path1)
        if (c.y == 2)
        {
            used_danger_row = true;
            break;
        }
    std::cout << "Used danger row y=2? " << (used_danger_row ? "yes" : "no") << "\n";

    return 0;
}
