#pragma once

#include <iostream>
#include <string>
#include <vector>
#include "common/SatelliteView.h"
#include "common/Player.h"
#include "common/AbstractGameManager.h"
#include "GameManagerRegistrar.h"
#include "AlgorithmRegistrar.h"
#include "InitialSatellite.h"
#include <set>
#include <utility>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <memory>

struct GameArgs{
    size_t map_width,map_height,max_steps,num_shells;
    std::unique_ptr<SatelliteView> map;
    std::string map_name ,GameManagerName,player1Name,player2Name;
    size_t playerAndAlgoFactory1ID, playerAndAlgoFactory2ID, GameManagerID;
};


struct ParsedMap {
    size_t map_width{};
    size_t map_height{};
    size_t max_steps{};
    size_t num_shells{};
    std::set<std::pair<size_t, size_t>> player1tanks;
    std::set<std::pair<size_t, size_t>> player2tanks;
    std::set<std::pair<size_t, size_t>> mines;
    std::set<std::pair<size_t, size_t>> walls;
};


class Mode{
public:
    virtual ~Mode() = default;
    virtual std::vector<GameArgs> getAllGames(std::vector<std::string> game_maps) =0;
    ParsedMap parseBattlefieldFile(const std::string& filename);
};