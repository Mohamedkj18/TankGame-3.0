
#include "RunGames.h"



TankAlgorithmFactory make_tank_factory(size_t algo_id) {
    return [algo_id](int player_index, int tank_index) {
        auto& reg = AlgorithmRegistrar::getAlgorithmRegistrar();
        auto& entry = reg.getPlayerAndAlgoFactory(algo_id);
        return entry.createTankAlgorithm(player_index, tank_index); // unique_ptr<TankAlgorithm>
    };
}


std::unique_ptr<Player> make_player(size_t algo_id,
    int player_index,
    size_t x, size_t y,
    size_t max_steps, size_t num_shells)
{
auto& reg = AlgorithmRegistrar::getAlgorithmRegistrar();
auto& entry = reg.getPlayerAndAlgoFactory(algo_id);
return entry.createPlayer(player_index, x, y, max_steps, num_shells); // unique_ptr<Player>
}


std::string satelliteViewToString(const SatelliteView& view, size_t width, size_t height) {
    std::string s;
    s.reserve((width+1)*height);
    for (size_t y=0;y<height;y++) {
        for (size_t x=0;x<width;x++) s.push_back(view.getObjectAt(x,y));
            s.push_back('\n');
    }
    std::cout << "Satellite View String: " << s << "\n";
    return s;
}




RanGame run_single_game(const GameArgs& g, bool verbose) {
    // 1) GameManager by ID
    auto& gmReg = GameManagerRegistrar::getGameManagerRegistrar();
    auto it = gmReg.gameManagers.find(g.GameManagerID);
    if (it == gmReg.gameManagers.end() || !it->second.hasFactory()) {
        throw std::runtime_error("GameManager not found or not loadable: " + g.GameManagerName);
    }
    std::unique_ptr<AbstractGameManager> gm = it->second.create(verbose);  // factory(verbose) under the hood 

    // 2) Factories for the two algorithms
    TankAlgorithmFactory f1 = make_tank_factory(g.playerAndAlgoFactory1ID);
    TankAlgorithmFactory f2 = make_tank_factory(g.playerAndAlgoFactory2ID);

    // 3) Players for each side (initial x,y are placeholders; GM/Player usually handle placement using the SatelliteView)
    std::unique_ptr<Player> p1 = make_player(g.playerAndAlgoFactory1ID, /*player_index=*/1, g.map_width, g.map_height, g.max_steps, g.num_shells);
    std::unique_ptr<Player> p2 = make_player(g.playerAndAlgoFactory2ID, /*player_index=*/2, g.map_width, g.map_height, g.max_steps, g.num_shells);
  
    // 4) Run the game
    GameResult res = gm->run(
        g.map_width, g.map_height,
        std::move(*g.map),                      
        g.map_name,
        g.max_steps, g.num_shells,
        *p1, g.player1Name, *p2, g.player2Name,
        f1, f2
    );
    std::string gameFinalState = satelliteViewToString(*res.gameState , g.map_width, g.map_height);
    return RanGame{ g.GameManagerName, g.map_name, g.playerAndAlgoFactory1ID, g.playerAndAlgoFactory2ID, std::move(res), gameFinalState };
}


void runThreads(std::unique_ptr<AbstractMode>& mode, std::vector<GameArgs> jobs, int num_threads, bool verbose) {
    std::atomic<size_t> next{0};
    const size_t n = jobs.size();    
    auto worker = [&] {
        while (true) {
            size_t i = next.fetch_add(1, std::memory_order_relaxed);
            if (i >= n) break;
            RanGame result = run_single_game(jobs[i], verbose); 
            mode->applyCompetitionScore(jobs[i], std::move(result.result), result.gameFinalState);
        }
    };

    std::vector<std::thread> threads;
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back(worker);
    }
    for (auto& th : threads) {
        th.join();
    }
}


void runAllGames(std::unique_ptr<AbstractMode>& mode, std::vector<GameArgs> jobs, bool verbose) {
    for(const auto& g: jobs) {
        std::cout << "Running game: " << g.GameManagerName << " vs " << g.map_name << "\n";
        RanGame rg = run_single_game(g, verbose);
        std::cout << "Winner: " << rg.result.winner << "\n";
        mode->applyCompetitionScore(g, std::move(rg.result), rg.gameFinalState);
    }
}


std::unique_ptr<AbstractMode> createMode(Cli cli, std::vector<std::string> &maps) {
    std::unique_ptr<AbstractMode> mode;
    if (cli.mode == Cli::Comparative) {
        const char* reqs[]={"game_map","game_managers_folder","algorithm1","algorithm2"};
        for (auto* r: reqs) if (!cli.kv.count(r)) { usage(std::string("Missing ") + r); return nullptr; }
        if (!file_exists(cli.kv["game_map"])) { usage("game_map not found: " + cli.kv["game_map"]); return nullptr; }
        if (!dir_exists(cli.kv["game_managers_folder"])) { usage("game_managers_folder missing/not dir: " + cli.kv["game_managers_folder"]); return nullptr; }
        maps.push_back(cli.kv["game_map"]);
        std::cout << "Game map to be used: " << maps[0] << "\n";
        mode = std::make_unique<ComparativeMode>();
        // (Pass chosen algos to your ComparativeMode as needed.)
    } else {
        const char* reqs[]={"game_maps_folder","game_manager","algorithms_folder"};
        for (auto* r: reqs) if (!cli.kv.count(r)) { usage(std::string("Missing ") + r); return nullptr; }
        if (!dir_exists(cli.kv["game_maps_folder"])) { usage("game_maps_folder missing/not dir: " + cli.kv["game_maps_folder"]); return nullptr; }
        if (!dir_exists(cli.kv["algorithms_folder"])) { usage("algorithms_folder missing/not dir: " + cli.kv["algorithms_folder"]); return nullptr; }
        maps = list_files(cli.kv["game_maps_folder"]);
        if (maps.empty()) { usage("game_maps_folder has no files."); return nullptr; }
        mode = std::make_unique<CompetitionMode>();
        // (Pass chosen GM to your CompetitionMode as needed.)
    }
    return mode;
}

