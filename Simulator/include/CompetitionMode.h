#pragma once

#include "AbstractMode.h"



class CompetitionMode: public AbstractMode {
    public:
    ~CompetitionMode() override = default;
    std::vector<GameArgs> getAllGames(std::vector<std::string> game_maps) override ;
        
};