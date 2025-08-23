#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include "WorldView.h"
#include "Types.h"

namespace Algorithm_212788293_212497127
{

    // 8-dir vectors (must match your facing indices)
    static constexpr int DIR8[8][2] = {
        {+1, 0}, {+1, +1}, {0, +1}, {-1, +1}, {-1, 0}, {-1, -1}, {0, -1}, {+1, -1}};

    // Find the first wall along the tank's current facing; return true if found and fill 'hit'.
    inline bool firstWallAhead(const WorldView &wv, const TankLocal &me, Cell &hit)
    {
        std::size_t cx = me.x, cy = me.y;
        const int dx = DIR8[me.facing_deg][0], dy = DIR8[me.facing_deg][1];
        const std::size_t limit = std::max(wv.w, wv.h);
        for (std::size_t step = 1; step < limit; ++step)
        {
            cx = wv.wrapX(static_cast<long long>(cx) + dx);
            cy = wv.wrapY(static_cast<long long>(cy) + dy);
            if (wv.isWall(cx, cy))
            {
                hit = Cell{cx, cy};
                return true;
            }
            if (wv.hasFriend(cx, cy) || wv.isMine(cx, cy) || wv.hasEnemy(cx, cy))
            {
                // Something else blocks before a wall
                return false;
            }
        }
        return false;
    }

    // Render board to 'out'. If 'me' != nullptr, overlay 'T' at tank position.
    // If 'mark' != nullptr, overlay 'X' at that cell (e.g., a shot target).
    inline void printBoardWithOverlay(const WorldView &wv,
                                      std::ostream &out,
                                      const char *title,
                                      const TankLocal *me = nullptr,
                                      const Cell *mark = nullptr)
    {
        std::vector<std::string> grid(wv.h, std::string(wv.w, '.'));
        for (std::size_t y = 0; y < wv.h; ++y)
        {
            for (std::size_t x = 0; x < wv.w; ++x)
            {
                if (wv.isWall(x, y))
                    grid[y][x] = '#';
                else if (wv.isMine(x, y))
                    grid[y][x] = '@';
                else if (wv.hasShell(x, y))
                    grid[y][x] = '*';
                else if (wv.hasEnemy(x, y))
                    grid[y][x] = 'E';
                else if (wv.hasFriend(x, y))
                    grid[y][x] = 'F';
            }
        }
        if (me)
        {
            grid[me->y][me->x] = 'T';
        }
        if (mark)
        {
            grid[mark->y][mark->x] = 'X';
        }

        out << "\n=== " << (title ? title : "Board") << " ===\n";
        for (std::size_t y = 0; y < wv.h; ++y)
        {
            for (std::size_t x = 0; x < wv.w; ++x)
                out << grid[y][x];
            out << '\n';
        }
        if (me)
        {
            out << "Tank at (" << me->x << "," << me->y
                << "), facing=" << static_cast<int>(me->facing_deg) << "\n";
        }
        out << std::flush;
    }

} // namespace Algorithm_212788293_212497127
