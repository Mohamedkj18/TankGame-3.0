#include "include/CompetitionMode.h"

std::vector<GameArgs> CompetitionMode::getAllGames(std::vector<std::string> game_maps) {
    // This function retrieves all games for the competition mode based on the provided game maps.
    std::vector<GameArgs> games;

    // Retrieve the game manager factory
    auto& gameManagerRegistrar = GameManagerRegistrar::getGameManagerRegistrar();
    auto& algorithmRegistrar = AlgorithmRegistrar::getAlgorithmRegistrar();

    std::string gameManagerName = gameManagerRegistrar.begin()->second.name();

    int algoCount = algorithmRegistrar.getAlgoID();
    // Iterate through all registered game managers
    for(int i=0; i<game_maps.size(); ++i) {
        const auto& game_map = game_maps[i];
        // Parse the map file to get the initial state
        ParsedMap parsedMap = parseBattlefieldFile(game_map);
        
        // Create an InitialSatellite object with the parsed map data
        auto satellite = std::make_unique<InitialSatellite>(
            parsedMap.player1tanks,
            parsedMap.player2tanks,
            parsedMap.walls,
            parsedMap.mines
        );

        for(int j=0; j<algoCount; ++j) {
            // Create game arguments for each algorithm
            games.push_back({
                parsedMap.map_width,
                parsedMap.map_height,
                parsedMap.max_steps,
                parsedMap.num_shells,
                std::move(satellite), // Use the InitialSatellite object
                game_map, // Map name // Player 2 algorithm factory name
                gameManagerName, 
                algorithmRegistrar.getPlayerAndAlgoFactory((size_t)(i+j+1)%(algoCount-1)).name(),
                algorithmRegistrar.getPlayerAndAlgoFactory((size_t)j).name(),
                (size_t)(i+j+1)%(algoCount-1),
                 (size_t)j
            });
        }
    }
    return games;
}