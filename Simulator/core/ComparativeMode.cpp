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
    
    if (!dlopen_self_register(cli.kv["algorithm1"], lib1, err)) {
        std::cerr << "Failed to load Algorithm shared object: " << cli.kv["algorithm1"] << "\nError: " << err << "\n";
        return 1;
    }
    algoReg.validateLastRegistration();
    algoReg.updateAlgoID();
    std::cout << "Registered Algorithm: " << algoReg.getPlayerAndAlgoFactory(algoReg.getAlgoID() - 1).name() << "\n";

    
    algoReg.createAlgorithmFactoryEntry(fs::path(cli.kv["algorithm2"]).stem().string());
    
    
    
    std::cout << "Registered Algorithm: " << algoReg.getPlayerAndAlgoFactory(algoReg.getAlgoID() - 1).name() << "\n";
    if (!dlopen_self_register(cli.kv["algorithm2"], lib2, err)) {
        std::cerr << "Failed to load Algorithm shared object: " << cli.kv["algorithm2"] << "\nError: " << err << "\n";
        return 1;
    }
    algoReg.validateLastRegistration();
    algoReg.updateAlgoID();
    std::cout << "Registered Algorithm: " << algoReg.getPlayerAndAlgoFactory(algoReg.getAlgoID() - 1).name() << "\n";


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





void ComparativeMode::applyCompetitionScore(const GameArgs& g, GameResult res, std::string finalGameState) {
    // Build the key OUTSIDE the lock (cheap later, shorter critical section)
    ComparativeKey key{res.winner, res.reason, res.rounds, finalGameState};

    std::lock_guard<std::mutex> lk(clusters_mtx);
    std::cout << "[DEBUG] Applying competition score for GameArgs" << std::endl;
              
    if(gmNamesAndResults.count(key) == 0) {
        // If this key is new, initialize the vector for GM names
        gmNamesAndResults[key] = std::vector<std::string>();
    }
    // Append GM name to the vector for this outcome (creates vector if missing)
    gmNamesAndResults[key].push_back(g.GameManagerName);

    // Insert a representative GameResult for this outcome if not present yet
    // (try_emplace constructs in-place and won't move if the key already exists)
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
    // adjust to your convention: 0=Draw, 1=P1, 2=P2
    switch (w) {
        case 0: return "Draw";
        case 1: return "Player 1";
        case 2: return "Player 2";
        default: return "Unknown";
    }
}





void ComparativeMode::writeComparativeResults(const std::string& game_managers_folder,
    const std::string& game_map_filename,
    const std::string& algorithm1_so,
    const std::string& algorithm2_so)
{
// Snapshot maps under lock (in case worker threads still exist upstream).
std::vector<std::pair<ComparativeKey, std::vector<std::string>>> groups;
{
std::lock_guard<std::mutex> lk(clusters_mtx);
groups.reserve(gmNamesAndResults.size());
for (const auto& [key, gm_list] : gmNamesAndResults) {
groups.emplace_back(key, gm_list);
}
}

// Sort groups: by winner, reason, rounds, then descending #GMs (stable view)
std::sort(groups.begin(), groups.end(),
[](const auto& a, const auto& b) {
const auto& ka = a.first; const auto& kb = b.first;
if (ka.winner != kb.winner) return ka.winner < kb.winner;
if (ka.reason != kb.reason) return static_cast<int>(ka.reason) < static_cast<int>(kb.reason);
if (ka.rounds != kb.rounds) return ka.rounds < kb.rounds;
if (a.second.size() != b.second.size()) return a.second.size() > b.second.size();
return ka.finalBoard < kb.finalBoard;
});

// Output file
const std::string out_path =
game_managers_folder + "/comparative_results_" + unique_time_str() + ".txt";
std::ofstream out(out_path);
if (!out) {
throw std::runtime_error("Failed to open output file: " + out_path);
}

// Header
out << "=== Comparative Mode Results ===\n";
out << "Map: " << basename_of(game_map_filename) << "\n";
out << "Algorithm 1: " << basename_of(algorithm1_so) << "\n";
out << "Algorithm 2: " << basename_of(algorithm2_so) << "\n";
out << "Game Managers folder: " << game_managers_folder << "\n";
out << "Total distinct outcome groups: " << groups.size() << "\n\n";

if (groups.empty()) {
out << "(No results)\n";
out.close();
std::cout << "Comparative results written to " << out_path << "\n";
return;
}

// Body
size_t idx = 0;
for (auto& [key, gm_list] : groups) {
++idx;

// Representative (from gamesResults if present; fallback to key)
ComparativeKey rep;
{
std::lock_guard<std::mutex> lk(clusters_mtx);
auto it = gamesResults.find(key);
rep = (it != gamesResults.end()) ? it->second : key;
}

std::sort(gm_list.begin(), gm_list.end()); // stable, readable order

out << "---- Group " << idx << " ----\n";
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

// Final board (representative)
out << "Final board:\n";
if (!rep.finalBoard.empty()) {
out << rep.finalBoard;
if (rep.finalBoard.back() != '\n') out << "\n";
} else {
out << "(no board captured)\n";
}

out << "\n"; // blank line between groups
}

// Footer (optional legend)
out << "Legend: '1' - P1 tank, '2' - P2 tank, '#' - wall, '@' - mine, ' ' - empty\n";
out.close();
std::cout << "Comparative results written to " << out_path << "\n";
}