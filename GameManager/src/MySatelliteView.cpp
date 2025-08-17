#include <cstddef>
#include "MySatelliteView.h"

namespace GameManager_212788293_212497127
{
    MySatelliteView::MySatelliteView(int tankPos,
                                     const std::map<int, std::unique_ptr<Tank>> &tanks,
                                     const std::unordered_map<int, std::unique_ptr<Shell>> &shells,
                                     const std::set<int> &mines,
                                     const std::unordered_map<int, Wall> &walls)
        : tankPos(tankPos), tanks(tanks), shells(shells), mines(mines), walls(walls) {}

    char MySatelliteView::getObjectAt(size_t x, size_t y) const
    {
        int bijectionIndex = UC::bijection(2 * x, 2 * y);
        if (bijectionIndex == tankPos)
            return '%'; // Current Tank
        if (tanks.count(bijectionIndex))
            return (char)('0' + tanks.at(bijectionIndex)->getPlayerId());
        if (shells.count(bijectionIndex))
            return '*'; // Shell
        if (mines.count(bijectionIndex))
            return '@'; // Mine
        if (walls.count(bijectionIndex))
            return '#'; // Wall

        return ' ';
    }
}