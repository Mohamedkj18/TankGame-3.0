#pragma once
#include "Types.h"          // Cell
#include "BattleInfoLite.h" // RoleTag

namespace Algorithm_212788293_212497127
{

    struct WorldView; // fwd-decl (full type only needed in .cpp)

    // A tiny, pure “where to go” module (no side effects).
    // Each function returns a PASSABLE target cell for the given role.

    Cell targetForAggressor(const WorldView &wv, Cell me);
    Cell targetForAnchor(const WorldView &wv, Cell me);
    Cell targetForFlanker(const WorldView &wv, Cell me);
    Cell targetForSurvivor(const WorldView &wv, Cell me);

    // Convenience dispatcher (deterministic)
    inline Cell targetForRole(RoleTag tag, const WorldView &wv, Cell me)
    {
        switch (tag)
        {
        case RoleTag::Aggressor:
            return targetForAggressor(wv, me);
        case RoleTag::Anchor:
            return targetForAnchor(wv, me);
        case RoleTag::Flanker:
            return targetForFlanker(wv, me);
        case RoleTag::Survivor:
            return targetForSurvivor(wv, me);
        default:
            return targetForAggressor(wv, me);
        }
    }

} // namespace Algorithm_212788293_212497127
