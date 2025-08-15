#include "Mode.h"



class ComparativeMode: public Mode {
    public:
    std::vector<GameArgs> getAllGames(std::vector<std::string> game_maps) override ;
        
};