#pragma once

#include "Mode.h"



class ComparativeMode: public Mode {
    public:
    ~ComparativeMode() override = default;
    std::vector<GameArgs> getAllGames(std::vector<std::string> game_maps) override ;
        
};