#pragma once

#include "common/Player.h"
#include "MyBattleInfo.h"
#include "MySatelliteView.h"
// #include "Algorithms/Roles/Role.h"
// #include "Algorithms/Roles/ChaserRole.h"
// #include "Algorithms/Roles/EvasiorRole.h"
// #include "Algorithms/Roles/SniperRole.h"
// #include "Algorithms/Roles/DefenderRole.h"
// #include "Algorithms/Roles/DecoyRole.h"

#include <unordered_map>
#include <vector>
#include <string>
#include <utility>
#include <set>
#include <queue>
#include <stack>

class Role;
struct EnemyScanResult
{
    int closestDistance = INT_MAX;
    bool hasLineOfSight = false;
    bool isInOpen = false;
    bool ShouldKeepRole = false;
};
class MyPlayer : public Player
{
protected:
    int player_index;
    size_t playerGameWidth, playerGameHeight;
    size_t max_steps, num_shells;
    int lastGameStep;

    std::unordered_map<int, std::vector<std::pair<int, int>>> tanksPlannedPaths;
    std::unordered_map<int, std::vector<ActionRequest>> tanksPlannedActions;
    std::unordered_map<int, std::pair<int, int>> tankPositions;
    std::unordered_map<int, int> tanksRemainingShells;
    std::unordered_map<int, Direction> tanksDirection;
    std::unordered_map<int, std::string> tankRoles;
    std::vector<std::vector<char>> lastSatellite;

public:
    MyPlayer(int player_index, size_t x, size_t y, size_t max_steps, size_t num_shells);
    void updateTankWithBattleInfo(TankAlgorithm &tank, SatelliteView &satellite_view) override;
    std::string getRoleName(int tankId) const
    {
        auto it = tankRoles.find(tankId);
        return (it != tankRoles.end()) ? it->second : "Unknown";
    }

protected:
    EnemyScanResult assignRole(int tankId, std::pair<int, int> pos, std::set<int> shells, std::set<int> enemyTanks, int numFriendlyTanks);
    virtual std::unique_ptr<Role> createRole(int tankId, std::pair<int, int> pos, EnemyScanResult scan, std::set<int> shells, std::set<int> enemyTanks, int numOfFriendlyTanks) = 0;

    virtual bool shouldKeepRole(int tankId, const std::pair<int, int> &pos, const std::string &role, EnemyScanResult scan, std::set<int> shells, std::set<int> enemyTanks, int numFriendlyTanks) = 0;
    EnemyScanResult scanVisibleEnemies(int x0, int y0) const;
    int manhattanDistance(int x1, int y1, int x2, int y2) const;
    bool isClearLine(int x0, int y0, int x1, int y1) const;
    bool isInOpen(int x, int y) const;
    std::set<std::pair<int, int>> getCalculatedPathsSet();
    bool isInRedZone(int x, int y, std::set<int> shellsPositions, std::set<int> enemies) const;
    void updateTanksStatus();
    void deleteTankData(int tankId);
    int getTankId(std::pair<int, int> pos);
    void initializeTanksData();

private:
    bool gotBattleInfo = false;
    Direction updateTankDirection(int tankId);
    std::pair<int, int> prepareInfoForBattleInfo(std::set<int> &mines, std::set<int> &walls, std::set<int> &shells, std::set<int> &friendlyTanks, std::set<int> &enemyTanks, SatelliteView &satellite_view);
};

// ------------------------ Player 1 ------------------------
class Player1 : public MyPlayer
{
public:
    using MyPlayer::MyPlayer;
    virtual ~Player1();

    std::unique_ptr<Role> createRole(int tankId, std::pair<int, int> pos, EnemyScanResult scan, std::set<int> shells, std::set<int> enemyTanks, int numOfFriendlyTanks) override;
    bool shouldKeepRole(int tankId, const std::pair<int, int> &pos, const std::string &role, EnemyScanResult scan, std::set<int> shells, std::set<int> enemyTanks, int numFriendlyTanks) override;
};

// ------------------------ Player 2 ------------------------
class Player2 : public MyPlayer
{
public:
    using MyPlayer::MyPlayer;
    virtual ~Player2();

    std::unique_ptr<Role> createRole(int tankId, std::pair<int, int> pos, EnemyScanResult scan, std::set<int> shells, std::set<int> enemyTanks, int numOfFriendlyTanks) override;
    bool shouldKeepRole(int tankId, const std::pair<int, int> &pos, const std::string &role, EnemyScanResult scan, std::set<int> shells, std::set<int> enemyTanks, int) override;

    // void assignRole(int tankId) override;
};
