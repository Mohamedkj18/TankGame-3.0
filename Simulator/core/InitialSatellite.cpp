#include "InitialSatellite.h"

char InitialSatellite::getObjectAt(size_t x, size_t y) const
{
    if (player1Tanks.count({x, y}))
        return '1';
    if (player2Tanks.count({x, y}))
        return '2';
    if (walls.count({x, y}))
        return '#';
    if (mines.count({x, y}))
        return '@';
    return ' '; // Empty space
}
