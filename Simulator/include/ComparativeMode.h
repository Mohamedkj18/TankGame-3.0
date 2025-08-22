#pragma once

#include "AbstractMode.h"


struct ComparativeKey {
    int winner;
    GameResult::Reason reason;
    size_t rounds;
    std::string finalBoard;

    bool operator==(const ComparativeKey& o) const {
        return winner == o.winner &&
               reason == o.reason &&
               rounds == o.rounds &&
               finalBoard == o.finalBoard;
    }
};

struct ComparativeKeyHash {
    size_t operator()(const ComparativeKey& k) const noexcept {
        size_t h1 = std::hash<int>{}(k.winner);
        size_t h2 = std::hash<int>{}(static_cast<int>(k.reason));
        size_t h3 = std::hash<size_t>{}(k.rounds);
        size_t h4 = std::hash<std::string>{}(k.finalBoard);
        return (((h1 ^ (h2 << 1)) >> 1) ^ (h3 << 1)) ^ h4;
    }
};



class ComparativeMode: public AbstractMode {
    // Key = outcome, Value = list of GM names that produced it
    std::unordered_map<ComparativeKey, std::vector<std::string>, ComparativeKeyHash> gmNamesAndResults;
    std::unordered_map<ComparativeKey, ComparativeKey, ComparativeKeyHash> gamesResults;
    std::mutex clusters_mtx;
    public:
    ~ComparativeMode() override = default;
    std::vector<GameArgs> getAllGames(std::vector<std::string> game_maps) override ;
    int openSOFiles(Cli cli, std::vector<LoadedLib> algoLibs, std::vector<LoadedLib> gmLibs) override;
    int register2Algorithms(Cli cli, std::vector<LoadedLib> algoLibs);
    int registerGameManagers(Cli cli, std::vector<LoadedLib> gmLibs);
    void applyCompetitionScore(const GameArgs& g, GameResult res, std::string finalGameState) override ;
    void writeComparativeResults(const std::string& game_managers_folder, const std::string& game_map_filename, const std::string& algorithm1_so, const std::string& algorithm2_so);

     
    
};