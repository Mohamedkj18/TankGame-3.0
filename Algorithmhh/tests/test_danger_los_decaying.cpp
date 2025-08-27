#include <cassert>
#include <iostream>
#include "WorldView.h"
#include "Danger.h"

using namespace Algorithm_212788293_212497127;

int main()
{
    WorldView wv{12, 5};
    wv.setMask(2, 2, WorldView::ENEMY); // no walls to keep it simple

    // base=8, decay=1, max_range=5, cardinals only
    seedEnemyLOSDanger(wv, /*base=*/8, /*decay=*/1, /*max_range=*/5, /*include_diagonals=*/false);

    // East: distances 1..5 from (2,2)
    assert(wv.getDanger(3, 2) == 8);
    assert(wv.getDanger(4, 2) == 7);
    assert(wv.getDanger(5, 2) == 6);
    assert(wv.getDanger(6, 2) == 5);
    assert(wv.getDanger(7, 2) == 4);

    // West wrap: 1..5 to the left
    assert(wv.getDanger(1, 2) == 8);
    assert(wv.getDanger(0, 2) == 7);
    assert(wv.getDanger(11, 2) == 6);
    assert(wv.getDanger(10, 2) == 5);
    assert(wv.getDanger(9, 2) == 4);

    std::cout << "LOS decaying: OK\n";
    return 0;
}
