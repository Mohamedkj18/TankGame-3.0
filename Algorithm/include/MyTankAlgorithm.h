#pragma once

#include "MyBattleInfo.h"
#include "common/TankAlgorithm.h"
#include "common/BattleInfo.h"
#include "MyPlayer.h"
#include "Roles/Role.h"
#include <vector>
#include <string>
#include <algorithm>
#include <optional>
#include <memory>
#include <set>
#include <unordered_map>

namespace Algorithm_212788293_212497127
{
    class Role;
    class TankAlgorithm_212788293_212497127 : public TankAlgorithm
    {
    private:
        int moveIndex;
        int tankId;
        int playerId;
        std::vector<ActionRequest> plannedMoves;
        std::vector<std::pair<int, int>> bfsPath;

        // Metadata from BattleInfo
        std::unordered_map<int, std::vector<std::pair<int, int>>> tanksPlannedPaths;
        std::unique_ptr<Role> role;
        std::set<int> nearbyFriendlies;
        std::set<int> threats;
        std::set<int> mines;
        std::set<int> walls;
        std::set<int> shells;
        std::vector<std::vector<char>> lastSatellite;
        std::set<std::pair<int, int>> bannedPositionsForTank;

        bool movePending;
        int gameWidth;
        int gameHeight;
        UC::Direction currentDirection;
        std::pair<int, int> currentPos;

    public:
        TankAlgorithm_212788293_212497127(int player_index, int tank_index);

        ActionRequest getAction() override;
        void updateBattleInfo(BattleInfo &info) override;

        // Execution logic
        std::pair<int, int> move(std::pair<int, int> current, UC::Direction direction);
        std::string getDirectionFromPosition(std::pair<int, int> current, std::pair<int, int> target);
        int rotateTowards(std::string desiredDir, int step);
        double getAngleFromDirections(const std::string &directionStr, const std::string &desiredDir);

        // BFS pathfinding
        std::vector<std::pair<int, int>> getPath(std::pair<int, int> start, std::pair<int, int> target, std::set<std::pair<int, int>> avoidCells);
        bool isSquareValid(int x, int y, std::set<std::pair<int, int>> cellsToAvoid);
        std::pair<int, int> findFirstLegalLocationToFlee(std::pair<int, int> from, std::set<std::pair<int, int>> redZone);
        std::pair<int, int> getTargetForTank();
        std::pair<int, int> moveTank(std::pair<int, int> pos, UC::Direction dir);
        std::set<std::pair<int, int>> getBannedPositionsForTank() { return bannedPositionsForTank; };
        std::set<std::pair<int, int>> getShells();
        bool isThreatAhead();
        bool isFriendlyTooClose();
        bool shouldShoot(UC::Direction currDir, std::pair<int, int> currPos);
        void setTankId(int id) { tankId = id; };
        int getTankId() const { return tankId; };
        void setPlayerId(int id) { playerId = id; };
        int getPlayerId() const { return playerId; };
        std::set<int> getEnemyTanks() { return threats; };
        UC::Direction getCurrentDirection() const { return currentDirection; }
        void setCurrentDirection(UC::Direction dir) { currentDirection = dir; }

        std::pair<int, int> getCurrentPosition() const { return currentPos; }
        void setCurrentPosition(std::pair<int, int> pos) { currentPos = pos; }

        int getMaxMovesPerUpdate() const { return maxMovesPerUpdate; }
        void setMaxMovesPerUpdate(int moves) { maxMovesPerUpdate = moves; }

        void setNextMoves(const std::vector<ActionRequest> &moves) { plannedMoves = moves; }
        const std::vector<ActionRequest> &getNextMoves() const { return plannedMoves; }

        const std::vector<std::pair<int, int>> &getBFSPath() const { return bfsPath; }
        void setBFSPath(const std::vector<std::pair<int, int>> &path) { bfsPath = path; }

        int manhattanDistance(int x1, int y1, int x2, int y2) const;
        int getGameWidth() const { return gameWidth; }
        int getGameHeight() const { return gameHeight; }
        bool isInOpen(std::pair<int, int> pos) const;
        std::pair<int, int> findNearestFriendlyTank(std::pair<int, int> myPos);
        bool isThreatWithinRange(int range) const;

        void setRole(std::unique_ptr<Role> newRole);
        std::optional<std::pair<int, int>> findEnemyInRange(std::pair<int, int> position, int range);
        std::pair<int, int> attackWall(std::pair<int, int> pos);

        // std::set<std::pair<int, int>> isShellsInRange(std::pair<int, int> pos, int range);

    private:
        int range;
        int maxMovesPerUpdate;
    };
}