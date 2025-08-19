#pragma once

#include "AbstractMode.h"



class ComparativeMode: public AbstractMode {
    public:
    ~ComparativeMode() override = default;
    std::vector<GameArgs> getAllGames(std::vector<std::string> game_maps) override ;
        
};