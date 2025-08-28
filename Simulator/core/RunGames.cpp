
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


std::string satelliteViewToString(const SatelliteView& sv, size_t W, size_t H) {
    std::string s;
    s.reserve(H*(W+1));
    for (size_t y=0; y<H; ++y) {
        for (size_t x=0; x<W; ++x) {
            char ch = sv.getObjectAt(x,y);
            unsigned char u = static_cast<unsigned char>(ch);
            if (ch == '\0' || (u < 32 && ch != '\n' && ch != '\t' && ch != '\r')) ch='.';
            if (ch == ' ') ch = '.';           
            if (ch == '\r') ch = '\n';         
            s.push_back(ch);
        }
        s.push_back('\n');
    }
    return s;
}





RanGame run_single_game(const GameArgs& g, bool verbose) {
    auto& gmReg = GameManagerRegistrar::getGameManagerRegistrar();
    auto it = gmReg.gameManagers.find(g.GameManagerID);
    if (it == gmReg.gameManagers.end() || !it->second.hasFactory()) {
        throw std::runtime_error("GameManager not found or not loadable: " + g.GameManagerName);
    }
    std::unique_ptr<AbstractGameManager> gm = it->second.create(verbose);
    TankAlgorithmFactory f1 = make_tank_factory(g.playerAndAlgoFactory1ID);
    TankAlgorithmFactory f2 = make_tank_factory(g.playerAndAlgoFactory2ID);

    std::unique_ptr<Player> p1 = make_player(g.playerAndAlgoFactory1ID, /*player_index=*/1, g.map_width, g.map_height, g.max_steps, g.num_shells);
    std::unique_ptr<Player> p2 = make_player(g.playerAndAlgoFactory2ID, /*player_index=*/2, g.map_width, g.map_height, g.max_steps, g.num_shells);
  
    GameResult res = gm->run(
        g.map_width, g.map_height,
        std::move(*g.map),                      
        g.map_name,
        g.max_steps, g.num_shells,
        *p1, g.player1Name, *p2, g.player2Name,
        f1, f2
    );
    std::string gameFinalState = satelliteViewToString(*res.gameState.get() , g.map_width, g.map_height);
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
        RanGame rg = run_single_game(g, verbose);
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
        mode = std::make_unique<ComparativeMode>();
    } else {
        const char* reqs[]={"game_maps_folder","game_manager","algorithms_folder"};
        for (auto* r: reqs) if (!cli.kv.count(r)) { usage(std::string("Missing ") + r); return nullptr; }
        if (!dir_exists(cli.kv["game_maps_folder"])) { usage("game_maps_folder missing/not dir: " + cli.kv["game_maps_folder"]); return nullptr; }
        if (!dir_exists(cli.kv["algorithms_folder"])) { usage("algorithms_folder missing/not dir: " + cli.kv["algorithms_folder"]); return nullptr; }
        maps = list_files(cli.kv["game_maps_folder"]);
        if (maps.empty()) { usage("game_maps_folder has no files."); return nullptr; }
        mode = std::make_unique<CompetitionMode>();
    }
    return mode;
}

void runModeResults(AbstractMode* mode, Cli& cli) {
    if (auto* cm = dynamic_cast<CompetitionMode*>(mode)) 
    cm->writeCompetitionResults(cli.kv["algorithms_folder"], cli.kv["game_maps_folder"], fs::path(cli.kv["game_manager"]).filename().string());

if (auto* cm = dynamic_cast<ComparativeMode*>(mode)) 
    cm->writeComparativeResults(cli.kv["game_managers_folder"], fs::path(cli.kv["game_map"]).filename().string(), fs::path(cli.kv["algorithm1"]).filename().string(), fs::path(cli.kv["algorithm2"]).filename().string());
}

