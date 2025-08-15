
#include "include/Mode.h"
#include "include/ComparativeMode.h"
#include "include/CompetitionMode.h"
#include "include/GameManagerRegistrar.h"
#include "include/AlgorithmRegistrar.h"
#include "common/GameResult.h"

struct RanGame {
    std::string gm_name;
    std::string map_name;
    size_t algo1_id, algo2_id;
    GameResult result;
};