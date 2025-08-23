#include "Pathfinding.h"
#include "WorldView.h"
#include <queue>
#include <limits>
#include <cstdint>
#include <vector>
#include "WorldViewDebug.h"
#include <iostream>

namespace Algorithm_212788293_212497127
{

    namespace
    {
        using Cost = std::uint32_t;

        inline std::size_t index2D(std::size_t x, std::size_t y, std::size_t W)
        {
            return y * W + x;
        }

        // Fixed neighbor order for determinism: Right, Left, Down, Up
        static const int DX[4] = {+1, -1, 0, 0};
        static const int DY[4] = {0, 0, +1, -1};

        struct Node
        {
            std::size_t x, y;
            Cost f, g, h;
            std::uint64_t seq; // insertion order for final tie-break
        };

        // Strict, deterministic tie-break: (f, g, h, y, x, seq)
        struct Cmp
        {
            bool operator()(const Node &a, const Node &b) const
            {
                if (a.f != b.f)
                    return a.f > b.f;
                if (a.g != b.g)
                    return a.g > b.g;
                if (a.h != b.h)
                    return a.h > b.h;
                if (a.y != b.y)
                    return a.y > b.y;
                if (a.x != b.x)
                    return a.x > b.x;
                return a.seq > b.seq;
            }
        };
    } // namespace

    std::vector<Cell> aStar(const WorldView &wv, Cell from, Cell to)
    {
        const std::size_t W = wv.w, H = wv.h;
        std::vector<Cell> empty{};
        std::cout << "A* from (" << from.x << "," << from.y << ") to (" << to.x << "," << to.y << ")\n";
        // printWorldView(wv, std::cout, 'F', 'E');
        if (W == 0 || H == 0)
            return empty;
        if (from.x >= W || from.y >= H || to.x >= W || to.y >= H)
            return empty;

        // Trivial case: start == goal
        if (from.x == to.x && from.y == to.y)
            return {from};

        // Heuristic: toroidal Manhattan (admissible for 4-connected, unit base step)
        auto H_toroid = [&](std::size_t x, std::size_t y) -> Cost
        {
            return static_cast<Cost>(wv.toroidalDx(x, to.x) + wv.toroidalDy(y, to.y));
        };

        // Open set
        std::priority_queue<Node, std::vector<Node>, Cmp> open;

        // Best g and parent
        const std::size_t N = W * H;
        std::vector<Cost> best_g(N, std::numeric_limits<Cost>::max());
        std::vector<std::size_t> parent(N, std::numeric_limits<std::size_t>::max());

        // Init
        const std::size_t start_i = index2D(from.x, from.y, W);
        const std::size_t goal_i = index2D(to.x, to.y, W);

        std::uint64_t seq = 0;
        const Cost g0 = 0;
        const Cost h0 = H_toroid(from.x, from.y);
        best_g[start_i] = g0;
        open.push(Node{from.x, from.y, /*f=*/g0 + h0, g0, h0, seq++});

        // Deterministic integer step base; add danger if seeded (0 by default)
        const Cost STEP_BASE = 1;

        // Main loop
        while (!open.empty())
        {
            Node cur = open.top();
            open.pop();
            const std::size_t ci = index2D(cur.x, cur.y, W);

            // Skip stale entries
            if (cur.g != best_g[ci])
                continue;

            // Goal?
            if (ci == goal_i)
            {
                // Reconstruct path (goal -> start)
                std::vector<Cell> rev;
                rev.reserve(64);
                std::size_t it = goal_i;
                while (true)
                {
                    const std::size_t yy = it / W;
                    const std::size_t xx = it % W;
                    rev.push_back(Cell{xx, yy});
                    if (it == start_i)
                        break;
                    it = parent[it];
                }
                // Reverse to (start .. goal)
                return std::vector<Cell>(rev.rbegin(), rev.rend());
            }

            // Expand neighbors in fixed order
            for (int k = 0; k < 4; ++k)
            {
                const std::size_t nx = wv.wrapX(static_cast<long long>(cur.x) + DX[k]);
                const std::size_t ny = wv.wrapY(static_cast<long long>(cur.y) + DY[k]);

                if (wv.isBlocked(nx, ny))
                    continue; // includes walls & mines

                // Integer, deterministic step cost (danger defaults to 0 if unused)
                const Cost step_cost = STEP_BASE + static_cast<Cost>(wv.getDanger(nx, ny));
                const Cost ng = cur.g + step_cost;

                const std::size_t ni = index2D(nx, ny, W);
                if (ng < best_g[ni])
                { // strict improvement only
                    best_g[ni] = ng;
                    parent[ni] = ci;
                    const Cost hh = H_toroid(nx, ny);
                    open.push(Node{nx, ny, /*f=*/ng + hh, ng, hh, seq++});
                }
            }
        }

        // No path found
        return empty;
    }

} // namespace Algorithm_212788293_212497127
