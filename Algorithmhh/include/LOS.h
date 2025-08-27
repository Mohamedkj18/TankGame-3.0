#ifndef ALG_212788293_212497127_LOS_H
#define ALG_212788293_212497127_LOS_H

#include <cstddef>
#include <cstdint>
#include "Types.h"
#include "WorldView.h"
#include "TeamState.h"

namespace Algorithm_212788293_212497127
{

    struct WorldView;
    struct TeamState;

    enum class HitType : std::uint8_t
    {
        None,
        Wall,
        Friend,
        Enemy,
        Shell,
        Mine
    };

    struct RayHit
    {
        HitType type{HitType::None};
        std::size_t x{0}, y{0};
        std::size_t steps{0}; // how many steps from origin to the hit cell
    };

    // Map a facing angle (multiples of 45°) to a unit step (dx,dy) on the grid.
    // 0°→(+1,0), 90°→(0,+1), 180°→(-1,0), 270°→(0,-1); diagonals for 45/135/225/315.
    void stepFromFacing45(int facing_deg, int &dx, int &dy);

    // Raycast from (x,y) along integer step (dx,dy) on a torus, returning the first blocking entity.
    // Deterministic: fixed check order and fixed step order. max_steps=0 → computed to cover one full cycle.
    RayHit raycastFirstHit(const WorldView &wv,
                           std::size_t x, std::size_t y,
                           int dx, int dy,
                           std::size_t max_steps = 0);

    // Convenience: “would I hit an enemy if I shoot straight ahead?”
    bool firstHitIsEnemyFacing(const WorldView &wv, const TeamState &tv, const TankLocal &me);

    // Convenience (rarely needed): test if the very next cell in front is safe to move into.
    inline bool nextStepBlocked(const WorldView &wv, const TankLocal &me)
    {
        int dx = 0, dy = 0;
        stepFromFacing45(me.facing_deg, dx, dy);
        const std::size_t nx = wv.wrapX(static_cast<long long>(me.x) + dx);
        const std::size_t ny = wv.wrapY(static_cast<long long>(me.y) + dy);
        // Treat walls, mines, and friend cells as blocking for movement.
        return wv.isWall(nx, ny) || wv.isMine(nx, ny) || wv.hasFriend(nx, ny);
    }

} // namespace Algorithm_212788293_212497127
#endif
