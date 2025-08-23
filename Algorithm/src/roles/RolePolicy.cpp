#include "roles/RolePolicy.h"
#include "WorldView.h"
#include <limits>
#include <cstdint>

namespace Algorithm_212788293_212497127
{

    // helpers
    static inline std::size_t manhT(const WorldView &wv, Cell a, Cell b)
    {
        return wv.manhattanToroidal(a, b);
    }

    // deterministic argmin over PASSABLE cells; tie-break (y,x)
    template <typename ScoreFn>
    static Cell argmin_passable(const WorldView &wv, ScoreFn score)
    {
        Cell best{0, 0};
        bool has = false;
        std::uint64_t bestS = std::numeric_limits<std::uint64_t>::max();
        for (std::size_t y = 0; y < wv.h; ++y)
        {
            for (std::size_t x = 0; x < wv.w; ++x)
            {
                if (wv.isBlocked(x, y))
                    continue; // walls/mines blocked
                const std::uint64_t s = score(x, y);
                if (!has || s < bestS || (s == bestS && (y < best.y || (y == best.y && x < best.x))))
                {
                    has = true;
                    bestS = s;
                    best = Cell{x, y};
                }
            }
        }
        return has ? best : Cell{0, 0};
    }

    // deterministic argmax over PASSABLE cells; tie-break (y,x)
    template <typename ScoreFn>
    static Cell argmax_passable(const WorldView &wv, ScoreFn score)
    {
        Cell best{0, 0};
        bool has = false;
        std::int64_t bestS = std::numeric_limits<std::int64_t>::min();
        for (std::size_t y = 0; y < wv.h; ++y)
        {
            for (std::size_t x = 0; x < wv.w; ++x)
            {
                if (wv.isBlocked(x, y))
                    continue;
                const std::int64_t s = score(x, y);
                if (!has || s > bestS || (s == bestS && (y < best.y || (y == best.y && x < best.x))))
                {
                    has = true;
                    bestS = s;
                    best = Cell{x, y};
                }
            }
        }
        return has ? best : Cell{0, 0};
    }

    // -------- Aggressor --------
    // Choose a PASSABLE approach tile near the nearest enemy (not the enemy cell).
    // Minimize: 8*dist_to_enemy + 4*danger + 1*dist_from_me
    Cell targetForAggressor(const WorldView &wv, Cell me)
    {
        // nearest enemy (for reference)
        std::size_t bestd = std::numeric_limits<std::size_t>::max();
        Cell enemy{0, 0};
        bool seen = false;
        for (std::size_t y = 0; y < wv.h; ++y)
            for (std::size_t x = 0; x < wv.w; ++x)
                if (wv.hasEnemy(x, y))
                {
                    const auto d = manhT(wv, me, Cell{x, y});
                    if (!seen || d < bestd || (d == bestd && (y < enemy.y || (y == enemy.y && x < enemy.x))))
                    {
                        seen = true;
                        bestd = d;
                        enemy = Cell{x, y};
                    }
                }
        if (!seen)
            return targetForAnchor(wv, me); // fallback

        return argmin_passable(wv, [&](std::size_t x, std::size_t y) -> std::uint64_t
                               {
        if (x==enemy.x && y==enemy.y) return std::numeric_limits<std::uint64_t>::max()/2ull;
        const auto dE = manhT(wv, Cell{x,y}, enemy);
        const auto dM = manhT(wv, me, Cell{x,y});
        const auto dang = static_cast<std::uint64_t>(wv.getDanger(x,y));
        return 8ull*dE + 4ull*dang + 1ull*dM; });
    }

    // -------- Anchor --------
    // Hold central control: pick passable cell near map center, low danger.
    Cell targetForAnchor(const WorldView &wv, Cell /*me*/)
    {
        const Cell center{wv.w / 2, wv.h / 2};
        return argmin_passable(wv, [&](std::size_t x, std::size_t y) -> std::uint64_t
                               {
        const auto dC = manhT(wv, Cell{x,y}, center);
        const auto dang = static_cast<std::uint64_t>(wv.getDanger(x,y));
        return 1ull*dC + 1ull*dang; });
    }

    // -------- Flanker --------
    // Seek side/back tiles around nearest enemy (ring distance 1–2), prefer low danger.
    // Minimize: (ring_mismatch*heavy) + 4*danger + 1*dist_from_me
    Cell targetForFlanker(const WorldView &wv, Cell me)
    {
        // find nearest enemy
        std::size_t bestd = std::numeric_limits<std::size_t>::max();
        Cell e{0, 0};
        bool seen = false;
        for (std::size_t y = 0; y < wv.h; ++y)
            for (std::size_t x = 0; x < wv.w; ++x)
                if (wv.hasEnemy(x, y))
                {
                    const auto d = manhT(wv, me, Cell{x, y});
                    if (!seen || d < bestd || (d == bestd && (y < e.y || (y == e.y && x < e.x))))
                    {
                        seen = true;
                        bestd = d;
                        e = Cell{x, y};
                    }
                }
        if (!seen)
            return targetForAnchor(wv, me);

        // fixed candidate ring deltas (radius 1 & 2), deterministic order
        static const int DX[] = {+1, -1, 0, 0, +2, -2, 0, 0, +1, +1, -1, -1};
        static const int DY[] = {0, 0, +1, -1, 0, 0, +2, -2, +1, -1, +1, -1};

        Cell best{me};
        bool has = false;
        std::uint64_t bestS = std::numeric_limits<std::uint64_t>::max();
        for (int i = 0; i < static_cast<int>(sizeof(DX) / sizeof(DX[0])); ++i)
        {
            const std::size_t cx = wv.wrapX(static_cast<long long>(e.x) + DX[i]);
            const std::size_t cy = wv.wrapY(static_cast<long long>(e.y) + DY[i]);
            if (wv.isBlocked(cx, cy))
                continue;
            const auto dE = manhT(wv, Cell{cx, cy}, e);
            const auto dM = manhT(wv, me, Cell{cx, cy});
            const auto dang = static_cast<std::uint64_t>(wv.getDanger(cx, cy));
            const std::uint64_t ring_penalty = (dE == 1 || dE == 2) ? 0ull : (dE > 2 ? (dE - 2) : (1 - dE)) * 8ull;
            const std::uint64_t s = ring_penalty + 4ull * dang + 1ull * dM;
            if (!has || s < bestS || (s == bestS && (cy < best.y || (cy == best.y && cx < best.x))))
            {
                has = true;
                bestS = s;
                best = Cell{cx, cy};
            }
        }
        if (has)
            return best;

        // fallback near enemy with danger penalty
        return argmin_passable(wv, [&](std::size_t x, std::size_t y) -> std::uint64_t
                               {
        if (x==e.x && y==e.y) return std::numeric_limits<std::uint64_t>::max()/2ull;
        return 8ull*manhT(wv, Cell{x,y}, e) + 4ull*static_cast<std::uint64_t>(wv.getDanger(x,y)) + manhT(wv, me, Cell{x,y}); });
    }

    // -------- Survivor --------
    // Maximize safety: far from enemies, low danger, mildly prefer closer to me.
    // Maximize: 8*nearest_enemy_dist − 4*danger − 1*dist_from_me
    Cell targetForSurvivor(const WorldView &wv, Cell me)
    {
        return argmax_passable(wv, [&](std::size_t x, std::size_t y) -> std::int64_t
                               {
        std::size_t dE = 0;
        bool any=false;
        std::size_t best = std::numeric_limits<std::size_t>::max();
        for (std::size_t yy=0; yy<wv.h; ++yy)
            for (std::size_t xx=0; xx<wv.w; ++xx)
                if (wv.hasEnemy(xx,yy)) {
                    any=true;
                    const auto d = manhT(wv, Cell{x,y}, Cell{xx,yy});
                    if (d < best) best = d;
                }
        dE = any ? best : 0;
        const std::int64_t dang = static_cast<std::int64_t>(wv.getDanger(x,y));
        const std::int64_t dM   = static_cast<std::int64_t>(manhT(wv, me, Cell{x,y}));
        return 8*dE - 4*dang - dM; });
    }

} // namespace Algorithm_212788293_212497127
