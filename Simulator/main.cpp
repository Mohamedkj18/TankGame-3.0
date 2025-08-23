
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

    std::vector<std::string> maps;
    std::unique_ptr<AbstractMode> mode = createMode(cli, maps);
    if(!mode) return 1;
 
    // Load game managers
    std::vector<LoadedLib> gmLibs;
    std::vector<LoadedLib> algoLibs;
    
    if(mode->openSOFiles(cli, algoLibs, gmLibs) != 0) {
        std::cerr << "Failed to open shared object files.\n";
        return 1;
    }

    std::vector<GameArgs> jobs = mode->getAllGames(maps);
    if (jobs.empty()) { std::cerr << "No games to run.\n"; return 0; }
    const size_t n = jobs.size();
    num_threads = std::min<size_t>(num_threads, n);
    
    if(num_threads > 1)runThreads(mode, std::move(jobs), num_threads, cli.verbose);
    else runAllGames(mode, std::move(jobs), cli.verbose); 

    if (auto* cm = dynamic_cast<CompetitionMode*>(mode.get())) 
        cm->writeCompetitionResults(cli.kv["algorithms_folder"], cli.kv["game_maps_folder"], fs::path(cli.kv["game_manager"]).filename().string());
    
    if (auto* cm = dynamic_cast<ComparativeMode*>(mode.get())) 
        cm->writeComparativeResults(cli.kv["game_managers_folder"], fs::path(cli.kv["game_map"]).filename().string(), fs::path(cli.kv["algorithm1"]).filename().string(), fs::path(cli.kv["algorithm2"]).filename().string());
    
    return 0;
}
