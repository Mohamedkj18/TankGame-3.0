#include "ComparativeMode.h"


std::vector<GameArgs> ComparativeMode::getAllGames(std::vector<std::string> game_maps) {
    std::vector<GameArgs> games;

    auto& gameManagerRegistrar = GameManagerRegistrar::getGameManagerRegistrar();
    auto& algorithmRegistrar = AlgorithmRegistrar::getAlgorithmRegistrar();
    size_t factoryAndPlayer1Name = algorithmRegistrar.begin()->first;
    size_t factoryAndPlayer2Name = algorithmRegistrar.rbegin()->first;
    std::string player1Name = algorithmRegistrar.begin()->second.name();
    std::string player2Name = algorithmRegistrar.rbegin()->second.name();
    try{
    struct ParsedMap parsedMap = parseBattlefieldFile(game_maps[0]);
    for (size_t i = 0; i < gameManagerRegistrar.getGameManagerCount(); i++) {
        if(gameManagerRegistrar.gameManagers.count(i)){
        auto& gameManagerFactory = gameManagerRegistrar.getGameManagerFactory(i);
        auto satellite = std::make_unique<InitialSatellite>( parsedMap.player1tanks, parsedMap.player2tanks, parsedMap.walls, parsedMap.mines);
        if (gameManagerFactory.hasFactory()) 
            games.push_back({
                parsedMap.map_width,
                parsedMap.map_height,
                parsedMap.max_steps,
                parsedMap.num_shells,
                std::move(satellite),
                game_maps[0],
                gameManagerFactory.name(), 
                player1Name, 
                player2Name, 
                factoryAndPlayer1Name,
                factoryAndPlayer2Name,
                i 
            });

        }
    }
    }catch(const std::exception& e){
        usage("Error parsing map file: " + std::string(e.what()));
        return {};
        }
    return games;
}


int ComparativeMode::openSOFiles(Cli cli, std::vector<LoadedLib> algoLibs, std::vector<LoadedLib> gmLibs) {    
    if(register2Algorithms(cli, algoLibs))return 1;
    if(registerGameManagers(cli, gmLibs))return 1;
    return 0;
}


int ComparativeMode::register2Algorithms(Cli cli, std::vector<LoadedLib> algoLibs){
    auto& algoReg = AlgorithmRegistrar::getAlgorithmRegistrar();
    std::string err;
    LoadedLib lib1 , lib2;
    if (!file_exists(cli.kv["algorithm1"]) || !file_exists(cli.kv["algorithm2"])) {
        usage("One or both algorithms not found: " + cli.kv["algorithm1"] + ", " + cli.kv["algorithm2"]);
        return 1;
    }
    algoReg.createAlgorithmFactoryEntry(fs::path(cli.kv["algorithm1"]).stem().string());
    if (!dlopen_self_register(cli.kv["algorithm1"], lib1, err)) {
        usage("Failed to load Algorithm shared object: " + cli.kv["algorithm1"] + "\nError: " + err + "\n");
        return 1;
    }


    algoReg.validateLastRegistration();
    algoReg.updateAlgoID();

    algoReg.createAlgorithmFactoryEntry(fs::path(cli.kv["algorithm2"]).stem().string());
     
    if (!dlopen_self_register(cli.kv["algorithm2"], lib2, err)) {
        usage("Failed to load Algorithm shared object: " + cli.kv["algorithm2"] + "\nError: " + err + "\n");
        return 1;
    }
    algoReg.validateLastRegistration();
    algoReg.updateAlgoID();

    if (algoReg.count() < 2) {
        usage("algorithms_folder must contain at least two algorithms.");
        return 1;
    }
    algoLibs.push_back(lib1);
    algoLibs.push_back(lib2);
    return 0;
}


int ComparativeMode::registerGameManagers(Cli cli, std::vector<LoadedLib> gmLibs){
    auto& gmReg = GameManagerRegistrar::getGameManagerRegistrar();
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
        }
    if (gmReg.getGameManagerCount() == 0) {
        usage("game_managers_folder must contain at least one game manager.");
        return 1;
        }
    return 0;
}






void ComparativeMode::applyCompetitionScore(const GameArgs& g, GameResult res, std::string finalGameState) {
    ComparativeKey key{res.winner, res.reason, res.rounds, finalGameState};

    std::lock_guard<std::mutex> lk(clusters_mtx);
              
    if(gmNamesAndResults.count(key) == 0) {
        gmNamesAndResults[key] = std::vector<std::string>();
    }
    gmNamesAndResults[key].push_back(g.GameManagerName);
    gamesResults.try_emplace(key, key);
}




static const char* reasonToStr(GameResult::Reason r) {
    switch (r) {
        case GameResult::ALL_TANKS_DEAD: return "ALL_TANKS_DEAD";
        case GameResult::MAX_STEPS:      return "MAX_STEPS";
        case GameResult::ZERO_SHELLS:    return "ZERO_SHELLS";
    }
    return "UNKNOWN";
}




static inline std::string basename_of(const std::string& path) {
    const auto p = path.find_last_of("/\\");
    return (p == std::string::npos) ? path : path.substr(p + 1);
}


static inline const char* winnerToStr(int w) {
    switch (w) {
        case 0: return "Draw";
        case 1: return "Player 1";
        case 2: return "Player 2";
        default: return "Unknown";
    }
}


static bool printHeader(std::ostream& out,
    const std::string& game_managers_folder,
    const std::string& game_map_filename,
    const std::string& algorithm1_so,
    const std::string& algorithm2_so, int groups_size){
    out << "=== Comparative Mode Results ===\n";
    out << "Map: " << basename_of(game_map_filename) << "\n";
    out << "Algorithm 1: " << basename_of(algorithm1_so) << "\n";
    out << "Algorithm 2: " << basename_of(algorithm2_so) << "\n";
    out << "Game Managers folder: " << game_managers_folder << "\n";
    out << "Total distinct outcome groups: " << groups_size << "\n\n";
    if (groups_size == 0) {
        out << "(No results)\n";
        return false;
        }
    return true;
}


static void printBody(std::ostream& out,
    const ComparativeKey& key,
     std::vector<std::string>& gm_list,
    const ComparativeKey& rep) {
    std::sort(gm_list.begin(), gm_list.end()); 
    out << "---- Group ----\n";
    out << "Winner: " << winnerToStr(key.winner)
        << "  |  Reason: " << reasonToStr(key.reason)
        << "  |  Rounds: " << key.rounds
        << "  |  GameManagers: " << gm_list.size() << "\n";

    // Managers
    out << "Managers: ";
    for (size_t i = 0; i < gm_list.size(); ++i) {
        if (i) out << ", ";
        out << gm_list[i];
    }
    out << "\n";

    out << "Final board:\n";
    if (!rep.finalBoard.empty()) {
        out << rep.finalBoard;
        if (rep.finalBoard.back() != '\n') out << "\n";
    } else {
        out << "(no board captured)\n";
    }

    out << "\n"; 
}

static void sortingHelperFunc(std::vector<std::pair<ComparativeKey, std::vector<std::string>>> & groups) {
    std::sort(groups.begin(), groups.end(), [](const auto& a, const auto& b) {
        const auto& ka = a.first; const auto& kb = b.first;
        if (ka.winner != kb.winner) return ka.winner < kb.winner;
        if (ka.reason != kb.reason) return static_cast<int>(ka.reason) < static_cast<int>(kb.reason);
        if (ka.rounds != kb.rounds) return ka.rounds < kb.rounds;
        if (a.second.size() != b.second.size()) return a.second.size() > b.second.size();
        return ka.finalBoard < kb.finalBoard;
        });
}



void ComparativeMode::writeComparativeResults(const std::string& game_managers_folder, const std::string& game_map_filename, const std::string& algorithm1_so, const std::string& algorithm2_so){
    std::vector<std::pair<ComparativeKey, std::vector<std::string>>> groups;{
    groups.reserve(gmNamesAndResults.size());
    for (const auto& [key, gm_list] : gmNamesAndResults) {
        groups.emplace_back(key, gm_list);
        }
    }
    sortingHelperFunc(groups);
    const std::string out_path = game_managers_folder + "/comparative_results_" + unique_time_str() + ".txt";
    std::ofstream out(out_path);
    if (!out) throw std::runtime_error("Failed to open output file: " + out_path);
    printHeader(out, game_managers_folder, game_map_filename, algorithm1_so, algorithm2_so,  groups.size());
    if (!printHeader(out, game_managers_folder, game_map_filename, algorithm1_so, algorithm2_so, groups.size())) {
        out.close();
        return;
        }
    for (auto& [key, gm_list] : groups) {
    ComparativeKey rep;{
        std::lock_guard<std::mutex> lk(clusters_mtx);
        auto it = gamesResults.find(key);
        rep = (it != gamesResults.end()) ? it->second : key;
    }
    printBody(out, key, gm_list, rep);
    }
    out.close();
}







