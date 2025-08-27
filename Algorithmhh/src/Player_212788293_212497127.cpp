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
#include <iostream>
#include <optional>

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

        // ---------------- seed dangers (deterministic) ----------------
        seedEnemyLOSDanger(wv, K_ENEMY_LOS_BASE, K_ENEMY_LOS_DECAY, K_ENEMY_LOS_RANGE, K_LOS_DIAGONALS);
        seedShellDanger(wv, K_SHELL_BASE);
        seedShellDangerPredictiveUnknown(wv, K_SHELL_BASE, K_SHELL_DECAY, K_SHELL_HORIZON, K_SHELL_MARK_PASS);
        seedWallProximityDanger(wv, K_WALL_PROX_K, K_WALL_PROX_R);
        applyReservationDanger(wv, team_, tid, K_RESERVATION_K);

        // Make friend cells unattractive so A* avoids overlap (hard guard still below)
        static constexpr std::uint32_t K_FRIEND_HUGE = 1000;
        for (std::size_t y = 0; y < wv.h; ++y)
            for (std::size_t x = 0; x < wv.w; ++x)
                if (wv.hasFriend(x, y))
                    wv.addDanger(x, y, K_FRIEND_HUGE);

        // ---------------- helpers ----------------
        static constexpr int DIR8[8][2] = {
            {+1, 0}, {+1, +1}, {0, +1}, {-1, +1}, {-1, 0}, {-1, -1}, {0, -1}, {+1, -1}};

        auto unitDeltaWrapped = [&](int raw, std::size_t M) -> int
        {
            if (raw > 1)
                return -1;
            if (raw < -1)
                return +1;
            return (raw > 0) - (raw < 0);
        };
        auto toroidalDelta1 = [&](std::size_t a, std::size_t b, std::size_t M) -> int
        {
            long long d = (long long)b - (long long)a;
            if (d > (long long)M / 2)
                d -= (long long)M;
            if (d < -(long long)M / 2)
                d += (long long)M;
            return (d > 0) - (d < 0);
        };

        // first hit classifier along a ray (ignores shells)
        auto firstHitKindAlongDir = [&](Cell from, int dx, int dy) -> char
        {
            // returns: 'F' friend, '#': wall, '@': mine, 'E': enemy, '.': none
            if (dx == 0 && dy == 0)
                return '.';
            std::size_t cx = from.x, cy = from.y;
            const std::size_t limit = std::max(wv.w, wv.h);
            for (std::size_t step = 1; step < limit; ++step)
            {
                cx = wv.wrapX((long long)cx + dx);
                cy = wv.wrapY((long long)cy + dy);
                if (wv.hasFriend(cx, cy))
                    return 'F';
                if (wv.isWall(cx, cy))
                    return '#';
                if (wv.isMine(cx, cy))
                    return '@';
                if (wv.hasEnemy(cx, cy))
                    return 'E';
            }
            return '.';
        };

        // return best clean-LOS direction to ANY enemy; choose the shortest enemy-on-ray in steps
        auto findBestCleanLOSAim = [&]() -> std::optional<std::pair<int, int>>
        {
            std::optional<std::pair<int, int>> best{};
            std::size_t best_steps = std::numeric_limits<std::size_t>::max();

            for (int i = 0; i < 8; ++i)
            {
                int dx = DIR8[i][0], dy = DIR8[i][1];
                char k = firstHitKindAlongDir(my_pos, dx, dy);
                if (k != 'E')
                    continue;

                // count steps to the first enemy along this ray
                std::size_t cx = my_pos.x, cy = my_pos.y, steps = 0;
                const std::size_t limit = std::max(wv.w, wv.h);
                for (std::size_t s = 1; s < limit; ++s)
                {
                    cx = wv.wrapX((long long)cx + dx);
                    cy = wv.wrapY((long long)cy + dy);
                    if (wv.hasEnemy(cx, cy))
                    {
                        steps = s;
                        break;
                    }
                }
                if (!best || steps < best_steps)
                {
                    best = std::make_pair(dx, dy);
                    best_steps = steps;
                }
            }
            return best;
        };

        auto anyPassableNeighbor8 = [&](Cell c) -> bool
        {
            for (int i = 0; i < 8; ++i)
            {
                std::size_t nx = wv.wrapX((long long)c.x + DIR8[i][0]);
                std::size_t ny = wv.wrapY((long long)c.y + DIR8[i][1]);
                if (!wv.isWall(nx, ny) && !wv.isMine(nx, ny))
                    return true;
            }
            return false;
        };

        auto pickNearestEnemy = [&]() -> std::optional<Cell>
        {
            bool seen = false;
            Cell best{0, 0};
            std::size_t bestd = 0;
            for (std::size_t y = 0; y < wv.h; ++y)
                for (std::size_t x = 0; x < wv.w; ++x)
                    if (wv.hasEnemy(x, y))
                    {
                        std::size_t d = wv.manhattanToroidal(my_pos, Cell{x, y});
                        if (!seen || d < bestd || (d == bestd && (y < best.y || (y == best.y && x < best.x))))
                        {
                            seen = true;
                            bestd = d;
                            best = Cell{x, y};
                        }
                    }
            if (!seen)
                return std::nullopt;
            return best;
        };

        auto pickAdjacentWallGreedyToward = [&](Cell target, int &dx, int &dy) -> bool
        {
            // choose adjacent wall that most reduces toroidal manhattan to target.
            bool found = false;
            Cell best{0, 0};
            int bdx = 0, bdy = 0;
            std::size_t bestd = 0;
            static constexpr int D4[4][2] = {{+1, 0}, {0, +1}, {-1, 0}, {0, -1}};
            auto trycand = [&](int sx, int sy)
            {
                std::size_t nx = wv.wrapX((long long)my_pos.x + sx);
                std::size_t ny = wv.wrapY((long long)my_pos.y + sy);
                if (!wv.isWall(nx, ny))
                    return;
                std::size_t d = wv.manhattanToroidal(Cell{nx, ny}, target);
                if (!found || d < bestd || (d == bestd && (ny < best.y || (ny == best.y && nx < best.x))))
                {
                    found = true;
                    bestd = d;
                    best = Cell{nx, ny};
                    bdx = (sx > 0) - (sx < 0);
                    bdy = (sy > 0) - (sy < 0);
                }
            };
            for (int i = 0; i < 4; ++i)
                trycand(D4[i][0], D4[i][1]); // prefer 4-neigh first
            for (int i = 0; i < 8; ++i)
                trycand(DIR8[i][0], DIR8[i][1]);
            if (found)
            {
                dx = bdx;
                dy = bdy;
                return true;
            }
            return false;
        };

        // ---------------- role target & path (may be overridden by LOS/breach) ----------------
        const Cell target_role = targetForRole(role, wv, my_pos);
        std::vector<Cell> path = aStar(wv, my_pos, target_role);
        if (path.empty())
            path.push_back(my_pos);

        // ---------------- construct BI scaffold (may be overridden) ----------------
        BattleInfoLite bi{};
        bi.tag = role;
        bi.waypoint = (path.size() >= 2) ? path[1] : path[0];

        {
            int dx = toroidalDelta1(my_pos.x, bi.waypoint.x, wv.w);
            int dy = toroidalDelta1(my_pos.y, bi.waypoint.y, wv.h);
            bi.dir_dx = dx;
            bi.dir_dy = dy;
            bi.shoot_when_aligned = false;
        }

        // ====================== PRIORITY 0: always shoot if any clean LOS exists ======================
        if (auto aim = findBestCleanLOSAim())
        {
            bi.plan_len = 1;
            bi.plan_dx[0] = static_cast<std::int8_t>(aim->first);
            bi.plan_dy[0] = static_cast<std::int8_t>(aim->second);
            bi.plan_shoot[0] = 1;   // shoot once aligned
            bi.waypoint = my_pos;   // no movement, we just aim & fire
            bi.dir_dx = aim->first; // TA rotates toward this direction
            bi.dir_dy = aim->second;
            bi.shoot_when_aligned = true;

            // Reserve our tile so a teammate doesn't bump us while we fire
            int friends_alive = 0;
            for (std::size_t y = 0; y < wv.h; ++y)
                for (std::size_t x = 0; x < wv.w; ++x)
                    if (wv.hasFriend(x, y))
                        ++friends_alive;
            const std::uint8_t base_ttl = (std::uint8_t)std::max(1, friends_alive - 1);
            team_.reserveMove(my_pos.x, my_pos.y, tid, base_ttl);

            tank.updateBattleInfo(bi);
            return;
        }

        // ====================== PRIORITY 1: stuck handling & breaching ======================
        const bool stuck = !anyPassableNeighbor8(my_pos);

        if (stuck)
        {
            // If we have adjacent walls -> greedy breacher toward nearest enemy
            bool any_wall_adj = false;
            for (int i = 0; i < 8; ++i)
            {
                std::size_t nx = wv.wrapX((long long)my_pos.x + DIR8[i][0]);
                std::size_t ny = wv.wrapY((long long)my_pos.y + DIR8[i][1]);
                if (wv.isWall(nx, ny))
                {
                    any_wall_adj = true;
                    break;
                }
            }

            if (auto enemy = pickNearestEnemy())
            {
                if (any_wall_adj)
                {
                    int bdx = 0, bdy = 0;
                    if (pickAdjacentWallGreedyToward(*enemy, bdx, bdy))
                    {
                        bi.plan_len = 1;
                        bi.plan_dx[0] = (std::int8_t)bdx;
                        bi.plan_dy[0] = (std::int8_t)bdy;
                        bi.plan_shoot[0] = 1;
                        bi.waypoint = my_pos;
                        bi.dir_dx = bdx;
                        bi.dir_dy = bdy;
                        bi.shoot_when_aligned = true;

                        int friends_alive = 0;
                        for (std::size_t y = 0; y < wv.h; ++y)
                            for (std::size_t x = 0; x < wv.w; ++x)
                                if (wv.hasFriend(x, y))
                                    ++friends_alive;
                        const std::uint8_t base_ttl = (std::uint8_t)std::max(1, friends_alive - 1);
                        team_.reserveMove(my_pos.x, my_pos.y, tid, base_ttl);

                        tank.updateBattleInfo(bi);
                        return;
                    }
                }
                else
                {
                    // No adjacent walls: outward breach along the enemy ray if a wall is the first blocker
                    int sdx = toroidalDelta1(my_pos.x, enemy->x, wv.w);
                    int sdy = toroidalDelta1(my_pos.y, enemy->y, wv.h);
                    if (firstHitKindAlongDir(my_pos, sdx, sdy) == '#')
                    {
                        bi.plan_len = 1;
                        bi.plan_dx[0] = (std::int8_t)sdx;
                        bi.plan_dy[0] = (std::int8_t)sdy;
                        bi.plan_shoot[0] = 1;
                        bi.waypoint = my_pos;
                        bi.dir_dx = sdx;
                        bi.dir_dy = sdy;
                        bi.shoot_when_aligned = true;

                        int friends_alive = 0;
                        for (std::size_t y = 0; y < wv.h; ++y)
                            for (std::size_t x = 0; x < wv.w; ++x)
                                if (wv.hasFriend(x, y))
                                    ++friends_alive;
                        const std::uint8_t base_ttl = (std::uint8_t)std::max(1, friends_alive - 1);
                        team_.reserveMove(my_pos.x, my_pos.y, tid, base_ttl);

                        tank.updateBattleInfo(bi);
                        return;
                    }
                }
            }
            // fall through if nothing to breach this tick
        }

        // ====================== PRIORITY 2: normal role movement (guarded) ======================
        {
            const int K = BattleInfoLite::BI_MAX_STEPS;
            int wrote = 0;

            for (std::size_t i = 0; i + 1 < path.size() && wrote < K; ++i)
            {
                const Cell a = path[i], b = path[i + 1];

                // HARD GUARDS: don't step into a friend or a cell reserved by another tank
                int by = -1;
                if (wv.hasFriend(b.x, b.y))
                    break;
                if (team_.isMoveReserved(b.x, b.y, by) && by != -1 && by != tid)
                    break;

                int rdx = (int)b.x - (int)a.x;
                int rdy = (int)b.y - (int)a.y;
                int dx = unitDeltaWrapped(rdx, wv.w);
                int dy = unitDeltaWrapped(rdy, wv.h);

                bi.plan_dx[wrote] = (std::int8_t)dx;
                bi.plan_dy[wrote] = (std::int8_t)dy;
                bi.plan_shoot[wrote] = 0;
                ++wrote;
            }
            bi.plan_len = (std::uint8_t)wrote;
        }

        // Aim toward waypoint; opportunistic shot if that ray happens to hit an enemy cleanly
        if (bi.dir_dx != 0 || bi.dir_dy != 0)
            if (firstHitKindAlongDir(my_pos, bi.dir_dx, bi.dir_dy) == 'E')
                bi.shoot_when_aligned = true;

        // ---------- Reservations ----------
        int friends_alive = 0;
        for (std::size_t y = 0; y < wv.h; ++y)
            for (std::size_t x = 0; x < wv.w; ++x)
                if (wv.hasFriend(x, y))
                    ++friends_alive;
        const std::uint8_t base_ttl = (std::uint8_t)std::max(1, friends_alive - 1);

        if (path.size() >= 2 && bi.plan_len > 0)
        {
            const int reserve_steps = std::min<int>(bi.plan_len, (int)path.size() - 1);
            for (int i = 0; i < reserve_steps; ++i)
            {
                const Cell dest = path[i + 1];
                team_.reserveMove(dest.x, dest.y, tid, (std::uint8_t)(base_ttl + i));
            }
        }
        else
        {
            team_.reserveMove(my_pos.x, my_pos.y, tid, base_ttl);
        }

        // ---------- Push to tank ----------
        tank.updateBattleInfo(bi);
    }

}
