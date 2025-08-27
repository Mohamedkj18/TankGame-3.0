#include <iostream>
#include <cassert>
#include "WorldView.h"
#include "WorldViewDebug.h"
#include "LOS.h"
#include "TeamState.h"

using namespace Algorithm_212788293_212497127;

int main()
{
    WorldView wv{12, 5};
    TeamState tv;
    tv.ensure(wv.w, wv.h);
    TankLocal me{1, 0, 0, 0, 0};

    // Me at (2,2), facing east. Enemy at (9,2). Path cells: 3,4,5,6,7,8,9
    wv.setMask(2, 2, WorldView::FRIEND);
    me.x = 2;
    me.y = 2;
    me.facing_deg = 0;
    wv.setMask(9, 2, WorldView::ENEMY);

    // Reserve a friendly move into (5,2) this step — should block our shot
    const int friend_id = 0; // lower id wins ties; any id is fine here
    const uint8_t ttl = 5;
    tv.reserveMove(5, 2, friend_id, ttl);

    std::cout << "\n=== LOS teamstate: friend reserved ahead ===\n";
    printWorldView(wv, std::cout, 'F', 'E');
    assert(!firstHitIsEnemyFacing(wv, tv, me) && "Reservation must block shooting");

    // If we clear reservation, shot should be allowed
    tv.clearReservations();

    // brute-clear reservation for the test
    tv.ensure(wv.w, wv.h); // resets all reservations
    assert(firstHitIsEnemyFacing(wv, tv, me) && "No reservation, enemy should be first hit");

    std::cout << "LOS teamstate: OK\n";
    return 0;
}
