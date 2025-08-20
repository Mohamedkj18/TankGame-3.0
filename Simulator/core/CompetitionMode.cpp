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
    size_t algoCount = algorithmRegistrar.getAlgoID();
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


int CompetitionMode::openSOFiles(Cli cli ,std::vector<LoadedLib> algoLibs, std::vector<LoadedLib> gmLibs) {
    auto& gmReg = GameManagerRegistrar::getGameManagerRegistrar();
    gmReg.initializeGameManagerCount();
    auto& algoReg = AlgorithmRegistrar::getAlgorithmRegistrar();
    algoReg.initializeAlgoID();
    std::cout << "Registering GameManager: " << cli.kv["game_manager"] << "\n";

    if (!file_exists(cli.kv["game_manager"])) {
        usage("game_manager not found: " + cli.kv["game_manager"]);
        return 1;
    }
    gmReg.createGameManagerFactoryEntry(fs::path(cli.kv["game_manager"]).stem().string());
    std::string err;
    LoadedLib lib;
    if (!dlopen_self_register(cli.kv["game_manager"], lib, err)) {
        std::cerr << "Failed to load GameManager shared object: " << cli.kv["game_manager"] << "\nError: " << err << "\n";
        return 1;
    }
    gmReg.validateLastRegistration();
    gmReg.updateGameManagerCount();
    gmLibs.push_back(lib);
    std::cout << "Registered GameManager: " << gmReg.getGameManagerFactory(gmReg.getGameManagerCount() - 1).name() << "\n";

    for(const auto& algoSO : list_shared_objects(cli.kv["algorithms_folder"])) {
        std::string err;
        LoadedLib lib;
        std::string algoName = fs::path(algoSO).stem().string();
        algoReg.createAlgorithmFactoryEntry(algoName);
        if (!dlopen_self_register(algoSO, lib, err)) {
            std::cerr << "Failed to load Algorithm shared object: " << algoSO << "\nError: " << err << "\n";
            continue;
        }
        algoReg.validateLastRegistration();
        algoReg.updateAlgoID();
        algoLibs.push_back(lib);
        algoNamesAndScores[algoName] = 0; // Initialize score for this algorithm
        std::cout << "Registered Algorithm: " << algoReg.getPlayerAndAlgoFactory(algoReg.getAlgoID() - 1).name() << "\n";
    }
    if (algoReg.count() < 2) {
        usage("algorithms_folder must contain at least two algorithms.");
        return 1;
    }
    return 0;    
}


void add_relaxed(std::atomic<size_t>& x, size_t d) {
    x.fetch_add(d, std::memory_order_relaxed);
}

void CompetitionMode::applyCompetitionScore(const GameArgs& g, GameResult r) {
    const std::string a1 = g.player1Name; // your IDs from job building
    const std::string a2 = g.player2Name;
    switch (r.winner) {
        case 1:  add_relaxed(algoNamesAndScores[a1], 3); break;
        case 2:  add_relaxed(algoNamesAndScores[a2], 3); break;
        default: // 0 = tie
            add_relaxed(algoNamesAndScores[a1], 1);
            add_relaxed(algoNamesAndScores[a2], 1);
            break;
    }
}
