#include <queue>
#include <memory>
#include "MyTankAlgorithm.h"
#include "Roles/ChaserRole.h"
#include "common/TankAlgorithmRegistration.h"

namespace Algorithm_212788293_212497127
{

    REGISTER_TANK_ALGORITHM(TankAlgorithm_212788293_212497127);

    TankAlgorithm_212788293_212497127::TankAlgorithm_212788293_212497127(int player_index, int tank_index)
        : moveIndex(0), tankId(tank_index), playerId(player_index)
    {
        maxMovesPerUpdate = 5;
        range = 20;
        currentDirection = player_index == 1 ? UC::Direction::L : UC::Direction::R;
    }

    void TankAlgorithm_212788293_212497127::updateBattleInfo(BattleInfo &info)
    {
        auto &myInfo = static_cast<MyBattleInfo &>(info);

        if (!myInfo.getShouldKeepRole() || !role)
            setRole(myInfo.extractRole());

        threats = myInfo.getEnemyTanks();
        nearbyFriendlies = myInfo.getFriendlyTanks();
        mines = myInfo.getMines();
        walls = myInfo.getWalls();
        bannedPositionsForTank = myInfo.getPlannedPositions();
        gameWidth = myInfo.getWidth();
        gameHeight = myInfo.getHeight();
        shells = myInfo.getShells();
        currentPos = {myInfo.getMyXPosition(), myInfo.getMyYPosition()};

        bfsPath = role->prepareActions(*this);

        // send data back to player
        myInfo.setPath(bfsPath);
        myInfo.setPlannedActions(plannedMoves);
        moveIndex = 0;
    }

    ActionRequest TankAlgorithm_212788293_212497127::getAction()
    {
        if (!role)
        {
            return ActionRequest::GetBattleInfo;
        }

        return role->getNextAction();
    }

    std::pair<int, int> TankAlgorithm_212788293_212497127::move(std::pair<int, int> pos, UC::Direction dir)
    {
        auto offset = UC::DirectionsUtils::stringToIntDirection[dir];
        return {(pos.first + offset[0] + gameWidth) % gameWidth, (pos.second + offset[1] + gameHeight) % gameHeight};
    }

    bool TankAlgorithm_212788293_212497127::isThreatAhead()
    {
        std::pair<int, int> front = move(currentPos, currentDirection);
        int id = UC::bijection(front.first, front.second);
        return threats.count(id) > 0;
    }

    bool TankAlgorithm_212788293_212497127::isFriendlyTooClose()
    {
        for (UC::Direction d : UC::DirectionsUtils::directions)
        {
            std::pair<int, int> adj = move(currentPos, d);
            int id = UC::bijection(adj.first, adj.second);
            if (nearbyFriendlies.count(id))
                return true;
        }
        return false;
    }

    bool TankAlgorithm_212788293_212497127::shouldShoot(UC::Direction currDir, std::pair<int, int> currPos)
    {
        std::pair<int, int> look = currentPos;
        for (int i = 1; i < range + 1; ++i)
        {
            look = {(currPos.first + UC::DirectionsUtils::stringToIntDirection[currDir][0] * i + gameWidth) % gameWidth,
                    (currPos.second + UC::DirectionsUtils::stringToIntDirection[currDir][1] * i + gameHeight) % gameHeight};
            int id = UC::bijection(look.first, look.second);
            if (nearbyFriendlies.count(id))
                return false; // don't friendly fire
            if (threats.count(id) || walls.count(id))
                return true;
        }
        return false;
    }

    bool TankAlgorithm_212788293_212497127::isSquareValid(int x, int y, std::set<std::pair<int, int>> cellsToAvoid)
    {
        if (x < 0 || y < 0 || x >= static_cast<int>(gameWidth) || y >= static_cast<int>(gameHeight))
            return false;

        int pos = UC::bijection(x, y);
        if (mines.count(pos) > 0 || walls.count(pos) > 0 || cellsToAvoid.count(std::make_pair(x, y)))
            return false;

        return true;
    }

    std::pair<int, int> TankAlgorithm_212788293_212497127::findFirstLegalLocationToFlee(std::pair<int, int> from, std::set<std::pair<int, int>> redZone)
    {
        using Pos = std::pair<int, int>;
        using Entry = std::pair<int, Pos>; // cost, position

        auto isAdjacentToEnemy = [&](const Pos &pos) -> bool
        {
            for (const auto &enemy : threats)
            {
                std::pair<int, int> enemyPos = UC::inverseBijection(enemy);
                if (manhattanDistance(pos.first, pos.second, enemyPos.first, enemyPos.second) <= 1)
                    return true;
            }
            return false;
        };

        auto isSafe = [&](const Pos &pos) -> bool
        {
            return redZone.count(pos) == 0 &&
                   mines.count(UC::bijection(pos.first, pos.second)) == 0 &&
                   walls.count(UC::bijection(pos.first, pos.second)) == 0 &&
                   !isAdjacentToEnemy(pos);
        };

        std::priority_queue<Entry, std::vector<Entry>, std::greater<>> pq;
        std::set<Pos> visited;

        pq.push({0, from});
        visited.insert(from);

        while (!pq.empty())
        {
            auto [cost, pos] = pq.top();
            pq.pop();

            if (pos != from && cost >= 5 && isSafe(pos))
                return pos;

            for (const auto &dir : UC::DirectionsUtils::directions)
            {
                int nx = (pos.first + UC::DirectionsUtils::stringToIntDirection[dir][0] + gameWidth) % gameWidth;
                int ny = (pos.second + UC::DirectionsUtils::stringToIntDirection[dir][1] + gameHeight) % gameHeight;
                Pos next = {nx, ny};

                if (visited.count(next))
                    continue;

                visited.insert(next);
                if (!isSafe(next))
                    continue;
                pq.push({cost + 1, next});
            }
        }

        return std::make_pair(0, 0);
    }

    std::pair<int, int> TankAlgorithm_212788293_212497127::moveTank(std::pair<int, int> pos, UC::Direction dir)
    {
        auto offset = UC::DirectionsUtils::stringToIntDirection[dir];
        return {
            (pos.first + offset[0] + static_cast<int>(gameWidth)) % static_cast<int>(gameWidth),
            (pos.second + offset[1] + static_cast<int>(gameHeight)) % static_cast<int>(gameHeight)};
    }

    std::pair<int, int> TankAlgorithm_212788293_212497127::getTargetForTank()
    {
        // In case of equal distance, prefer the one with lower rotation cost
        std::pair<int, int> myPos = currentPos;
        std::pair<int, int> bestTarget = {-1, -1};
        int minDist = INT_MAX;

        for (int y = 0; y < static_cast<int>(gameHeight); ++y)
        {
            for (int x = 0; x < static_cast<int>(gameWidth); ++x)
            {
                int pos = UC::bijection(x, y);
                if (threats.count(pos) > 0)
                {
                    int dist = manhattanDistance(myPos.first, myPos.second, x, y);

                    if (dist < minDist)
                    {
                        minDist = dist;
                        bestTarget = {x, y};
                    }
                }
            }
        }
        return (bestTarget.first == -1) ? std::pair<int, int>{0, 0} : bestTarget;
    }

    std::vector<std::pair<int, int>> TankAlgorithm_212788293_212497127::getPath(std::pair<int, int> start, std::pair<int, int> target, std::set<std::pair<int, int>> avoidCells)
    {
        std::queue<std::pair<int, int>> queue;
        std::unordered_map<std::pair<int, int>, bool, UC::pair_hash> visited;
        std::unordered_map<std::pair<int, int>, std::pair<int, int>, UC::pair_hash> parent;

        queue.push(start);
        visited[start] = true;

        while (!queue.empty())
        {
            auto current = queue.front();
            queue.pop();

            if (current == target)
            {
                std::vector<std::pair<int, int>> path;
                while (current != start)
                {
                    path.push_back(current);
                    current = parent[current];
                }
                std::reverse(path.begin(), path.end());

                return path;
            }

            for (const auto &dir : UC::DirectionsUtils::directions)
            {
                auto next = moveTank(current, dir);
                if (!visited[next] && isSquareValid(next.first, next.second, avoidCells))
                {
                    visited[next] = true;
                    parent[next] = current;
                    queue.push(next);
                }
            }
        }

        return {};
    }

    int TankAlgorithm_212788293_212497127::manhattanDistance(int x1, int y1, int x2, int y2) const
    {
        return std::abs(x1 - x2) + std::abs(y1 - y2);
    }

    bool TankAlgorithm_212788293_212497127::isInOpen(std::pair<int, int> pos) const
    {
        int wallCount = 0;
        int x = pos.first;
        int y = pos.second;

        for (int dy = -1; dy <= 1; ++dy)
        {
            for (int dx = -1; dx <= 1; ++dx)
            {
                if (dx == 0 && dy == 0)
                    continue;

                int nx = (x + dx + gameWidth) % gameWidth;
                int ny = (y + dy + gameHeight) % gameHeight;

                if (walls.count(UC::bijection(nx, ny)) > 0)
                    wallCount++;
            }
        }

        return wallCount <= 3;
    }

    bool TankAlgorithm_212788293_212497127::isThreatWithinRange(int range) const
    {
        for (int i = currentPos.first - range; i <= currentPos.first + range; ++i)
        {
            for (int j = currentPos.second - range; j <= currentPos.second + range; ++j)
            {
                int x = (i + gameWidth) % gameWidth;
                int y = (j + gameHeight) % gameHeight;
                int pos = UC::bijection(x, y);

                if (threats.count(pos) > 0)
                {
                    return true;
                }
            }
        }

        return false;
    }

    std::pair<int, int> TankAlgorithm_212788293_212497127::findNearestFriendlyTank(std::pair<int, int> from)
    {
        int minPath = INT_MAX;
        std::pair<int, int> candidate = {-1, -1};

        for (int id : nearbyFriendlies)
        {
            if (id == UC::bijection(from.first, from.second))
                continue;
            std::pair<int, int> pos = UC::inverseBijection(id);
            size_t pathLength = getPath(from, pos, bannedPositionsForTank).size();

            if (pathLength < (size_t)minPath)
            {
                minPath = pathLength;
                candidate = pos;
            }
        }

        if (candidate.first == -1)
            return {gameWidth / 2, gameHeight / 2};

        return candidate;
    }

    void TankAlgorithm_212788293_212497127::setRole(std::unique_ptr<Role> newRole)
    {
        role = std::move(newRole);
    }

    std::optional<std::pair<int, int>> TankAlgorithm_212788293_212497127::findEnemyInRange(std::pair<int, int> position, int range)
    {

        for (int i = 0; i < 8; ++i)
        {
            for (int j = 1; j <= range; ++j)
            {

                int x = (position.first + UC::DirectionsUtils::stringToIntDirection[UC::DirectionsUtils::directions[i]][0] * j + gameWidth * j) % gameWidth;
                int y = (position.second + UC::DirectionsUtils::stringToIntDirection[UC::DirectionsUtils::directions[i]][1] * j + gameHeight * j) % gameHeight;
                int pos = UC::bijection(x, y);

                if (threats.count(pos) > 0)
                {
                    return std::make_pair(x, y);
                }
            }
        }
        return std::nullopt;
    }

    std::set<std::pair<int, int>> TankAlgorithm_212788293_212497127::getShells()
    {
        std::set<std::pair<int, int>> shellsXY;
        for (const auto pos : shells)
        {
            shellsXY.insert(UC::inverseBijection(pos));
        }
        return shellsXY;
    }

    std::pair<int, int> TankAlgorithm_212788293_212497127::attackWall(std::pair<int, int> pos)
    {

        std::pair<int, int> closestWall;
        int dist, minDist = INT_MAX;
        for (auto &wall : walls)
        {
            std::pair<int, int> wallPos = UC::inverseBijection(wall);
            dist = manhattanDistance(pos.first, pos.second, wallPos.first, wallPos.second);

            if (dist < minDist)
            {
                minDist = dist;
                closestWall = wallPos;
            }
        }

        return closestWall;
    }
}
