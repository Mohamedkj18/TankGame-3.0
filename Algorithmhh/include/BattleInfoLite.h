#pragma once
#include <cstddef>
#include <cstdint>
#include "common/ActionRequest.h"
#include "common/BattleInfo.h"
#include "Types.h" // Cell, RoleTag

namespace Algorithm_212788293_212497127
{

    // Lightweight, per-tick order from Player → Tank
    struct BattleInfoLite : public BattleInfo
    {
        // Debug/trace
        RoleTag tag{RoleTag::Aggressor};
        Cell waypoint{0, 0};

        // Motor (single-step fallback)
        int dir_dx{0}; // each in {-1,0,1}
        int dir_dy{0};
        bool shoot_when_aligned{false};

        // NEW: short action script (plan) the tank can execute without re-contact.
        static constexpr int BI_MAX_STEPS = 4;
        std::uint8_t plan_len{0};
        std::int8_t plan_dx[BI_MAX_STEPS]{}; // each in {-1,0,1}
        std::int8_t plan_dy[BI_MAX_STEPS]{};
        std::uint8_t plan_shoot[BI_MAX_STEPS]{}; // 0 = move, 1 = shoot
    };

} // namespace Algorithm_212788293_212497127
