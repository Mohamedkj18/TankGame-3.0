#ifndef ALG_212788293_212497127_TYPES_H
#define ALG_212788293_212497127_TYPES_H
#include <cstddef>

namespace Algorithm_212788293_212497127
{
    enum class RoleTag
    {
        Aggressor,
        Anchor,
        Survivor,
        Flanker
    };

    struct TankLocal
    {
        int player_idx{};
        int tank_idx{};
        std::size_t x{}, y{};
        int facing_deg{}; // optional; wire later
    };

    struct Cell
    {
        std::size_t x{}, y{};
    };

    using TargetId = int;
} // namespace Algorithm_212788293_212497127
#endif
