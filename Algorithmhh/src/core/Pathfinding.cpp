#include "Pathfinding.h"
#include "WorldView.h"
#include <queue>
#include <limits>
#include <cstdint>
#include <vector>
#include "WorldViewDebug.h"

namespace Algorithm_212788293_212497127
{

    namespace
    {
        using Cost = std::uint32_t;

        inline std::size_t index2D(std::size_t x, std::size_t y, std::size_t W)
        {
            return y * W + x;
        }

        // 8-direction neighbor set in a fixed, deterministic order:
        // E, NE, N, NW, W, SW, S, SE  (must match DIR8 used elsewhere)
        static const int DX[8] = {+1, +1, 0, -1, -1, -1, 0, +1};
        static const int DY[8] = {0, +1, +1, +1, 0, -1, -1, -1};

        // Integer step costs (scaled by 10 to keep integers):
        // Card = 10, Diag = 14  (≈ sqrt(2)*10)
        static const Cost STEP_COST[8] = {
            10, 14, 10, 14, 10, 14, 10, 14};

        struct Node
        {
            std::size_t x, y;
            Cost f, g, h;
            std::uint64_t seq; // insertion order for final tie-break
            std::uint8_t dir;  // which neighbor index led here (for deterministic tie-breaks)
        };

        // Strict, deterministic tie-break: (f, g, h, y, x, dir, seq)
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
                if (a.dir != b.dir)
                    return a.dir > b.dir;
                return a.seq > b.seq;
            }
        };

        // Minimal toroidal deltas on each axis
        inline std::size_t toroidalDx(const WorldView &wv, std::size_t x, std::size_t tx)
        {
            const std::size_t W = wv.w;
            return (x <= tx) ? std::min(tx - x, W - (tx - x))
                             : std::min(x - tx, W - (x - tx));
        }
        inline std::size_t toroidalDy(const WorldView &wv, std::size_t y, std::size_t ty)
        {
            const std::size_t H = wv.h;
            return (y <= ty) ? std::min(ty - y, H - (ty - y))
                             : std::min(y - ty, H - (y - ty));
        }

        // Octile heuristic on a torus, scaled for (card=10, diag=14):
        // h = 10*(dx+dy) - 6*min(dx,dy)
        inline Cost H_octile_toroid(const WorldView &wv, std::size_t x, std::size_t y,
                                    std::size_t tx, std::size_t ty)
        {
            std::size_t dx = toroidalDx(wv, x, tx);
            std::size_t dy = toroidalDy(wv, y, ty);
            std::size_t mn = (dx < dy) ? dx : dy;
            return static_cast<Cost>(10 * (dx + dy) - 6 * mn);
        }

    } // namespace

    std::vector<Cell> aStar(const WorldView &wv, Cell from, Cell to)
    {
        const std::size_t W = wv.w, H = wv.h;
        std::vector<Cell> empty{};
        if (W == 0 || H == 0)
            return empty;
        if (from.x >= W || from.y >= H || to.x >= W || to.y >= H)
            return empty;

        // Trivial case: start == goal
        if (from.x == to.x && from.y == to.y)
            return {from};

        // Open set
        std::priority_queue<Node, std::vector<Node>, Cmp> open;

        // Best g and parent
        const std::size_t N = W * H;
        std::vector<Cost> best_g(N, std::numeric_limits<Cost>::max());
        std::vector<std::size_t> parent(N, std::numeric_limits<std::size_t>::max());

        const std::size_t start_i = index2D(from.x, from.y, W);
        const std::size_t goal_i = index2D(to.x, to.y, W);

        std::uint64_t seq = 0;
        const Cost g0 = 0;
        const Cost h0 = H_octile_toroid(wv, from.x, from.y, to.x, to.y);
        best_g[start_i] = g0;
        open.push(Node{from.x, from.y, /*f=*/g0 + h0, g0, h0, seq++, /*dir*/ 255});

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

            // Expand 8 neighbors in fixed order
            for (std::uint8_t k = 0; k < 8; ++k)
            {
                const std::size_t nx = wv.wrapX(static_cast<long long>(cur.x) + DX[k]);
                const std::size_t ny = wv.wrapY(static_cast<long long>(cur.y) + DY[k]);

                if (wv.isBlocked(nx, ny))
                    continue; // walls & mines only

                // Danger augments step cost; scale by 10 to match step units.
                const Cost danger = static_cast<Cost>(wv.getDanger(nx, ny));
                const Cost step_cost = STEP_COST[k] + danger;

                const Cost ng = cur.g + step_cost;

                const std::size_t ni = index2D(nx, ny, W);
                if (ng < best_g[ni])
                { // strict improvement only
                    best_g[ni] = ng;
                    parent[ni] = ci;
                    const Cost hh = H_octile_toroid(wv, nx, ny, to.x, to.y);
                    open.push(Node{nx, ny, /*f=*/ng + hh, ng, hh, seq++, k});
                }
            }
        }

        // No path found
        return empty;
    }

} // namespace Algorithm_212788293_212497127
