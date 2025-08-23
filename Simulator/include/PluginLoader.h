#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <memory>
#include <iostream>
#include <atomic>

namespace fs = std::filesystem;

struct LoadedLib
{
    void *handle = nullptr;
    std::string path;
};

struct Cli
{
    enum Mode
    {
        Comparative,
        Competition
    } mode;
    bool verbose = false;
    std::unordered_map<std::string, std::string> kv;
};

#if defined(__APPLE__)
inline constexpr const char *kDynExt = ".dylib";
#elif defined(_WIN32)
inline constexpr const char *kDynExt = ".dll";
#else
inline constexpr const char *kDynExt = ".so";
#endif

bool dlopen_self_register(const std::string &path, LoadedLib &out, std::string &err);
std::vector<std::string> list_shared_objects(const std::string &dir);
std::string stem_key(const std::string &path);
std::string pick_key_from_stem(const std::vector<std::string> &keys, const std::string &stem);

std::string trim(const std::string &s);

Cli parse_cli(int argc, char **argv, std::vector<std::string> &bad);

void usage(const std::string &err, const std::vector<std::string> &bad = {});

bool file_exists(const std::string &p);
bool dir_exists(const std::string &p);
std::vector<std::string> list_files(const std::string &dir);