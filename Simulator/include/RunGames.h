#pragma once

#include "AbstractMode.h"
#include "ComparativeMode.h"
#include "CompetitionMode.h"
#include "GameManagerRegistrar.h"
#include "AlgorithmRegistrar.h"
#include "common/GameResult.h"
#include <thread>
#include <atomic>

struct RanGame {
    std::string gm_name;
    std::string map_name;
    size_t algo1_id, algo2_id;
    GameResult result;
    std::string gameFinalState;
};

TankAlgorithmFactory make_tank_factory(size_t algo_id);
std::unique_ptr<Player> make_player(size_t algo_id, int player_index, size_t x, size_t y, size_t max_steps, size_t num_shells);
RanGame run_single_game(const GameArgs& g, bool verbose);
void openSOFilesCompetitionMode(Cli cli, std::vector<LoadedLib>& algoLibs, std::vector<LoadedLib>& gmLibs);
std::string satelliteViewToString(const SatelliteView& view, size_t width, size_t height);
void runThreads(std::unique_ptr<AbstractMode>& mode, std::vector<GameArgs> jobs, int num_threads, bool verbose);
void runAllGames(std::unique_ptr<AbstractMode>& mode, std::vector<GameArgs> jobs, bool verbose);
std::unique_ptr<AbstractMode> createMode(Cli cli, std::vector<std::string> &maps);
void runModeResults(AbstractMode* mode, Cli& cli);
