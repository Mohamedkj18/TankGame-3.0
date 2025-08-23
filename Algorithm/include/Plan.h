#ifndef ALG_212788293_212497127_PLAN_H
#define ALG_212788293_212497127_PLAN_H

#include "Types.h"
#include <optional>

namespace Algorithm_212788293_212497127
{

    struct Plan
    {
        RoleTag tag{};
        Cell waypoint{};
        std::optional<Cell> secondary{};
        std::optional<TargetId> focus{};
        std::size_t valid_until_tick{0};
    };

} // namespace Algorithm_212788293_212497127
#endif
