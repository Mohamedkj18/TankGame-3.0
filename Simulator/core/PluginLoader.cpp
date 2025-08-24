#include "PluginLoader.h"

#include <system_error>
#ifndef _WIN32
  #include <dlfcn.h>
#endif



std::string stem_key(const std::string& path) {
    return fs::path(path).stem().string();
}

std::vector<std::string> list_shared_objects(const std::string& dir) {
    std::vector<std::string> v;
    std::error_code ec;
    fs::directory_options opts = fs::directory_options::skip_permission_denied;
    for (fs::directory_iterator it(dir, opts, ec), end; it != end; it.increment(ec)) {
        if (ec) continue;
        if (!it->is_regular_file(ec)) continue;
        if (it->path().extension() == kDynExt) v.push_back(it->path().string());
    }
    std::sort(v.begin(), v.end());
    return v;
}

std::string pick_key_from_stem(const std::vector<std::string>& keys, const std::string& stem) {
    auto tolow = [](std::string s){ std::transform(s.begin(), s.end(), s.begin(), ::tolower); return s; };
    // exact
    for (auto& k: keys) if (k == stem) return k;
    // case-insensitive
    auto s = tolow(stem);
    for (auto& k: keys) if (tolow(k) == s) return k;
    // substring heuristic
    for (auto& k: keys) if (tolow(k).find(s) != std::string::npos) return k;
    return {};
}

bool dlopen_self_register(const std::string& path, LoadedLib& out, std::string& err) {
#ifdef _WIN32
    err = "dlopen_self_register not implemented for Windows";
    return false;
#else
    err.clear();
    void* h = dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
    if (!h) { err = dlerror(); return false; }
    out.handle = h;
    out.path = path;
    return true;
#endif
}



std::string trim(const std::string& s) {
    const char* ws = " \t\r\n";
    auto b = s.find_first_not_of(ws);
    auto e = s.find_last_not_of(ws);
    return (b==std::string::npos) ? "" : s.substr(b, e-b+1);
}

Cli parse_cli(int argc, char** argv, std::vector<std::string>& bad) {
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

void usage(const std::string& err, const std::vector<std::string>& bad) {
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

bool file_exists(const std::string& p){ std::error_code ec; return fs::is_regular_file(p,ec); }
bool dir_exists (const std::string& p){ std::error_code ec; return fs::is_directory(p,ec); }
std::vector<std::string> list_files(const std::string& dir){
    std::vector<std::string> v; for (auto& e: fs::directory_iterator(dir)) if (e.is_regular_file()) v.push_back(e.path().string());
    std::sort(v.begin(), v.end()); return v;
}


int closeLoadedLibs(std::vector<LoadedLib>& libs) {
    int failures = 0;
    for (auto it = libs.rbegin(); it != libs.rend(); ++it) {
        if (!it->handle) continue;
        if (dlclose(it->handle) != 0) {
            const char* err = dlerror();
            std::cerr << "Warning: dlclose failed for " << it->path
                      << ": " << (err ? err : "unknown error") << "\n";
            ++failures;
        }
        it->handle = nullptr;
    }
    libs.clear();
    return failures;
}



