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
    for (size_t i = 0; i < gameManagerRegistrar.getGameManagerCount(); i++) {
        if(gameManagerRegistrar.gameManagers.count(i)){
        auto& gameManagerFactory = gameManagerRegistrar.getGameManagerFactory(i);
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


int ComparativeMode::openSOFiles(Cli cli, std::vector<LoadedLib> algoLibs, std::vector<LoadedLib> gmLibs) {
    std::map<std::string,std::atomic<size_t>> gameManagersNames;
    auto& gmReg = GameManagerRegistrar::getGameManagerRegistrar();
    gmReg.initializeGameManagerCount();
    auto& algoReg = AlgorithmRegistrar::getAlgorithmRegistrar();
    algoReg.initializeAlgoID();
    std::string err;
    LoadedLib lib1 , lib2;
    if (!file_exists(cli.kv["algorithm1"]) || !file_exists(cli.kv["algorithm2"])) {
        usage("One or both algorithms not found: " + cli.kv["algorithm1"] + ", " + cli.kv["algorithm2"]);
        return 1;
    }
    algoReg.createAlgorithmFactoryEntry(fs::path(cli.kv["algorithm1"]).stem().string());
    std::cout << "Registering Algorithm: " << cli.kv["algorithm1"] << "\n";
    
    algoReg.validateLastRegistration();
    algoReg.updateAlgoID();
    std::cout << "Registered Algorithm: " << algoReg.getPlayerAndAlgoFactory(algoReg.getAlgoID() - 1).name() << "\n";
    if (!dlopen_self_register(cli.kv["algorithm1"], lib1, err)) {
        std::cerr << "Failed to load Algorithm shared object: " << cli.kv["algorithm1"] << "\nError: " << err << "\n";
        return 1;
    }
    
    algoReg.createAlgorithmFactoryEntry(fs::path(cli.kv["algorithm2"]).stem().string());
    
    
    algoReg.validateLastRegistration();
    algoReg.updateAlgoID();
    std::cout << "Registered Algorithm: " << algoReg.getPlayerAndAlgoFactory(algoReg.getAlgoID() - 1).name() << "\n";
    if (!dlopen_self_register(cli.kv["algorithm2"], lib2, err)) {
        std::cerr << "Failed to load Algorithm shared object: " << cli.kv["algorithm2"] << "\nError: " << err << "\n";
        return 1;
    }
    if (algoReg.count() < 2) {
        usage("algorithms_folder must contain at least two algorithms.");
        return 1;
    }
    algoLibs.push_back(lib1);
    algoLibs.push_back(lib2);
    for(const auto& gameManagerSO : list_shared_objects(cli.kv["game_managers_folder"])) {
        std::string gameManagerName = fs::path(gameManagerSO).stem().string();
        gmReg.createGameManagerFactoryEntry(gameManagerName);

        std::string err;
        LoadedLib lib;
        if (!dlopen_self_register(gameManagerSO, lib, err)) {
            std::cerr << "Failed to load GameManager shared object: " << gameManagerSO << "\nError: " << err << "\n";
            continue;
        }
        gmReg.validateLastRegistration();
        gmReg.updateGameManagerCount();
        gmLibs.push_back(lib);

        std::cout << "Registered GameManager: " << gmReg.getGameManagerFactory(gmReg.getGameManagerCount() - 1).name() << "\n";
        }
    if (gmReg.getGameManagerCount() == 0) {
        usage("game_managers_folder must contain at least one game manager.");
        return 1;
    }
    return 0;
}


std::string ComparativeMode::satelliteViewToString(const SatelliteView& view, size_t width, size_t height) {
    std::string s;
    s.reserve((width+1)*height);
    for (size_t y=0;y<height;++y) {
        for (size_t x=0;x<width;++x) s.push_back(view.getObjectAt(x,y));
            s.push_back('\n');
    }
    return s;
}


void ComparativeMode::applyCompetitionScore(const GameArgs& g, GameResult res) {
    // Build the key OUTSIDE the lock (cheap later, shorter critical section)
    std::string board = satelliteViewToString(*res.gameState, g.map_width, g.map_height);
    ComparativeKey key{res.winner, res.reason, res.rounds, std::move(board)};

    std::lock_guard<std::mutex> lk(clusters_mtx);

    // Append GM name to the vector for this outcome (creates vector if missing)
    gmNamesAndResults[key].push_back(g.GameManagerName);

    // Insert a representative GameResult for this outcome if not present yet
    // (try_emplace constructs in-place and won't move if the key already exists)
    gamesResults.try_emplace(key, std::move(res));
}
