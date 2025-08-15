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