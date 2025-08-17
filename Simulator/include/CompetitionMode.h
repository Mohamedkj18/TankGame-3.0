#pragma once

#include "Mode.h"



class CompetitionMode: public Mode {
    public:
    std::vector<GameArgs> getAllGames(std::vector<std::string> game_maps) override ;
        
};