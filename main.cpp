
#include <filesystem>
#include <dlfcn.h>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <optional>
#include <algorithm>
#include <chrono>

#include "GameManagerRegistrar.h"   // factories list + validate/removeLast  :contentReference[oaicite:5]{index=5}
#include "AlgorithmRegistrar.h"     // player+algo factories + validate      :contentReference[oaicite:6]{index=6}
#include "common/AbstractGameManager.h"                                     // run(...) signature   :contentReference[oaicite:7]{index=7}
#include "common/GameResult.h"                                              // winner/reason        :contentReference[oaicite:8]{index=8}
#include "common/SatelliteView.h"

enum class Mode { Comparative, Competition };

struct Args {
    Mode mode;
    bool verbose = false;
    size_t num_threads = 1; // spec: if missing or 1 → single thread. We ignore >1 for now. :contentReference[oaicite:9]{index=9}
    // comparative
    std::string game_map, game_managers_folder, algorithm1_so, algorithm2_so;
    // competition
    std::string game_maps_folder, game_manager_so, algorithms_folder;
};

[[noreturn]] static void usageAndExit(const std::string& msg) {
    std::cerr << "Error: " << msg << "\n\nUsage:\n"
              << "  -comparative game_map=<file> game_managers_folder=<folder> "
                 "algorithm1=<file.so> algorithm2=<file.so> [num_threads=<n>] [-verbose]\n"
              << "  -competition game_maps_folder=<folder> game_manager=<file.so> "
                 "algorithms_folder=<folder> [num_threads=<n>] [-verbose]\n";
    std::exit(1);
}

static Args parseArgs(int argc, char** argv) {
    if (argc < 2) usageAndExit("Missing mode flag.");
    Args a;

    // mode
    std::string mflag = argv[1];
    if (mflag == "-comparative") a.mode = Mode::Comparative;
    else if (mflag == "-competition") a.mode = Mode::Competition;
    else usageAndExit("First argument must be -comparative or -competition.");

    // parse k=v (order can be arbitrary; '=' may have spaces around; unsupported must error)  :contentReference[oaicite:10]{index=10}
    std::unordered_map<std::string,std::string> kv;
    auto trim = [](std::string& t){
        while(!t.empty() && isspace(t.front())) t.erase(t.begin());
        while(!t.empty() && isspace(t.back()))  t.pop_back();
    };
    for (int i=2;i<argc;++i) {
        std::string s = argv[i];
        if (s == "-verbose") { a.verbose = true; continue; }
        auto pos = s.find('=');
        if (pos == std::string::npos) usageAndExit("Unsupported argument: " + s);
        std::string k = s.substr(0,pos), v = s.substr(pos+1);
        trim(k); trim(v);
        kv[k]=v;
    }

    if (kv.count("num_threads")) {
        try { a.num_threads = std::stoul(kv["num_threads"]); } catch(...) { usageAndExit("num_threads must be integer"); }
    }

    if (a.mode == Mode::Comparative) {
        if (!kv.count("game_map") || !kv.count("game_managers_folder") || !kv.count("algorithm1") || !kv.count("algorithm2"))
            usageAndExit("Missing required comparative arguments.");
        a.game_map = kv["game_map"];
        a.game_managers_folder = kv["game_managers_folder"];
        a.algorithm1_so = kv["algorithm1"];
        a.algorithm2_so = kv["algorithm2"];
    } else {
        if (!kv.count("game_maps_folder") || !kv.count("game_manager") || !kv.count("algorithms_folder"))
            usageAndExit("Missing required competition arguments.");
        a.game_maps_folder = kv["game_maps_folder"];
        a.game_manager_so = kv["game_manager"];
        a.algorithms_folder = kv["algorithms_folder"];
    }
    return a;
}


