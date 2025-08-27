#ifndef ALG_212788293_212497127_PATHFINDING_H
#define ALG_212788293_212497127_PATHFINDING_H

#include "Types.h"
#include <vector>

namespace Algorithm_212788293_212497127
{
    struct WorldView;

    // Phase-1: declare; implement in src/core/Pathfinding.cpp when ready
    std::vector<Cell> aStar(const WorldView &wv, Cell from, Cell to);

} // namespace Algorithm_212788293_212497127
#endif
