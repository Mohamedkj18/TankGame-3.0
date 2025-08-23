#include "Player_212788293_212497127.h"

// Our internals
#include "BattleInfoLite.h"
#include "TeamState.h"
#include "WorldView.h"
#include "Pathfinding.h"
#include "Danger.h"
#include "roles/RolePolicy.h"
#include "Types.h"

#include <algorithm> // max
#include <cstdint>
#include <limits>

namespace Algorithm_212788293_212497127
{

    // ---------------- constants (tune here, still deterministic) --------------
    static constexpr std::uint32_t K_ENEMY_LOS_BASE = 8;  // base LOS danger
    static constexpr std::uint32_t K_ENEMY_LOS_DECAY = 1; // -1 per cell
    static constexpr int K_ENEMY_LOS_RANGE = 10;          // cap LOS paint
    static constexpr bool K_LOS_DIAGONALS = true;

    static constexpr std::uint32_t K_SHELL_BASE = 20;
    static constexpr std::uint32_t K_SHELL_DECAY = 6;
    static constexpr int K_SHELL_HORIZON = 2;
    static constexpr bool K_SHELL_MARK_PASS = true;

    static constexpr std::uint32_t K_WALL_PROX_K = 1;
    static constexpr int K_WALL_PROX_R = 1;

    static constexpr std::uint32_t K_RESERVATION_K = 50; // big cost to avoid teammate hops
    static constexpr int DIR8[8][2] = {
        {+1, 0}, {+1, +1}, {0, +1}, {-1, +1}, {-1, 0}, {-1, -1}, {0, -1}, {+1, -1}};

    // ---- satellite -> WorldView snapshot (and locate our tank) ----
    static WorldView buildWorldViewFromSatellite(::SatelliteView &sat,
                                                 std::size_t W, std::size_t H,
                                                 int my_player_idx,
                                                 Cell &out_my_pos)
    {
        WorldView wv{W, H};
        bool found_me = false;

        for (std::size_t y = 0; y < H; ++y)
        {
            for (std::size_t x = 0; x < W; ++x)
            {
                const char c = sat.getObjectAt(x, y);
                switch (c)
                {
                case '#':
                    wv.setMask(x, y, WorldView::WALL);
                    break;
                case '@':
                    wv.setMask(x, y, WorldView::MINE);
                    break;
                case '*':
                    wv.setMask(x, y, WorldView::SHELL);
                    break;
                case '%': // requesting tank
                    wv.setMask(x, y, WorldView::FRIEND);
                    if (!found_me)
                    {
                        out_my_pos = Cell{x, y};
                        found_me = true;
                    }
                    break;
                case '1':
                case '2':
                {
                    const int owner = (c == '1' ? 1 : 2);
                    if (owner == my_player_idx)
                        wv.setMask(x, y, WorldView::FRIEND);
                    else
                        wv.setMask(x, y, WorldView::ENEMY);
                    break;
                }
                default:
                    break; // empty
                }
            }
        }
        if (!found_me)
        {
            for (std::size_t y = 0; y < H && !found_me; ++y)
                for (std::size_t x = 0; x < W && !found_me; ++x)
                    if (wv.hasFriend(x, y))
                    {
                        out_my_pos = Cell{x, y};
                        found_me = true;
                    }
        }
        if (!found_me)
            out_my_pos = Cell{0, 0};
        return wv;
    }

    // ---- add big cost to cells reserved by teammates (so A* avoids them while planning) ----
    static void applyReservationDanger(WorldView &wv, const TeamState &tv, int my_tid, std::uint32_t k)
    {
        for (std::size_t y = 0; y < wv.h; ++y)
            for (std::size_t x = 0; x < wv.w; ++x)
            {
                int by = -1;
                if (tv.isMoveReserved(x, y, by) && by != -1 && by != my_tid)
                {
                    wv.addDanger(x, y, k);
                }
            }
    }

    // ---- deterministic role assignment (sticky per tank id) ----
    static RoleTag initialRoleForId(int tid)
    {
        switch (tid % 4)
        {
        case 0:
            return RoleTag::Aggressor;
        case 1:
            return RoleTag::Flanker;
        case 2:
            return RoleTag::Anchor;
        default:
            return RoleTag::Survivor;
        }
    }

    // ---------------- ctor ----------------

    Player_212788293_212497127::Player_212788293_212497127(int player_index,
                                                           std::size_t width,
                                                           std::size_t height,
                                                           std::size_t max_steps,
                                                           std::size_t num_shells) noexcept
        : player_idx_(player_index), w_(width), h_(height),
          max_steps_(max_steps), num_shells_(num_shells) {}

    int Player_212788293_212497127::getOrAssignTankIndex(const ::TankAlgorithm *tank)
    {
        auto it = tank_ids_.find(tank);
        if (it != tank_ids_.end())
            return it->second;
        const int id = next_tank_id_++;
        tank_ids_.emplace(tank, id);
        return id;
    }

    // helpers (local lambdas)
    static inline int toroidalDelta1(std::size_t a, std::size_t b, std::size_t M)
    {
        long long d = static_cast<long long>(b) - static_cast<long long>(a);
        if (d > static_cast<long long>(M) / 2)
            d -= static_cast<long long>(M);
        if (d < -static_cast<long long>(M) / 2)
            d += static_cast<long long>(M);
        return (d > 0) - (d < 0);
    }
    static inline int unitDeltaWrapped(int raw, std::size_t M)
    {
        if (raw > 1)
            return -1; // wrapped from 0 -> M-1
        if (raw < -1)
            return +1; // wrapped from M-1 -> 0
        return (raw > 0) - (raw < 0);
    }

    void Player_212788293_212497127::updateTankWithBattleInfo(::TankAlgorithm &tank,
                                                              ::SatelliteView &satellite_view)
    {
        // --- Build world snapshot & age intents
        Cell my_pos{0, 0};
        WorldView wv = buildWorldViewFromSatellite(satellite_view, w_, h_, player_idx_, my_pos);
        team_.ensure(w_, h_);
        team_.age();

        // --- Deterministic tank id & sticky role
        const int tid = getOrAssignTankIndex(&tank);
        RoleTag role;
        {
            auto it = role_by_tid_.find(tid);
            if (it != role_by_tid_.end())
                role = it->second;
            else
            {
                role = initialRoleForId(tid);
                role_by_tid_.emplace(tid, role);
            }
        }

        // --- Seed dangers (all deterministic)
        seedEnemyLOSDanger(wv, K_ENEMY_LOS_BASE, K_ENEMY_LOS_DECAY, K_ENEMY_LOS_RANGE, K_LOS_DIAGONALS);
        seedShellDanger(wv, K_SHELL_BASE);
        seedShellDangerPredictiveUnknown(wv, K_SHELL_BASE, K_SHELL_DECAY, K_SHELL_HORIZON, K_SHELL_MARK_PASS);
        seedWallProximityDanger(wv, K_WALL_PROX_K, K_WALL_PROX_R);
        applyReservationDanger(wv, team_, tid, K_RESERVATION_K);

        // --- Trapped helpers
        auto anyPassableNeighbor = [&](Cell c) -> bool
        {
            for (int i = 0; i < 8; ++i)
            {
                std::size_t nx = wv.wrapX(static_cast<long long>(c.x) + DIR8[i][0]);
                std::size_t ny = wv.wrapY(static_cast<long long>(c.y) + DIR8[i][1]);
                if (!wv.isBlocked(nx, ny) && !wv.isMine(nx, ny))
                    return true;
            }
            return false;
        };
        auto surroundedByWalls = [&](Cell c) -> bool
        {
            for (int i = 0; i < 8; ++i)
            {
                std::size_t nx = wv.wrapX(static_cast<long long>(c.x) + DIR8[i][0]);
                std::size_t ny = wv.wrapY(static_cast<long long>(c.y) + DIR8[i][1]);
                if (!wv.isWall(nx, ny))
                    return false;
            }
            return true;
        };
        auto surroundedByMines = [&](Cell c) -> bool
        {
            for (int i = 0; i < 8; ++i)
            {
                std::size_t nx = wv.wrapX(static_cast<long long>(c.x) + DIR8[i][0]);
                std::size_t ny = wv.wrapY(static_cast<long long>(c.y) + DIR8[i][1]);
                if (!wv.isMine(nx, ny))
                    return false;
            }
            return true;
        };
        auto firstHitIsEnemyAlongDir = [&](Cell from, int dx, int dy) -> bool
        {
            if (dx == 0 && dy == 0)
                return false;
            std::size_t cx = from.x, cy = from.y;
            const std::size_t limit = std::max(wv.w, wv.h);
            for (std::size_t step = 1; step < limit; ++step)
            {
                cx = wv.wrapX(static_cast<long long>(cx) + dx);
                cy = wv.wrapY(static_cast<long long>(cy) + dy);
                if (wv.hasFriend(cx, cy))
                    return false;
                if (wv.isWall(cx, cy))
                    return false;
                if (wv.isMine(cx, cy))
                    return false;
                if (wv.hasEnemy(cx, cy))
                    return true;
            }
            return false;
        };

        // --- Choose a role target (passable) and path
        const Cell target = targetForRole(role, wv, my_pos);

        std::vector<Cell> path = aStar(wv, my_pos, target);
        if (path.empty())
        {
            // Keep position if totally blocked; script will be empty; single-step will be rotate/no-op
            path.push_back(my_pos);
        }

        // --- Fill BattleInfo with short script from the path (up to K steps)
        BattleInfoLite bi{};
        bi.tag = role;
        bi.waypoint = (path.size() >= 2) ? path[1] : path[0];

        const int K = BattleInfoLite::BI_MAX_STEPS;
        int wrote = 0;
        for (std::size_t i = 0; i + 1 < path.size() && wrote < K; ++i)
        {
            const Cell a = path[i], b = path[i + 1];
            int rdx = static_cast<int>(b.x) - static_cast<int>(a.x);
            int rdy = static_cast<int>(b.y) - static_cast<int>(a.y);
            // adjust for wrap so result is in {-1,0,1}
            int dx = unitDeltaWrapped(rdx, wv.w);
            int dy = unitDeltaWrapped(rdy, wv.h);
            bi.plan_dx[wrote] = static_cast<std::int8_t>(dx);
            bi.plan_dy[wrote] = static_cast<std::int8_t>(dy);
            bi.plan_shoot[wrote] = 0; // default = move
            ++wrote;
        }
        bi.plan_len = static_cast<std::uint8_t>(wrote);

        // --- Trapped handling can override script (breacher / sniper)
        const bool stuck = !anyPassableNeighbor(my_pos);
        if (stuck && surroundedByWalls(my_pos))
        {
            // Breacher: choose adjacent wall deterministically and shoot
            bool found = false;
            int bdx = 0, bdy = 0;
            Cell pick{0, 0};
            static constexpr int D4[4][2] = {{+1, 0}, {0, +1}, {-1, 0}, {0, -1}};
            for (int pass = 0; pass < 2 && !found; ++pass)
            {
                const int cnt = pass == 0 ? 4 : 8;
                for (int i = 0; i < cnt; ++i)
                {
                    int sx = (pass == 0 ? D4[i][0] : DIR8[i][0]);
                    int sy = (pass == 0 ? D4[i][1] : DIR8[i][1]);
                    std::size_t nx = wv.wrapX(static_cast<long long>(my_pos.x) + sx);
                    std::size_t ny = wv.wrapY(static_cast<long long>(my_pos.y) + sy);
                    if (wv.isWall(nx, ny))
                    {
                        Cell cand{nx, ny};
                        if (!found || cand.y < pick.y || (cand.y == pick.y && cand.x < pick.x))
                        {
                            found = true;
                            pick = cand;
                            bdx = (sx > 0) - (sx < 0);
                            bdy = (sy > 0) - (sy < 0);
                        }
                    }
                }
            }
            if (found)
            {
                bi.plan_len = 1;
                bi.plan_dx[0] = static_cast<std::int8_t>(bdx);
                bi.plan_dy[0] = static_cast<std::int8_t>(bdy);
                bi.plan_shoot[0] = 1; // shoot when aligned
                bi.waypoint = my_pos; // no move
            }
        }
        else if (stuck && surroundedByMines(my_pos))
        {
            // Sniper (mines don’t get cleared by shells): only shoot if a clear enemy ray exists
            bool seen = false;
            Cell enemy{0, 0};
            std::size_t best = std::numeric_limits<std::size_t>::max();
            for (std::size_t y = 0; y < wv.h; ++y)
                for (std::size_t x = 0; x < wv.w; ++x)
                    if (wv.hasEnemy(x, y))
                    {
                        const auto d = wv.manhattanToroidal(my_pos, Cell{x, y});
                        if (!seen || d < best || (d == best && (y < enemy.y || (y == enemy.y && x < enemy.x))))
                        {
                            seen = true;
                            best = d;
                            enemy = Cell{x, y};
                        }
                    }
            if (seen)
            {
                int sdx = toroidalDelta1(my_pos.x, enemy.x, wv.w);
                int sdy = toroidalDelta1(my_pos.y, enemy.y, wv.h);
                if (firstHitIsEnemyAlongDir(my_pos, sdx, sdy))
                {
                    bi.plan_len = 1;
                    bi.plan_dx[0] = static_cast<std::int8_t>(sdx);
                    bi.plan_dy[0] = static_cast<std::int8_t>(sdy);
                    bi.plan_shoot[0] = 1; // shoot when aligned
                    bi.waypoint = my_pos; // do not move into mines
                }
                else
                {
                    // keep script (likely empty) and use single-step facing to scan; no movement
                    bi.dir_dx = sdx;
                    bi.dir_dy = sdy;
                    bi.shoot_when_aligned = false;
                    bi.waypoint = my_pos;
                }
            }
            else
            {
                // No enemy: rotate in tank layer (leave plan empty)
                bi.dir_dx = 0;
                bi.dir_dy = 0;
                bi.shoot_when_aligned = false;
                bi.waypoint = my_pos;
            }
        }
        else
        {
            // Not trapped: also fill single-step as a fallback (first hop)
            int dx = toroidalDelta1(my_pos.x, bi.waypoint.x, wv.w);
            int dy = toroidalDelta1(my_pos.y, bi.waypoint.y, wv.h);
            bi.dir_dx = dx;
            bi.dir_dy = dy;
            bi.shoot_when_aligned = false;

            // Micro: if the very first ray is a clean enemy LOS, mark that step as "shoot"
            if (bi.plan_len > 0)
            {
                const int sdx = static_cast<int>(bi.plan_dx[0]);
                const int sdy = static_cast<int>(bi.plan_dy[0]);
                if (firstHitIsEnemyAlongDir(my_pos, sdx, sdy))
                {
                    bi.plan_shoot[0] = 1;
                }
            }
        }

        // --- Reserve ahead for the plan so teammates can avoid us
        int friends_alive = 0;
        for (std::size_t y = 0; y < wv.h; ++y)
            for (std::size_t x = 0; x < wv.w; ++x)
                if (wv.hasFriend(x, y))
                    ++friends_alive;
        const std::uint8_t base_ttl = static_cast<std::uint8_t>(std::max(1, friends_alive - 1));

        if (path.size() >= 2)
        {
            const int reserve_steps = std::min<int>(bi.plan_len, static_cast<int>(path.size() - 1));
            for (int i = 0; i < reserve_steps; ++i)
            {
                const Cell dest = path[i + 1];
                team_.reserveMove(dest.x, dest.y, tid, static_cast<std::uint8_t>(base_ttl + i));
            }
        }
        else
        {
            // reserve current cell if not moving (prevents bumping)
            team_.reserveMove(my_pos.x, my_pos.y, tid, base_ttl);
        }

        // --- Push to tank
        tank.updateBattleInfo(bi);
    }

}
