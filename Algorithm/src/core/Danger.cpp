#include "Danger.h"
#include "WorldView.h"
#include <algorithm>
#include <numeric> // gcd/lcm (portable fallback below if needed)
#include <vector>

namespace Algorithm_212788293_212497127
{

    static inline std::size_t lcm_(std::size_t a, std::size_t b)
    {
        auto gcd = [](std::size_t x, std::size_t y)
        {
            while (y)
            {
                auto t = x % y;
                x = y;
                y = t;
            }
            return x;
        };
        return (a / gcd(a, b)) * b;
    }

    static inline std::size_t rayFullCycle(std::size_t W, std::size_t H, int dx, int dy)
    {
        const std::size_t px = (dx == 0) ? 1 : W;
        const std::size_t py = (dy == 0) ? 1 : H;
        return lcm_(px, py) - 1; // distinct steps excluding origin
    }

    static void addRayDecaying(WorldView &wv,
                               std::size_t x, std::size_t y,
                               int dx, int dy,
                               std::uint32_t base, std::uint32_t decay,
                               int max_range)
    {
        if (wv.w == 0 || wv.h == 0 || (dx == 0 && dy == 0) || base == 0)
            return;

        const std::size_t full = rayFullCycle(wv.w, wv.h, dx, dy);
        std::size_t limit = full;
        if (max_range > 0)
            limit = std::min<std::size_t>(limit, static_cast<std::size_t>(max_range));

        std::size_t cx = x, cy = y;
        for (std::size_t s = 1; s <= limit; ++s)
        {
            cx = wv.wrapX(static_cast<long long>(cx) + dx);
            cy = wv.wrapY(static_cast<long long>(cy) + dy);
            if (wv.isWall(cx, cy))
                break;

            std::uint32_t add = base;
            if (decay > 0)
            {
                const std::uint64_t dec = static_cast<std::uint64_t>(decay) * (s - 1);
                add = (dec >= base) ? 0u : static_cast<std::uint32_t>(base - dec);
            }
            if (add == 0)
                break; // further cells would be 0 too
            wv.addDanger(cx, cy, add);
        }
    }

    void seedEnemyLOSDanger(WorldView &wv,
                            std::uint32_t base,
                            std::uint32_t decay,
                            int max_range,
                            bool include_diagonals)
    {
        static const int DIRS8[8][2] = {
            {+1, 0}, {+1, +1}, {0, +1}, {-1, +1}, {-1, 0}, {-1, -1}, {0, -1}, {+1, -1}};
        static const int DIRS4[4][2] = {
            {+1, 0}, {0, +1}, {-1, 0}, {0, -1}};

        const int (*dirs)[2] = include_diagonals ? DIRS8 : DIRS4;

        const int ndirs = include_diagonals ? 8 : 4;

        for (std::size_t y = 0; y < wv.h; ++y)
        {
            for (std::size_t x = 0; x < wv.w; ++x)
            {
                if (!wv.hasEnemy(x, y))
                    continue;
                for (int i = 0; i < ndirs; ++i)
                {
                    addRayDecaying(wv, x, y, dirs[i][0], dirs[i][1], base, decay, max_range);
                }
            }
        }
    }

    // Old immediate seeding kept as inline wrapper
    void seedShellDanger(WorldView &wv, std::uint32_t shellBase)
    {
        for (std::size_t y = 0; y < wv.h; ++y)
            for (std::size_t x = 0; x < wv.w; ++x)
                if (wv.hasShell(x, y))
                    wv.addDanger(x, y, shellBase);
    }

    void seedShellDangerPredictiveUnknown(WorldView &wv,
                                          std::uint32_t base,
                                          std::uint32_t decay,
                                          int horizon,
                                          bool mark_intermediate)
    {
        static const int DIRS8[8][2] = {
            {+1, 0}, {+1, +1}, {0, +1}, {-1, +1}, {-1, 0}, {-1, -1}, {0, -1}, {+1, -1}};

        // Collect shell positions first so we don’t double-count as we add danger.
        std::vector<std::pair<std::size_t, std::size_t>> shells;
        shells.reserve(wv.w * wv.h / 8 + 1);
        for (std::size_t y = 0; y < wv.h; ++y)
            for (std::size_t x = 0; x < wv.w; ++x)
                if (wv.hasShell(x, y))
                    shells.emplace_back(x, y);

        for (auto [sx, sy] : shells)
        {
            for (int t = 1; t <= horizon; ++t)
            {
                const std::uint32_t add = (decay == 0 || base <= decay * static_cast<std::uint32_t>(t - 1))
                                              ? 0u
                                              : static_cast<std::uint32_t>(base - decay * static_cast<std::uint32_t>(t - 1));
                if (add == 0u)
                    break;

                const int stepLen = 2 * t; // 2 cells per game step
                for (const auto &d : DIRS8)
                {
                    // mark the prospective landing cell at distance 2*t
                    std::size_t cx = sx, cy = sy;
                    cx = wv.wrapX(static_cast<long long>(cx) + stepLen * d[0]);
                    cy = wv.wrapY(static_cast<long long>(cy) + stepLen * d[1]);
                    if (!wv.isWall(cx, cy))
                        wv.addDanger(cx, cy, add);

                    if (mark_intermediate)
                    {
                        // also mark the “passing” cell at distance 2*t - 1 (conservative smear)
                        if (stepLen - 1 > 0)
                        {
                            std::size_t px = sx, py = sy;
                            px = wv.wrapX(static_cast<long long>(px) + (stepLen - 1) * d[0]);
                            py = wv.wrapY(static_cast<long long>(py) + (stepLen - 1) * d[1]);
                            if (!wv.isWall(px, py))
                                wv.addDanger(px, py, add);
                        }
                    }
                }
            }
        }
    }

    void seedWallProximityDanger(WorldView &wv, std::uint32_t k, int radius)
    {
        if (radius <= 0)
            return;
        for (std::size_t y = 0; y < wv.h; ++y)
        {
            for (std::size_t x = 0; x < wv.w; ++x)
            {
                if (wv.isWall(x, y))
                    continue;
                bool near = false;
                for (int r = 1; r <= radius && !near; ++r)
                {
                    const std::size_t xR = wv.wrapX(static_cast<long long>(x) + r);
                    const std::size_t xL = wv.wrapX(static_cast<long long>(x) - r);
                    const std::size_t yD = wv.wrapY(static_cast<long long>(y) + r);
                    const std::size_t yU = wv.wrapY(static_cast<long long>(y) - r);
                    if (wv.isWall(xR, y) || wv.isWall(xL, y) ||
                        wv.isWall(x, yD) || wv.isWall(x, yU))
                        near = true;
                }
                if (near)
                    wv.addDanger(x, y, k);
            }
        }
    }

} // namespace Algorithm_212788293_212497127
