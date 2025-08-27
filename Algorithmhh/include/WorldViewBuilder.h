#pragma once
#include <cstddef>
#include "WorldView.h"

class SatelliteView; // from common/

namespace Algorithm_212788293_212497127
{

    // Build a snapshot from SatelliteView::getObjectAt.
    // W,H are the battlefield dimensions you received in Player's ctor.
    // my_player_index is 1 or 2 (per assignment).
    WorldView buildWorldView(const ::SatelliteView &sat,
                             std::size_t W, std::size_t H,
                             int my_player_index);

} // namespace Algorithm_212788293_212497127
