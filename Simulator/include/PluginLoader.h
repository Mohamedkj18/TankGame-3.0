#pragma once

#include <string>
#include <vector>

struct LoadedLib {
    void* handle = nullptr;
    std::string path;
};

#if defined(__APPLE__)
inline constexpr const char* kDynExt = ".dylib";
#elif defined(_WIN32)
inline constexpr const char* kDynExt = ".dll";
#else
inline constexpr const char* kDynExt = ".so";
#endif

bool dlopen_self_register(const std::string& path, LoadedLib& out, std::string& err);
std::vector<std::string> list_shared_objects(const std::string& dir);
std::string stem_key(const std::string& path);
std::string pick_key_from_stem(const std::vector<std::string>& keys, const std::string& stem);