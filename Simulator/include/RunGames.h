#pragma once

#include "AbstractMode.h"
#include "ComparativeMode.h"
#include "CompetitionMode.h"
#include "GameManagerRegistrar.h"
#include "AlgorithmRegistrar.h"
#include "common/GameResult.h"

struct RanGame {
    std::string gm_name;
    std::string map_name;
    size_t algo1_id, algo2_id;
    GameResult result;
};

TankAlgorithmFactory make_tank_factory(size_t algo_id);
std::unique_ptr<Player> make_player(size_t algo_id, int player_index, size_t x, size_t y, size_t max_steps, size_t num_shells);
RanGame run_single_game(const GameArgs& g, bool verbose);