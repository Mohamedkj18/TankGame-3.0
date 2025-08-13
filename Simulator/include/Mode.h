#include <iostream>
#include <string>
#include <vector>
#include "common/SatelliteView.h"
#include "common/Player.h"
#include "common/AbstractGameManager.h"
#include "GameManagerRegistrar.h"
#include "AlgorithmRegistrar.h"


struct GameArgs{
    size_t map_width,map_height,max_steps,num_shells;
    SatelliteView& map;
    std::string map_name, playerAndAlgoFactory1Name, playerAndAlgoFactory2Name
    ,GameManagerName, player1Name, player2Name;
};

class Mode{
public:
    virtual std::vector<GameArgs> getAllGames() =0;
};