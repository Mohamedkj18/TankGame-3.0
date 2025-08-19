#include "PluginLoader.h"

#include <filesystem>
#include <algorithm>
#include <system_error>
#ifndef _WIN32
  #include <dlfcn.h>
#endif

namespace fs = std::filesystem;

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
