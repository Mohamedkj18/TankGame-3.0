#pragma once

#include <unordered_map>
#include <map>
#include <optional>
#include <unordered_set>

#include <vector>
#include <set>
#include <utility>
#include "DirectionUtils.h"
#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <algorithm>
#include <sstream>
#include <atomic>
#include <chrono>
#include <thread>
#include <filesystem>
#include <cctype>
#include "AbstractGameManager.h"
#include "Tank.h"
#include "Shell.h"
#include "ActionRequest.h"

class Tank;
class Shell;

namespace
{
    std::atomic<uint64_t> g_run_counter{0};

    std::string sanitize(std::string s)
    {
        for (char &c : s)
        {
            if (!(std::isalnum((unsigned char)c) || c == '-' || c == '_' || c == '.'))
                c = '_';
        }
        if (s.size() > 64)
            s.resize(64);
        return s;
    }

    std::string make_unique_base(std::string gm, std::string map,
                                 std::string alg1, std::string alg2)
    {
        using namespace std::chrono;
        const auto ts = duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
        const auto id = g_run_counter.fetch_add(1, std::memory_order_relaxed);
        std::ostringstream os;
        os << sanitize(gm) << "__" << sanitize(map) << "__"
           << sanitize(alg1) << "_vs_" << sanitize(alg2)
           << "__run" << id << "__ts" << ts;
        return os.str();
    }
}

class GameManager : public AbstractGameManager
{
private:
    int width;
    int height;
    int gameStep;
    int totalShellsRemaining;
    int maxSteps;
    int numShellsPerTank;
    int totalTanks;

    std::vector<std::string> movesOfTanks;
    std::unordered_map<int, int> playerTanksCount;
    std::map<int, std::unique_ptr<Tank>> tanks;
    std::unordered_map<int, std::unique_ptr<Shell>> shells;
    std::set<int> mines;
    std::unordered_map<int, Wall> walls;
    std::set<int> wallsToRemove;
    std::set<int> tanksToRemove;
    std::set<int> shellsToRemove;
    std::unordered_map<int, std::unique_ptr<Shell>> secondaryShells;
    std::map<int, std::unique_ptr<Tank>> secondaryTanks;
    std::unordered_set<int> shellsFired;
    bool verbose;
    std::ofstream moves_out;
    std::ofstream viz_out;
    std::string verbose_dir;

public:
    GameManager(bool verbose);

    int getWidth();
    int getHeight();
    int bijection(int x, int y);

    std::unordered_map<int, Wall> &getWalls() { return walls; }
    void removeTank(int tankId);
    void removeShell(int ShellPos);
    void addShell(std::unique_ptr<Shell> shell);

    // int readFile(const std::string &fileName, std::shared_ptr<GameManager> self);
    // void processInputFile(const std::string &inputFilePath);

    // void runGame();

    int readMap(size_t width, size_t height, const SatelliteView &map);

    GameResult run(
        size_t map_width, size_t map_height,
        const SatelliteView &map,
        string map_name,
        size_t max_steps, size_t num_shells,
        Player &player1, string name1, Player &player2, string name2,
        TankAlgorithmFactory player1_tank_algo_factory,
        TankAlgorithmFactory player2_tank_algo_factory) override;

private:
    int getGameStep();

    std::map<int, std::unique_ptr<Tank>> &getTanks() { return tanks; }
    std::unordered_map<int, std::unique_ptr<Shell>> &getShells() { return shells; }
    std::set<int> &getMines() { return mines; }
    void getPlayersInput(std::ofstream &file);
    int getWallHealth(int wallPos);
    void incrementGameStep();
    void addTank(std::unique_ptr<Tank> tank);

    void addMine(int x, int y);
    void addWall(int x, int y);

    void removeMine(int x);
    void removeWall(int x);

    void removeTanks();
    void removeShells();
    void removeWalls();

    void hitWall(int x, int y);
    void hitTank(int tankId);

    void checkForAMine(int x, int y);
    void printBoard();

    void advanceShells();
    void advanceShellsRecentlyFired();
    void executeTanksMoves(bool firstPass);
    void executeBattleInfoRequests(Player &player1, Player &player2);
    void removeObjectsFromTheBoard();
    void reverseHandler(Tank &tank, ActionRequest move);
    void advanceTank(Tank &tank);
    void tankShootingShells(Tank &tank);
    void rotate(Tank &tank);
    void checkForTankCollision(Tank &tank);
    void checkForShellCollision(Shell &shell);
    void tankHitByAShell(int tankPos);
    void shellHitAWall(int shellPos);

    bool checkForAWinner();
    std::optional<int> winnerByTanks() const;

    std::vector<std::string> splitByComma(const std::string &input);
    void sortTanks();
    void outputTankMoves();
    void openVerboseFiles(const std::string &gmName, const std::string &mapName, const std::string &alg1Name, const std::string &alg2Name);

    void clearGameState();
};
