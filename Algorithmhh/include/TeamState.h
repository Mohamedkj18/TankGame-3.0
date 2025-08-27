#pragma once
#include <vector>
#include <cstddef>
#include <cstdint>

namespace Algorithm_212788293_212497127
{

    struct TeamState
    {
        std::size_t w{0}, h{0};

        // movement reservations: -1 = free, else tank_id that owns the cell
        std::vector<int> move_owner;   // size w*h
        std::vector<uint8_t> move_ttl; // size w*h

        // shot lanes: bit 1 means "friend plans to shoot through this cell"
        std::vector<uint8_t> shot_ttl; // size w*h (stores TTL counters)

        void ensure(std::size_t W, std::size_t H)
        {
            if (W == w && H == h)
                return;
            reset(W, H);
        }

        // New: full reset (clears all reservations/lanes)
        void reset(std::size_t W, std::size_t H)
        {
            w = W;
            h = H;
            move_owner.assign(W * H, -1);
            move_ttl.assign(W * H, 0);
            shot_ttl.assign(W * H, 0);
        }

        // New: clear without changing size
        void clearReservations()
        {
            std::fill(move_owner.begin(), move_owner.end(), -1);
            std::fill(move_ttl.begin(), move_ttl.end(), 0);
            std::fill(shot_ttl.begin(), shot_ttl.end(), 0);
        }

        inline std::size_t idx(std::size_t x, std::size_t y) const { return y * w + x; }

        // Age all TTLs by one (call once at the top of every Player::updateTankWithBattleInfo)
        void age()
        {
            for (auto &t : move_ttl)
                if (t)
                    --t;
            for (auto &t : shot_ttl)
                if (t)
                    --t;
            for (std::size_t i = 0; i < move_owner.size(); ++i)
                if (move_ttl[i] == 0)
                    move_owner[i] = -1;
        }

        // Reserve movement (cell), with TTL in {1..255}. Lower tank_id wins ties deterministically.
        bool reserveMove(std::size_t x, std::size_t y, int tank_id, uint8_t ttl)
        {
            const auto i = idx(x, y);
            if (move_ttl[i] == 0)
            {
                move_owner[i] = tank_id;
                move_ttl[i] = ttl;
                return true;
            }
            if (tank_id < move_owner[i])
            {
                move_owner[i] = tank_id;
                move_ttl[i] = ttl;
                return true;
            }
            return false;
        }
        bool isMoveReserved(std::size_t x, std::size_t y, int &by_tank) const
        {
            const auto i = idx(x, y);
            by_tank = move_owner[i];
            return move_ttl[i] != 0;
        }

        // Mark shot cells along a ray
        void markShot(std::size_t x, std::size_t y, uint8_t ttl)
        {
            shot_ttl[idx(x, y)] = ttl ? ttl : 1;
        }
        bool hasShot(std::size_t x, std::size_t y) const
        {
            return shot_ttl[idx(x, y)] != 0;
        }
    };

} // namespace Algorithm_212788293_212497127
