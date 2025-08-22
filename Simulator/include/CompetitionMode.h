#pragma once

#include "AbstractMode.h"



class CompetitionMode: public AbstractMode {

    std::map<std::string, std::atomic<size_t>> algoNamesAndScores;

    public:
    ~CompetitionMode() override = default;
    std::vector<GameArgs> getAllGames(std::vector<std::string> game_maps) override ;
    int openSOFiles(Cli cli, std::vector<LoadedLib> algoLibs, std::vector<LoadedLib> gmLibs) override;
    void applyCompetitionScore(const GameArgs& g, GameResult res, std::string finalGameState) override ;
    void add_relaxed(std::atomic<size_t>& x, size_t d);
    std::vector<std::pair<std::string, size_t>> build_sorted_score_table();
    void writeCompetitionResults(const std::string& algorithms_folder, const std::string& game_maps_folder, const std::string& game_manager_so);

};