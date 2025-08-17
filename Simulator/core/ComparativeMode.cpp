#include "ComparativeMode.h"


std::vector<GameArgs> ComparativeMode::getAllGames(std::vector<std::string> game_maps) {
    // This function retrieves all games for the comparative mode based on the provided game map.
    std::vector<GameArgs> games;

    // Retrieve the game manager factory
    auto& gameManagerRegistrar = GameManagerRegistrar::getGameManagerRegistrar();
    auto& algorithmRegistrar = AlgorithmRegistrar::getAlgorithmRegistrar();
    size_t factoryAndPlayer1Name = algorithmRegistrar.begin()->first;
    size_t factoryAndPlayer2Name = algorithmRegistrar.rbegin()->first;
    std::string player1Name = algorithmRegistrar.begin()->second.name();
    std::string player2Name = algorithmRegistrar.rbegin()->second.name();
    // Iterate through all registered game managers
    struct ParsedMap parsedMap = parseBattlefieldFile(game_maps[0]);
    auto satellite = std::make_unique<InitialSatellite> (parsedMap.player1tanks, parsedMap.player2tanks, parsedMap.walls, parsedMap.mines);
    for (size_t i = 0; i < gameManagerRegistrar.getGameManagerCount(); ++i) {
        if(gameManagerRegistrar.gameManagers.count(i)){
        auto& gameManagerFactory = gameManagerRegistrar.gameManagers.at(i);
        if (gameManagerFactory.hasFactory()) 
            games.push_back({
                parsedMap.map_width,
                parsedMap.map_height,
                parsedMap.max_steps,
                parsedMap.num_shells,
                std::move(satellite), // Satellite view of the initial state
                game_maps[0], // Map name
                gameManagerFactory.name(), // Game manager name
                player1Name, // Player 1 name
                player2Name, // Player 2 name
                factoryAndPlayer1Name, // Player 1 algorithm factory name
                factoryAndPlayer2Name,
                i // Game manager ID 
            });
        }}
    return games;
}