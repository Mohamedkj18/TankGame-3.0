#include <iostream>
#include <vector>
#include <cstdint>
#include <cassert>

#include "WorldView.h"
#include "WorldViewDebug.h"
#include "Pathfinding.h"
#include "Types.h"

using namespace Algorithm_212788293_212497127;

// ---- Helpers ---------------------------------------------------------------

static uint64_t checksum(const std::vector<Cell> &path)
{
    uint64_t h = 1469598103934665603ull;
    for (auto c : path)
    {
        h ^= (static_cast<uint64_t>(c.x) << 32) ^ static_cast<uint64_t>(c.y);
        h *= 1099511628211ull;
    }
    return h;
}

static bool verifyAdjacencyToroid(const WorldView &wv, const std::vector<Cell> &p)
{
    if (p.empty())
        return true;
    for (size_t i = 1; i < p.size(); ++i)
    {
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
        if (wv.isBlocked(c.x, c.y))
            return false;
    return true;
}

// Add integer danger along a straight ray until a wall is hit (toroidal aware).
static void addDangerRay(WorldView &wv, std::size_t x, std::size_t y, int dx, int dy, std::uint32_t k)
{
    std::size_t W = wv.w, H = wv.h;
    std::size_t cx = x, cy = y;
    for (std::size_t steps = 0; steps < std::max(W, H); ++steps)
    {
        cx = wv.wrapX(static_cast<long long>(cx) + dx);
        cy = wv.wrapY(static_cast<long long>(cy) + dy);
        if (wv.isWall(cx, cy))
            break; // LOS blocked by wall
        wv.addDanger(cx, cy, k);
    }
}

// Seed enemy line-of-sight danger (4 dirs) and shell danger (on current cell).
static void seedDanger(WorldView &wv, std::uint32_t enemyLOS = 8, std::uint32_t shellK = 20)
{
    for (std::size_t y = 0; y < wv.h; ++y)
    {
        for (std::size_t x = 0; x < wv.w; ++x)
        {
            if (wv.hasEnemy(x, y))
            {
                addDangerRay(wv, x, y, +1, 0, enemyLOS);
                addDangerRay(wv, x, y, -1, 0, enemyLOS);
                addDangerRay(wv, x, y, 0, +1, enemyLOS);
                addDangerRay(wv, x, y, 0, -1, enemyLOS);
            }
            if (wv.hasShell(x, y))
            {
                wv.addDanger(x, y, shellK); // heavy penalty on shell cell
            }
        }
    }
}

// Treat friendly-occupied cells as blocked for pathfinding by adding MINE bit.
// (We still keep the FRIEND bit so the printer shows 'F' instead of '@'.)
static void blockFriendCells(WorldView &wv)
{
    for (std::size_t y = 0; y < wv.h; ++y)
        for (std::size_t x = 0; x < wv.w; ++x)
            if (wv.hasFriend(x, y))
                wv.setMask(x, y, WorldView::MINE);
}

// ---- Test scenario ---------------------------------------------------------

int main()
{
    const std::size_t W = 16, H = 10;
    WorldView wv{W, H};

    // Walls: two barriers with gaps
    for (std::size_t x = 2; x <= 13; ++x)
        if (x != 8)
            wv.setMask(x, 5, WorldView::WALL);
    for (std::size_t x = 1; x <= 14; ++x)
        if (x != 12)
            wv.setMask(x, 3, WorldView::WALL);

    // Mines: a few to force detours
    wv.setMask(4, 1, WorldView::MINE);
    wv.setMask(5, 1, WorldView::MINE);
    wv.setMask(6, 1, WorldView::MINE);
    wv.setMask(10, 8, WorldView::MINE);

    // Friendly tanks: block their cells, but show as 'F'
    wv.setMask(1, 1, WorldView::FRIEND);
    wv.setMask(7, 4, WorldView::FRIEND);
    wv.setMask(9, 6, WorldView::FRIEND);
    blockFriendCells(wv);

    // Enemy tanks: create LOS danger lanes
    wv.setMask(14, 2, WorldView::ENEMY);
    wv.setMask(3, 8, WorldView::ENEMY);

    // Shells: penalize stepping onto current shell cells
    wv.setMask(12, 7, WorldView::SHELL);
    wv.setMask(6, 9, WorldView::SHELL);

    // Seed integer danger (no floats): enemy LOS + shell cells
    seedDanger(wv, /*enemyLOS=*/8, /*shellK=*/20);

    // Start/Goal
    Cell start{0, 0};
    Cell goal{15, 9};

    // Pathfind
    auto path = aStar(wv, start, goal);

    // Print world + path overlay
    std::cout << "\n=== WorldView (entities + danger) ===\n";
    printWorldViewWithPath(wv, path, std::cout, /*friendCh=*/'F', /*enemyCh=*/'E');

    // Validations
    if (path.empty())
    {
        std::cerr << "ERROR: No path found\n";
        return 1;
    }
    if (!verifyAdjacencyToroid(wv, path))
    {
        std::cerr << "ERROR: Non-adjacent steps (toroidal check failed)\n";
        return 1;
    }
    if (!verifyNoBlocked(wv, path))
    {
        std::cerr << "ERROR: Path stepped on a blocked cell (wall/mine/friend)\n";
        return 1;
    }

    // Determinism: repeat and compare checksums
    const int N = 8;
    const auto h1 = checksum(path);
    for (int i = 0; i < N; ++i)
    {
        auto p2 = aStar(wv, start, goal);
        if (checksum(p2) != h1 || p2.size() != path.size())
        {
            std::cerr << "ERROR: Non-deterministic path on repeat " << i << "\n";
            return 1;
        }
    }
    std::cout << "Determinism: OK (" << N << " identical runs)\n";

    // Report rough danger interaction
    bool touched_shell = false, crossed_enemy_lane = false;
    for (auto c : path)
    {
        if (wv.hasShell(c.x, c.y))
            touched_shell = true;
        if (wv.getDanger(c.x, c.y) > 0)
            crossed_enemy_lane = true;
    }
    std::cout << "Touched shell cell? " << (touched_shell ? "yes" : "no") << "\n";
    std::cout << "Crossed enemy-LOS lane (danger>0)? " << (crossed_enemy_lane ? "yes" : "no") << "\n";

    return 0;
}
