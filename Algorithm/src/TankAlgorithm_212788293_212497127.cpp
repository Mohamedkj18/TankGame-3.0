#include "TankAlgorithm_212788293_212497127.h"

namespace Algorithm_212788293_212497127
{

    // E, NE, N, NW, W, SW, S, SE (must match rotation semantics)
    static constexpr int DIR8[8][2] = {
        {+1, 0}, {+1, +1}, {0, +1}, {-1, +1}, {-1, 0}, {-1, -1}, {0, -1}, {+1, -1}};

    TankAlgorithm_212788293_212497127::TankAlgorithm_212788293_212497127(int p, int t) noexcept
        : player_idx_(p), tank_idx_(t) {}

    void TankAlgorithm_212788293_212497127::updateBattleInfo(::BattleInfo &info)
    {
        if (auto *p = dynamic_cast<BattleInfoLite *>(&info))
        {
            bi_ = *p;
            have_bi_ = true;
            plan_idx_ = 0; // start script from the beginning each time
        }
        else
        {
            have_bi_ = false; // unknown info type; fail safe
        }
    }

    int TankAlgorithm_212788293_212497127::dirIndexFromDelta(int dx, int dy)
    {
        dx = (dx > 0) - (dx < 0);
        dy = (dy > 0) - (dy < 0);
        // map to DIR8 index
        if (dx == +1 && dy == 0)
            return 0;
        if (dx == +1 && dy == +1)
            return 1;
        if (dx == 0 && dy == +1)
            return 2;
        if (dx == -1 && dy == +1)
            return 3;
        if (dx == -1 && dy == 0)
            return 4;
        if (dx == -1 && dy == -1)
            return 5;
        if (dx == 0 && dy == -1)
            return 6;
        if (dx == +1 && dy == -1)
            return 7;
        return 0; // default east for (0,0)
    }

    ::ActionRequest TankAlgorithm_212788293_212497127::rotateStepToward(int fromIdx, int toIdx)
    {
        int cw = (toIdx - fromIdx + 8) % 8;  // right turns
        int ccw = (fromIdx - toIdx + 8) % 8; // left turns
        // deterministic tie-break: prefer LEFT on equal distance
        return (ccw <= cw) ? ::ActionRequest::RotateLeft45 : ::ActionRequest::RotateRight45;
    }

    ::ActionRequest TankAlgorithm_212788293_212497127::getAction()
    {
        if (!have_bi_)
        {
            // If your ActionRequest has GetBattleInfo, you can return it here.
            // Otherwise, we rotate deterministically to keep progress.
            // return ::ActionRequest::GetBattleInfo;
            return ::ActionRequest::RotateLeft45;
        }

        // Prefer running the short script (plan) if present
        if (bi_.plan_len > 0 && plan_idx_ < bi_.plan_len)
        {
            const int dx = static_cast<int>(bi_.plan_dx[plan_idx_]);
            const int dy = static_cast<int>(bi_.plan_dy[plan_idx_]);
            const bool sh = (bi_.plan_shoot[plan_idx_] != 0);
            const int want = dirIndexFromDelta(dx, dy);

            if (want != facing_idx_)
            {
                auto ar = rotateStepToward(facing_idx_, want);
                if (ar == ::ActionRequest::RotateLeft45)
                    facing_idx_ = (facing_idx_ + 7) % 8;
                if (ar == ::ActionRequest::RotateRight45)
                    facing_idx_ = (facing_idx_ + 1) % 8;
                return ar; // stay on this plan step until aligned
            }

            // aligned → perform this scripted step
            ::ActionRequest ar = sh ? ::ActionRequest::Shoot : ::ActionRequest::MoveForward;

            // advance to next step; if done, we’ll ask for BI next call
            if (++plan_idx_ >= bi_.plan_len)
            {
                have_bi_ = false;
                plan_idx_ = 0;
            }
            return ar;
        }

        // Fallback to single-step motor if no script provided
        const int want = dirIndexFromDelta(bi_.dir_dx, bi_.dir_dy);
        if (want != facing_idx_)
        {
            auto ar = rotateStepToward(facing_idx_, want);
            if (ar == ::ActionRequest::RotateLeft45)
                facing_idx_ = (facing_idx_ + 7) % 8;
            if (ar == ::ActionRequest::RotateRight45)
                facing_idx_ = (facing_idx_ + 1) % 8;
            return ar;
        }

        // aligned
        ::ActionRequest ar = bi_.shoot_when_aligned ? ::ActionRequest::Shoot
                                                    : ::ActionRequest::MoveForward;
        // single-step orders are "one-and-done"
        have_bi_ = false;
        return ar;
    }

} // namespace Algorithm_212788293_212497127
