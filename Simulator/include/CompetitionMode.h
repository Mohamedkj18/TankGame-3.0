#pragma once

#include "Mode.h"



class CompetitionMode: public Mode {
    public:
    ~CompetitionMode() override = default;
    std::vector<GameArgs> getAllGames(std::vector<std::string> game_maps) override ;
        
};