#include "DecisionRunner.h"
#include "LOS.h"
#include "WorldView.h"
#include "TeamState.h"

// Only .cpp files may include ActionRequest to avoid ODR issues
#include "common/ActionRequest.h"

#include <cstdlib>
#include <algorithm>
#include <limits>

namespace Algorithm_212788293_212497127
{

    namespace
    {

        // snap any degree to multiples of 45 in [0,360)
        inline int snap45(int deg)
        {
            int a = deg % 360;
            if (a < 0)
                a += 360;
            a = ((a + 22) / 45) * 45;
            if (a >= 360)
                a -= 360;
            return a;
        }

        // minimal signed angle delta from a to b, multiples of 45, result in {-180,-135,...,135}
        inline int delta45(int a, int b)
        {
            int da = snap45(b) - snap45(a);
            if (da > 180)
                da -= 360;
            if (da < -180)
                da += 360;
            return da;
        }

        // choose left/right 45° to reduce |delta|; tie -> LEFT deterministically
        inline ::ActionRequest turnTowardOnce(int cur_deg, int target_deg)
        {
            int d = delta45(cur_deg, target_deg);
            if (d == 0)
                return ::ActionRequest::DoNothing;
            // if |d| >= 90, we still move only 45 per tick; pick direction to reduce |d|
            if (d > 0)
            {
                // need to rotate positive (right) to reach target; but tie-break prefers LEFT only on exact tie.
                // For single-step reduce, both left or right reduce |d| only when |d|==180 — pick LEFT deterministically.
                if (d == 180)
                    return ::ActionRequest::RotateLeft45;
                return ::ActionRequest::RotateRight45;
            }
            else
            {
                if (d == -180)
                    return ::ActionRequest::RotateLeft45; // tie -> LEFT
                return ::ActionRequest::RotateLeft45;
            }
        }

        // compute desired facing from me toward waypoint using toroidal shortest delta.
        // 8-connected: 0,45,90,...,315 degrees.
        inline int desiredFacingTo(const WorldView &wv, const TankLocal &me, const Cell &wp)
        {
            // toroidal deltas minimized on each axis
            std::size_t dx1 = wv.toroidalDx(me.x, wp.x);
            std::size_t dy1 = wv.toroidalDy(me.y, wp.y);

            // decide signed step on each axis (choose wrapped direction if shorter or equal; ties deterministic)
            int sx = 0, sy = 0;
            // x: moving east (+1) or west (-1)
            {
                // raw diff to the right (non-wrapped)
                std::size_t right = (wp.x >= me.x) ? (wp.x - me.x) : (wv.w - (me.x - wp.x));
                std::size_t left = (me.x >= wp.x) ? (me.x - wp.x) : (wv.w - (wp.x - me.x));
                if (right < left)
                    sx = +1;
                else if (left < right)
                    sx = -1;
                else
                    sx = +1; // tie → prefer +1 deterministically
            }
            // y: moving down (+1) or up (-1) (screen coords: y grows downward)
            {
                std::size_t down = (wp.y >= me.y) ? (wp.y - me.y) : (wv.h - (me.y - wp.y));
                std::size_t up = (me.y >= wp.y) ? (me.y - wp.y) : (wv.h - (wp.y - me.y));
                if (down < up)
                    sy = +1;
                else if (up < down)
                    sy = -1;
                else
                    sy = +1; // tie → prefer +1 deterministically
            }

            // zero axis if already aligned on that axis
            if (dx1 == 0)
                sx = 0;
            if (dy1 == 0)
                sy = 0;

            // map (sx,sy) to 8-direction angle
            if (sx == +1 && sy == 0)
                return 0;
            if (sx == +1 && sy == +1)
                return 45;
            if (sx == 0 && sy == +1)
                return 90;
            if (sx == -1 && sy == +1)
                return 135;
            if (sx == -1 && sy == 0)
                return 180;
            if (sx == -1 && sy == -1)
                return 225;
            if (sx == 0 && sy == -1)
                return 270;
            if (sx == +1 && sy == -1)
                return 315;

            // if waypoint == me: keep current facing
            return snap45(me.facing_deg);
        }

        // next cell coordinates if we move one step forward from current facing
        inline void forwardCell(const WorldView &wv, const TankLocal &me, std::size_t &nx, std::size_t &ny)
        {
            int dx = 0, dy = 0;
            stepFromFacing45(me.facing_deg, dx, dy);
            nx = wv.wrapX(static_cast<long long>(me.x) + dx);
            ny = wv.wrapY(static_cast<long long>(me.y) + dy);
        }

    } // namespace

    // 8-direction unit vectors, index order must match your rotation semantics
    static constexpr int DIR8[8][2] = {
        {+1, 0}, {+1, +1}, {0, +1}, {-1, +1}, {-1, 0}, {-1, -1}, {0, -1}, {+1, -1}};

    // Map a delta to a dir index in DIR8 (dx,dy must be in {-1,0,1} and not {0,0})
    static int dirIndexFromDelta(int dx, int dy)
    {
        for (int i = 0; i < 8; ++i)
            if (DIR8[i][0] == dx && DIR8[i][1] == dy)
                return i;
        return 0; // default (shouldn't happen if inputs valid)
    }

    // One 45° rotation step toward target index; break ties by rotating LEFT deterministically
    static ::ActionRequest rotateStepToward(int fromDir, int toDir)
    {
        int cw = (toDir - fromDir + 8) % 8;  // turn right steps
        int ccw = (fromDir - toDir + 8) % 8; // turn left steps
        if (ccw <= cw)
            return ::ActionRequest::RotateLeft45;
        return ::ActionRequest::RotateRight45;
    }

    // (dx,dy) unit step from 'a' to 'b' snapped to {-1,0,1}
    static void unitStepToward(const Cell &a, const Cell &b, int &dx, int &dy)
    {
        long long ddx = static_cast<long long>(b.x) - static_cast<long long>(a.x);
        long long ddy = static_cast<long long>(b.y) - static_cast<long long>(a.y);
        dx = (ddx > 0) - (ddx < 0);
        dy = (ddy > 0) - (ddy < 0);
    }

    // Any passable neighbor? (used only to gate “trapped” code; movement rules elsewhere remain unchanged)
    static bool anyPassableNeighbor(const WorldView &wv, const Cell me)
    {
        for (int i = 0; i < 8; ++i)
        {
            std::size_t nx = wv.wrapX(static_cast<long long>(me.x) + DIR8[i][0]);
            std::size_t ny = wv.wrapY(static_cast<long long>(me.y) + DIR8[i][1]);
            if (!wv.isBlocked(nx, ny) && !wv.isMine(nx, ny))
                return true;
        }
        return false;
    }

    // All 8 neighbors are walls?
    static bool surroundedByWalls(const WorldView &wv, const Cell me)
    {
        for (int i = 0; i < 8; ++i)
        {
            std::size_t nx = wv.wrapX(static_cast<long long>(me.x) + DIR8[i][0]);
            std::size_t ny = wv.wrapY(static_cast<long long>(me.y) + DIR8[i][1]);
            if (!wv.isWall(nx, ny))
                return false;
        }
        return true;
    }

    // All 8 neighbors are mines?
    static bool surroundedByMines(const WorldView &wv, const Cell me)
    {
        for (int i = 0; i < 8; ++i)
        {
            std::size_t nx = wv.wrapX(static_cast<long long>(me.x) + DIR8[i][0]);
            std::size_t ny = wv.wrapY(static_cast<long long>(me.y) + DIR8[i][1]);
            if (!wv.isMine(nx, ny))
                return false;
        }
        return true;
    }

    // Deterministically pick an adjacent wall to breach: prefer 4-neighbors, then diagonals; tiebreak (y,x)
    static bool pickAdjacentWall(const WorldView &wv, const Cell me, Cell &wallCell, int &wallDirIdx)
    {
        bool found = false;
        // 4-neighbors first
        static constexpr int D4[4][2] = {{+1, 0}, {0, +1}, {-1, 0}, {0, -1}};
        for (int pass = 0; pass < 2; ++pass)
        {
            int count = pass == 0 ? 4 : 8;
            for (int i = 0; i < count; ++i)
            {
                int dx = (pass == 0 ? D4[i][0] : DIR8[i][0]);
                int dy = (pass == 0 ? D4[i][1] : DIR8[i][1]);
                std::size_t nx = wv.wrapX(static_cast<long long>(me.x) + dx);
                std::size_t ny = wv.wrapY(static_cast<long long>(me.y) + dy);
                if (wv.isWall(nx, ny))
                {
                    Cell cand{nx, ny};
                    if (!found || (cand.y < wallCell.y) || (cand.y == wallCell.y && cand.x < wallCell.x))
                    {
                        found = true;
                        wallCell = cand;
                        wallDirIdx = dirIndexFromDelta((dx > 0) - (dx < 0), (dy > 0) - (dy < 0));
                    }
                }
            }
            if (found)
                break;
        }
        return found;
    }

    // First hit along current facing is a wall (and not a friend). Stop on walls; respect torus.
    static bool firstHitIsWallFacing(const WorldView &wv, const TeamState &tv, const TankLocal &me)
    {
        const int fx = DIR8[me.facing_deg][0], fy = DIR8[me.facing_deg][1];
        std::size_t cx = me.x, cy = me.y;
        for (std::size_t step = 1; step < std::max(wv.w, wv.h); ++step)
        {
            cx = wv.wrapX(static_cast<long long>(cx) + fx);
            cy = wv.wrapY(static_cast<long long>(cy) + fy);
            if (wv.hasFriend(cx, cy))
                return false; // friendly in the way → don't shoot
            if (wv.isWall(cx, cy))
                return true; // wall is first blocker → good to shoot
            if (wv.isMine(cx, cy))
                return false; // avoid detonating mines by fire (policy choice)
            if (wv.hasEnemy(cx, cy))
                return false; // enemy before wall → your normal shoot gate will handle it
        }
        return false;
    }

    ::ActionRequest DecisionRunner::decide(const BattleInfoLite &info,
                                           const WorldView &wv,
                                           const TeamState &tv,
                                           const TankLocal &me)
    {

        Cell meCell;
        meCell.x = me.x;
        meCell.y = me.y;

        // Only trigger if truly stuck (no passable neighbor). Keeps normal logic otherwise.
        const bool stuck = !anyPassableNeighbor(wv, meCell);
        if (stuck && surroundedByMines(wv, meCell))
        {
            // Sniper: hold ground, rotate toward nearest enemy, shoot if clear (no friendly-fire).
            // Find nearest enemy
            bool seen = false;
            Cell enemy{};
            std::size_t bestd = std::numeric_limits<std::size_t>::max();
            for (std::size_t y = 0; y < wv.h; ++y)
                for (std::size_t x = 0; x < wv.w; ++x)
                    if (wv.hasEnemy(x, y))
                    {
                        std::size_t d = wv.manhattanToroidal(meCell, Cell{x, y});
                        if (!seen || d < bestd || (d == bestd && (y < enemy.y || (y == enemy.y && x < enemy.x))))
                        {
                            seen = true;
                            bestd = d;
                            enemy = Cell{x, y};
                        }
                    }

            if (seen)
            {
                int dx, dy;
                unitStepToward(meCell, enemy, dx, dy);
                int targetDir = dirIndexFromDelta(dx, dy);
                if (me.facing_deg == targetDir)
                {
                    if (firstHitIsEnemyFacing(wv, tv, me))
                        return ::ActionRequest::Shoot;
                    // aligned but something else blocks (friend/wall) → rotate to search another lane deterministically
                    return ::ActionRequest::RotateLeft45;
                }
                else
                {
                    return rotateStepToward(me.facing_deg, targetDir);
                }
            }
            else
            {
                // No enemies visible: deterministic idle (rotate left) rather than moving onto mines.
                return ::ActionRequest::RotateLeft45;
            }
        }

        else if (stuck)
        {
            // Breacher: face a nearby wall and shoot (once aligned), avoiding friendly fire.
            Cell wall{};
            int wdir = 0;
            if (pickAdjacentWall(wv, meCell, wall, wdir))
            {
                if (me.facing_deg == wdir)
                {
                    if (firstHitIsWallFacing(wv, tv, me))
                        return ::ActionRequest::Shoot;
                    // If something else is in front (friend/enemy/mine), prefer to rotate to shift lane
                    return ::ActionRequest::RotateLeft45;
                }
                else
                {
                    return rotateStepToward(me.facing_deg, wdir);
                }
            }
            // No adjacent wall after all? fall through to normal logic
        }

        // 1) Shoot gate: if first thing ahead is an enemy, shoot (friendly-fire safe via TeamState).
        if (firstHitIsEnemyFacing(wv, tv, me))
        {
            return ::ActionRequest::Shoot;
        }

        // 2) Movement: align toward the role’s waypoint and step forward if legal.
        //    (Assumes Player gave a reasonable waypoint; pathfinding can be done at planning time.)
        const int want = desiredFacingTo(wv, me, info.waypoint);
        const int cur = snap45(me.facing_deg);
        if (want != cur)
        {
            return turnTowardOnce(cur, want);
        }

        // If already aligned, try to move forward if not blocked or reserved by teammate
        std::size_t nx = 0, ny = 0;
        forwardCell(wv, me, nx, ny);

        int by = -1;
        const bool reserved = tv.isMoveReserved(nx, ny, by);
        const bool blocked = wv.isBlocked(nx, ny) || wv.hasFriend(nx, ny);

        if (!reserved && !blocked)
        {
            return ::ActionRequest::MoveForward;
        }

        // 3) Aligned but next cell is not available -> pick a deterministic alternate micro:
        //    rotate left 45° as first fallback (deterministic tie-break), otherwise right 45°.
        //    Next tick we’ll re-evaluate.
        return ::ActionRequest::RotateLeft45;
    }

} // namespace Algorithm_212788293_212497127
