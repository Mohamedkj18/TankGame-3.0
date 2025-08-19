#include "CompetitionMode.h"

std::vector<GameArgs> CompetitionMode::getAllGames(std::vector<std::string> game_maps) {
    // This function retrieves all games for the competition mode based on the provided game maps.
    std::vector<GameArgs> games;
    

    // Retrieve the game manager factory
    auto& gameManagerRegistrar = GameManagerRegistrar::getGameManagerRegistrar();
    auto& algorithmRegistrar = AlgorithmRegistrar::getAlgorithmRegistrar();
    if(gameManagerRegistrar.gameManagers.empty()) {
        throw std::runtime_error("No game managers registered.");
    }
    std::string gameManagerName = gameManagerRegistrar.begin()->second.name();
    int algoCount = algorithmRegistrar.getAlgoID();
    // Iterate through all registered game managers
    for(int i=0; i<(int)game_maps.size(); i++) {
        std::set<std::pair<size_t, size_t>> assignedGames;
        const auto& game_map = game_maps[i];
        // Parse the map file to get the initial state
        ParsedMap parsedMap = parseBattlefieldFile(game_map);
        
        // Create an InitialSatellite object with the parsed map data
        auto satellite = std::make_unique<InitialSatellite>(parsedMap.player1tanks, parsedMap.player2tanks, parsedMap.walls, parsedMap.mines);

        for(size_t j=0; j<algoCount; j++) {
            // Create game arguments for each algorithm
            size_t k = (i+j+1)%(algoCount-1);
            if (k == j) continue; // Skip if both algorithms are the same
            if (assignedGames.count({k, j}) > 0 || assignedGames.count({j, k}) > 0 ) continue; // Skip if already assigned
            games.push_back({parsedMap.map_width, parsedMap.map_height, parsedMap.max_steps, parsedMap.num_shells,
                std::move(satellite), // Use the InitialSatellite object
                game_map, // Map name // Player 2 algorithm factory name
                gameManagerName, 
                algorithmRegistrar.getPlayerAndAlgoFactory(k).name(),
                algorithmRegistrar.getPlayerAndAlgoFactory(j).name(),
                k, j, 0
            });
        }
    }
    return games;
}