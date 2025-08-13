#include "Mode.h"



class CompetitionMode: public Mode {
    public:
    std::vector<GameArgs> getAllGames() override ;
        
};