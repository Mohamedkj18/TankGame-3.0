#ifndef ALG_212788293_212497127_ROLE_BASE_H
#define ALG_212788293_212497127_ROLE_BASE_H
#include "Types.h"
#include "Plan.h"

namespace Algorithm_212788293_212497127
{

    struct WorldView;
    struct TeamState;

    class Role
    {
    public:
        virtual ~Role() = default;
        virtual RoleTag tag() const = 0;
        virtual Plan plan(const WorldView &wv,
                          const TeamState &team,
                          int tank_index) = 0;
    };

} // namespace Algorithm_212788293_212497127
#endif
