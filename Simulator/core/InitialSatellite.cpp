#include "InitialSatellite.h"

char InitialSatellite::getObjectAt(size_t x, size_t y) const {
    if (player1Tanks.count({x, y})) return '1'; // Player 1 tank
    if (player2Tanks.count({x, y})) return '2'; // Player 2 tank
    if (walls.count({x, y})) return '#'; // Wall
    if (mines.count({x, y})) return '@'; // Mine
    return ' '; // Empty space
}


