#pragma once

#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <fstream>
#include <atomic>
#include "common/AbstractGameManager.h"

class MySatelliteView;

namespace GameManager_212788293_212497127
{
    // Walls live in doubled coordinates like other objects (x,y are even)
    struct Wall
    {
        int x;
        int y;
        short health;
    };

    class Tank;
    class Shell;

    class GameManager : public AbstractGameManager
    {
    private:
        int width{};
        int height{};
        int gameStep{};
        int totalShellsRemaining{};
        int maxSteps{};
        int numShellsPerTank{};
        int totalTanks{};

        std::vector<std::string> movesOfTanks;
        std::unordered_map<int, int> playerTanksCount;

        // keyed by bijection(x,y) on doubled grid
        std::map<int, std::unique_ptr<Tank>> tanks;
        std::unordered_map<int, std::unique_ptr<Shell>> shells;
        std::set<int> mines;
        std::unordered_map<int, Wall> walls;

        std::set<int> wallsToRemove;
        std::set<int> tanksToRemove;
        std::set<int> shellsToRemove;

        // staging maps for movement steps
        std::unordered_map<int, std::unique_ptr<Shell>> secondaryShells;
        std::map<int, std::unique_ptr<Tank>> secondaryTanks;
        std::unordered_set<int> shellsFired;

        bool verbose{false};
        std::ofstream moves_out;
        std::ofstream viz_out;
        std::string verbose_dir{"verbose"};

    public:
        explicit GameManager(bool verbose);
        ~GameManager() noexcept override = default;

        int getWidth() { return width; }
        int getHeight() { return height; }
        int bijection(int x, int y);

        std::unordered_map<int, Wall> &getWalls() { return walls; }

        void removeTank(int tankPos);
        void removeShell(int shellPos);
        void addShell(std::unique_ptr<Shell> shell);

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
        int getGameStep() { return gameStep; }
        int getWallHealth(int wallPos);

        std::map<int, std::unique_ptr<Tank>> &getTanks() { return tanks; }
        std::unordered_map<int, std::unique_ptr<Shell>> &getShells() { return shells; }
        std::set<int> &getMines() { return mines; }

        void getPlayersInput(std::ofstream &file);
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

        // allow GameObject helpers to query dimensions
        friend class GameObject;
    };
}
