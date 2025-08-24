
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

    runModeResults(mode.get(), cli);
    
    int close_failures = 0;
    close_failures += closeLoadedLibs(algoLibs);
    close_failures += closeLoadedLibs(gmLibs);
    if (close_failures) {
        std::cerr << "Note: " << close_failures << " plugin(s) failed to close cleanly.\n";
        }
    return 0;
}
