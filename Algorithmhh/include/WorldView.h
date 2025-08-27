#ifndef ALG_212788293_212497127_WORLDVIEW_H
#define ALG_212788293_212497127_WORLDVIEW_H

#include <cstddef>
#include <cstdint>
#include <vector>
#include "Types.h"

namespace Algorithm_212788293_212497127
{

    struct WorldView
    {
        // --- Dimensions ---
        std::size_t w{0}, h{0};

        // --- Cell bitmask (occupancy & tags) ---
        // You can set/clear multiple bits on the same cell.
        enum CellMask : std::uint8_t
        {
            EMPTY = 0,
            WALL = 1 << 0,
            MINE = 1 << 1,
            FRIEND = 1 << 2, // any friendly tank currently on the cell
            ENEMY = 1 << 3,  // any enemy tank currently on the cell
            SHELL = 1 << 4,  // shell occupies the cell *now*
            GOAL = 1 << 5,   // utility tag for debugging/path targets
            VISITED = 1 << 6 // utility tag for flood-fills/debug
        };

        // Grids (size w*h). `mask` defaults to EMPTY, `danger` to 0,
        // `shell_eta` to a large number meaning "no shell scheduled".
        std::vector<std::uint8_t> mask;       // CellMask bits
        std::vector<std::uint32_t> danger;    // additive risk/cost per cell
        std::vector<std::uint16_t> shell_eta; // ticks until a shell arrives (0=now). 0xFFFF = none.

        // --- Construction / reset ---
        WorldView() = default;
        explicit WorldView(std::size_t W, std::size_t H) { reset(W, H); }

        void reset(std::size_t W, std::size_t H)
        {
            w = W;
            h = H;
            mask.assign(W * H, EMPTY);
            danger.assign(W * H, 0u);
            shell_eta.assign(W * H, 0xFFFFu);
        }

        // --- Indexing helpers ---
        inline std::size_t idx(std::size_t x, std::size_t y) const { return y * w + x; }
        inline std::size_t wrapX(long long x) const { return static_cast<std::size_t>((x % (long long)w + (long long)w) % (long long)w); }
        inline std::size_t wrapY(long long y) const { return static_cast<std::size_t>((y % (long long)h + (long long)h) % (long long)h); }

        // Toroidal distances for heuristics
        inline std::size_t toroidalDx(std::size_t a, std::size_t b) const
        {
            const std::size_t d = (a > b) ? (a - b) : (b - a);
            return (w == 0) ? d : std::min(d, w - d);
        }
        inline std::size_t toroidalDy(std::size_t a, std::size_t b) const
        {
            const std::size_t d = (a > b) ? (a - b) : (b - a);
            return (h == 0) ? d : std::min(d, h - d);
        }
        inline std::size_t manhattanToroidal(Cell a, Cell b) const
        {
            return toroidalDx(a.x, b.x) + toroidalDy(a.y, b.y);
        }

        // --- Mask operations ---
        inline std::uint8_t getMask(std::size_t x, std::size_t y) const { return mask[idx(x, y)]; }
        inline void setMask(std::size_t x, std::size_t y, std::uint8_t bits) { mask[idx(x, y)] |= bits; }
        inline void clearMask(std::size_t x, std::size_t y, std::uint8_t bits) { mask[idx(x, y)] &= static_cast<std::uint8_t>(~bits); }

        inline bool isWall(std::size_t x, std::size_t y) const { return getMask(x, y) & WALL; }
        inline bool isMine(std::size_t x, std::size_t y) const { return getMask(x, y) & MINE; }
        inline bool hasFriend(std::size_t x, std::size_t y) const { return getMask(x, y) & FRIEND; }
        inline bool hasEnemy(std::size_t x, std::size_t y) const { return getMask(x, y) & ENEMY; }
        inline bool hasShell(std::size_t x, std::size_t y) const { return getMask(x, y) & SHELL; }

        // Passability policy (Phase-1): walls and mines are blocking.
        inline bool isBlocked(std::size_t x, std::size_t y) const
        {
            return isWall(x, y) || isMine(x, y);
        }

        // --- Danger & shell-ETA utilities ---
        inline void addDanger(std::size_t x, std::size_t y, std::uint32_t k) { danger[idx(x, y)] += k; }
        inline std::uint32_t getDanger(std::size_t x, std::size_t y) const { return danger[idx(x, y)]; }
        inline void setShellETA(std::size_t x, std::size_t y, std::uint16_t t) { shell_eta[idx(x, y)] = t; }
        inline std::uint16_t getShellETA(std::size_t x, std::size_t y) const { return shell_eta[idx(x, y)]; }

        // --- Neighborhood (4-connected) with toroidal wrap ---
        template <class F>
        inline void forEachNeighbor4(std::size_t x, std::size_t y, F &&f) const
        {
            static const int dx[4] = {+1, -1, 0, 0};
            static const int dy[4] = {0, 0, +1, -1};
            for (int k = 0; k < 4; ++k)
            {
                const std::size_t nx = wrapX(static_cast<long long>(x) + dx[k]);
                const std::size_t ny = wrapY(static_cast<long long>(y) + dy[k]);
                f(nx, ny);
            }
        }
    };

} // namespace Algorithm_212788293_212497127
#endif
