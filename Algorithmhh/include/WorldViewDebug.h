#pragma once
#include <ostream>
#include <vector>
#include "WorldView.h"
#include "Types.h"

namespace Algorithm_212788293_212497127
{

    // Print a textual board for the given WorldView.
    // friendCh/enemyCh let you match the simulator's glyphs ('1','2' etc.)
    void printWorldView(const WorldView &wv,
                        std::ostream &out,
                        char friendCh = 'F',
                        char enemyCh = 'E');

    // Optional: overlay a path (e.g., from aStar) and/or labeled waypoints.
    void printWorldViewWithPath(const WorldView &wv,
                                const std::vector<Cell> &path,
                                std::ostream &out,
                                char friendCh = 'F',
                                char enemyCh = 'E');

} // namespace Algorithm_212788293_212497127
