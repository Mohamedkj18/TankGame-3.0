#pragma once

#include "Mode.h"
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