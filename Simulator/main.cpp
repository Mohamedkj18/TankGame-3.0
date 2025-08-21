
#include "AbstractMode.h"
#include "ComparativeMode.h"
#include "CompetitionMode.h"
#include "common/GameResult.h"
#include "include/GameManagerRegistrar.h"
#include "include/AlgorithmRegistrar.h"
#include "RunGames.h"
#include <thread>











int main(int argc, char** argv) {
    std::vector<std::string> bad;
    Cli cli = parse_cli(argc, argv, bad);
    if (!bad.empty()) { usage("Unsupported or missing arguments.", bad); return 1; }

    int num_threads = 1;
    if (cli.kv.count("num_threads")) {
        try { num_threads = std::max(1, std::stoi(cli.kv["num_threads"])); }
        catch (...) { usage("num_threads must be an integer."); return 1; }
    }

    std::unique_ptr<AbstractMode> mode;
    std::vector<std::string> maps;

    if (cli.mode == Cli::Comparative) {
        const char* reqs[]={"game_map","game_managers_folder","algorithm1","algorithm2"};
        for (auto* r: reqs) if (!cli.kv.count(r)) { usage(std::string("Missing ") + r); return 1; }
        if (!file_exists(cli.kv["game_map"])) { usage("game_map not found: " + cli.kv["game_map"]); return 1; }
        if (!dir_exists(cli.kv["game_managers_folder"])) { usage("game_managers_folder missing/not dir: " + cli.kv["game_managers_folder"]); return 1; }
        maps.push_back(cli.kv["game_map"]);
        mode = std::make_unique<ComparativeMode>();
        // (Pass chosen algos to your ComparativeMode as needed.)
    } else {
        const char* reqs[]={"game_maps_folder","game_manager","algorithms_folder"};
        for (auto* r: reqs) if (!cli.kv.count(r)) { usage(std::string("Missing ") + r); return 1; }
        if (!dir_exists(cli.kv["game_maps_folder"])) { usage("game_maps_folder missing/not dir: " + cli.kv["game_maps_folder"]); return 1; }
        if (!dir_exists(cli.kv["algorithms_folder"])) { usage("algorithms_folder missing/not dir: " + cli.kv["algorithms_folder"]); return 1; }
        maps = list_files(cli.kv["game_maps_folder"]);
        if (maps.empty()) { usage("game_maps_folder has no files."); return 1; }
        mode = std::make_unique<CompetitionMode>();
        // (Pass chosen GM to your CompetitionMode as needed.)
    }
    // Load game managers
    std::vector<LoadedLib> gmLibs;
    std::vector<LoadedLib> algoLibs;
    
    if(mode->openSOFiles(cli, algoLibs, gmLibs) != 0) {
        std::cerr << "Failed to open shared object files.\n";
        return 1;
    }


    // Build jobs via your Mode
    std::vector<GameArgs> jobs = mode->getAllGames(maps);
    std::cout << "initialized jobs with " << jobs.size() << " games.\n";
    if (jobs.empty()) { std::cerr << "No games to run.\n"; return 0; }

    // Run sequentially for now (add worker threads later)
    std::atomic<size_t> next{0};
    const size_t n = jobs.size();
    num_threads = std::min<size_t>(num_threads, n);
    std::vector<RanGame> results(n);


    auto worker = [&] {
        while (true) {
            size_t i = next.fetch_add(1, std::memory_order_relaxed);
            if (i >= n) break;
            results[i] = run_single_game(jobs[i], cli.verbose);
            mode->applyCompetitionScore(jobs[i], std::move(results[i].result));
        }
    };

    std::vector<std::thread> threads;
    for (int t = 0; t < num_threads; ++t)
        threads.emplace_back(worker);

    for (auto& th : threads) th.join();




    std::cout << "results init with " << results.size() << " game results.\n";
    

    // TODO: write outputs per mode (comparative_results_*.txt or competition_*.txt)
    std::cout << "Ran " << results.size() << " games.\n";
    for (const auto& result : results) {
        std::cout << "Game Result: " << result.gm_name << " vs " << result.map_name
                  << " | Winner: " << result.result.winner
                  << " | Reason: " << result.result.reason
                  << " | Remaining Tanks: ";
        for (const auto& tank : result.result.remaining_tanks) {
            std::cout << tank << " ";
        }
        std::cout << "\n";
    }
}
