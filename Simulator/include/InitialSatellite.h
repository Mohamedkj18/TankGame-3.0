#pragma once

#include "common/SatelliteView.h"
#include <set>

class InitialSatellite : public SatelliteView {
    private:
        std::set<std::pair<size_t,size_t>> player1Tanks;
        std::set<std::pair<size_t,size_t>> player2Tanks;
        std::set<std::pair<size_t,size_t>> walls;
        std::set<std::pair<size_t,size_t>> mines;
    public:
        InitialSatellite(std::set<std::pair<size_t,size_t>> player1Tanks
        ,std::set<std::pair<size_t,size_t>> player2Tanks
        ,std::set<std::pair<size_t,size_t>> walls
        ,std::set<std::pair<size_t,size_t>> mines) : player1Tanks(player1Tanks), 
        player2Tanks(player2Tanks), walls(walls), mines(mines) {}

        char getObjectAt(size_t x, size_t y) const override ;
    };