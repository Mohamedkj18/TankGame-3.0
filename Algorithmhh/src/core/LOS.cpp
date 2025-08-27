#include "LOS.h"
#include "WorldView.h"
#include <numeric> // std::gcd, std::lcm (C++17+)

namespace Algorithm_212788293_212497127
{

    // Normalize to multiples of 45° and map to grid step (screen coords: y grows downward).
    void stepFromFacing45(int facing_deg, int &dx, int &dy)
    {
        // Normalize to [0,360)
        int a = facing_deg % 360;
        if (a < 0)
            a += 360;
        // Snap to nearest multiple of 45 (defensive; caller should already provide multiples)
        a = ((a + 22) / 45) * 45;
        if (a >= 360)
            a -= 360;

        switch (a)
        {
        case 0:
            dx = +1;
            dy = 0;
            break; // east/right
        case 45:
            dx = +1;
            dy = +1;
            break; // south-east (down-right)
        case 90:
            dx = 0;
            dy = +1;
            break; // south/down
        case 135:
            dx = -1;
            dy = +1;
            break; // south-west
        case 180:
            dx = -1;
            dy = 0;
            break; // west/left
        case 225:
            dx = -1;
            dy = -1;
            break; // north-west (up-left)
        case 270:
            dx = 0;
            dy = -1;
            break; // north/up
        case 315:
            dx = +1;
            dy = -1;
            break; // north-east
        default:
            dx = +1;
            dy = 0;
            break;
        }
    }

    // Compute a safe upper bound for steps until the ray repeats on a torus.
    // For dx∈{-1,0,1},dy∈{-1,0,1}, the cycle along x is (dx==0?1:W), along y is (dy==0?1:H).
    static std::size_t maxRaySteps(std::size_t W, std::size_t H, int dx, int dy)
    {
        const std::size_t px = (dx == 0) ? 1 : W;
        const std::size_t py = (dy == 0) ? 1 : H;
#if __cpp_lib_gcd_lcm
        return std::lcm(px, py);
#else
        auto gcd = [](std::size_t a, std::size_t b)
        {
            while (b)
            {
                auto t = a % b;
                a = b;
                b = t;
            }
            return a;
        };
        return (px / gcd(px, py)) * py;
#endif
    }

    RayHit raycastFirstHit(const WorldView &wv,
                           std::size_t x, std::size_t y,
                           int dx, int dy,
                           std::size_t max_steps)
    {
        RayHit out{};
        const std::size_t W = wv.w, H = wv.h;
        if (W == 0 || H == 0 || (dx == 0 && dy == 0))
            return out;

        const std::size_t limit = (max_steps == 0) ? maxRaySteps(W, H, dx, dy)
                                                   : max_steps;

        std::size_t cx = x, cy = y;
        for (std::size_t s = 1; s <= limit; ++s)
        {
            cx = wv.wrapX(static_cast<long long>(cx) + dx);
            cy = wv.wrapY(static_cast<long long>(cy) + dy);

            // Deterministic check order: walls first (hard blocker), then tanks, then shells, then mines.
            if (wv.isWall(cx, cy))
                return RayHit{HitType::Wall, cx, cy, s};
            if (wv.hasFriend(cx, cy))
                return RayHit{HitType::Friend, cx, cy, s};
            if (wv.hasEnemy(cx, cy))
                return RayHit{HitType::Enemy, cx, cy, s};
            if (wv.hasShell(cx, cy))
                return RayHit{HitType::Shell, cx, cy, s};
            if (wv.isMine(cx, cy))
                return RayHit{HitType::Mine, cx, cy, s};
            // empty: keep marching
        }
        return out; // HitType::None
    }

    bool firstHitIsEnemyFacing(const WorldView &wv, const TeamState &tv, const TankLocal &me)
    {
        int dx = 0, dy = 0;
        stepFromFacing45(me.facing_deg, dx, dy);

        // March along the ray until the first hard blocker in WorldView
        const std::size_t W = wv.w, H = wv.h;
        std::size_t cx = me.x, cy = me.y;
        for (std::size_t s = 1;; ++s)
        {
            cx = wv.wrapX(static_cast<long long>(cx) + dx);
            cy = wv.wrapY(static_cast<long long>(cy) + dy);

            // If a teammate has reserved this cell for this step, we must not shoot through it.
            int by = -1;
            if (tv.isMoveReserved(cx, cy, by))
                return false; // a friend will step here

            if (wv.isWall(cx, cy))
                return false; // blocked by wall
            if (wv.hasFriend(cx, cy))
                return false; // current friend in line
            if (wv.hasEnemy(cx, cy))
                return true; // enemy is the first hard hit
            if (wv.hasShell(cx, cy))
                return false; // conservative: don't shoot through shells
            if (wv.isMine(cx, cy))
                return false; // mine blocks LOS (policy)
            // else: empty → continue; break if we looped fully around:
            if (cx == me.x && cy == me.y)
                return false; // wrapped around → nothing hit
        }
    }

} // namespace Algorithm_212788293_212497127
