#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <memory>

#include "include/Mode.h"
#include "include/ComparativeMode.h"
#include "include/CompetitionMode.h"
#include "common/GameResult.h"
#include "include/GameManagerRegistrar.h"
#include "include/AlgorithmRegistrar.h"

namespace fs = std::filesystem;

// ---------- CLI ----------
struct Cli {
    enum Mode { Comparative, Competition } mode;
    bool verbose = false;
    std::unordered_map<std::string,std::string> kv;
};

static inline std::string trim(const std::string& s) {
    const char* ws = " \t\r\n";
    auto b = s.find_first_not_of(ws);
    auto e = s.find_last_not_of(ws);
    return (b==std::string::npos) ? "" : s.substr(b, e-b+1);
}

static Cli parse_cli(int argc, char** argv, std::vector<std::string>& bad) {
    Cli cli{};
    bool mode_set=false;
    for (int i=1;i<argc;++i) {
        std::string s(argv[i]);
        if (s=="-comparative") { cli.mode=Cli::Comparative; mode_set=true; continue; }
        if (s=="-competition") { cli.mode=Cli::Competition; mode_set=true; continue; }
        if (s=="-verbose")     { cli.verbose=true; continue; }
        auto eq = s.find('=');
        if (eq!=std::string::npos) {
            auto k = trim(s.substr(0,eq));
            auto v = trim(s.substr(eq+1));
            if (!k.empty() && !v.empty()) cli.kv[k]=v; else bad.push_back(s);
        } else bad.push_back(s);
    }
    if (!mode_set) bad.push_back("(missing -comparative/-competition)");
    return cli;
}

static void usage(const std::string& err, const std::vector<std::string>& bad={}) {
    std::cerr << "Error: " << err << "\n";
    if (!bad.empty()) {
        std::cerr << "Unsupported/invalid:"; for (auto& s: bad) std::cerr << " " << s; std::cerr << "\n";
    }
    std::cerr <<
"Comparative:\n"
"  ./sim -comparative game_map=<file> game_managers_folder=<dir> algorithm1=<so> algorithm2=<so> [num_threads=<n>] [-verbose]\n"
"Competition:\n"
"  ./sim -competition game_maps_folder=<dir> game_manager=<so> algorithms_folder=<dir> [num_threads=<n>] [-verbose]\n";
}

static bool file_exists(const std::string& p){ std::error_code ec; return fs::is_regular_file(p,ec); }
static bool dir_exists (const std::string& p){ std::error_code ec; return fs::is_directory(p,ec); }
static std::vector<std::string> list_files(const std::string& dir){
    std::vector<std::string> v; for (auto& e: fs::directory_iterator(dir)) if (e.is_regular_file()) v.push_back(e.path().string());
    std::sort(v.begin(), v.end()); return v;
}

// ------ runner hooks from section (1) and (2) above ------
static TankAlgorithmFactory make_tank_factory(size_t algo_id);
static std::unique_ptr<Player> make_player(size_t algo_id,int player_index,size_t x,size_t y,size_t max_steps,size_t num_shells);
struct RanGame{ std::string gm_name, map_name; size_t algo1_id, algo2_id; GameResult result; };
static RanGame run_single_game(const GameArgs& g, bool verbose);

// ---------------------------------------------------------
int main(int argc, char** argv) {
    std::vector<std::string> bad;
    Cli cli = parse_cli(argc, argv, bad);
    if (!bad.empty()) { usage("Unsupported or missing arguments.", bad); return 1; }

    int num_threads = 1;
    if (cli.kv.count("num_threads")) {
        try { num_threads = std::max(1, std::stoi(cli.kv["num_threads"])); }
        catch (...) { usage("num_threads must be an integer."); return 1; }
    }

    std::unique_ptr<Mode> mode;
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

    // Build jobs via your Mode
    std::vector<GameArgs> jobs = mode->getAllGames(maps);
    if (jobs.empty()) { std::cerr << "No games to run.\n"; return 0; }

    // Run sequentially for now (add worker threads later)
    std::vector<RanGame> results; results.reserve(jobs.size());
    for (auto& g : jobs) {
        results.push_back(run_single_game(g, cli.verbose));
    }

    // TODO: write outputs per mode (comparative_results_*.txt or competition_*.txt)
    std::cout << "Ran " << results.size() << " games.\n";
    return 0;
}
