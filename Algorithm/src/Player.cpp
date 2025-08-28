
#include "MyPlayer.h"
#include "common/PlayerRegistration.h"

namespace Algorithm_212788293_212497127
{
    MyPlayer::MyPlayer(int player_index, size_t x, size_t y, size_t max_steps, size_t num_shells)
        : player_index(player_index), playerGameWidth(x), playerGameHeight(y), max_steps(max_steps), num_shells(num_shells) {}

    void MyPlayer::updateTankWithBattleInfo(TankAlgorithm &tank, SatelliteView &satellite_view)
    {
        ;
        std::set<int> friendlyTanks, enemyTanks, mines, walls, shells;

        int myX = -1, myY = -1;
        std::pair<int, int> tankPos = prepareInfoForBattleInfo(mines, walls, shells, friendlyTanks, enemyTanks, satellite_view);

        if (!gotBattleInfo)
        {
            initializeTanksData();
        }

        myX = tankPos.first;
        myY = tankPos.second;

        MyBattleInfo info(playerGameWidth, playerGameHeight, friendlyTanks, enemyTanks, mines, walls, shells);

        info.setMyXPosition(myX);
        info.setMyYPosition(myY);

        int tankId = getTankId({myX, myY});

        EnemyScanResult scan = assignRole(tankId, {myX, myY}, shells, enemyTanks, friendlyTanks.size());
        if (!scan.ShouldKeepRole)
        {
            info.setRole(createRole(tankId, {myX, myY}, scan, shells, enemyTanks, friendlyTanks.size()));
            info.setShouldKeepRole(false);
        }
        else
        {
            info.setShouldKeepRole(true);
        }

        std::set<std::pair<int, int>> planned = getCalculatedPathsSet();
        info.setPlannedPositions(planned);
        tank.updateBattleInfo(info);
        tanksPlannedActions[tankId] = info.getPlannedActions();
        tanksPlannedPaths[tankId] = info.getPath();

        if (tanksRemainingShells.count(tankId) == 0)
        {
            tanksRemainingShells[tankId] = num_shells;
        }
    }

    void MyPlayer::updateTanksStatus()
    {

        for (auto &[id, path] : tanksPlannedPaths)
        {
            bool tankAlive = false;
            for (auto pos : path)
            {
                if (lastSatellite[pos.second][pos.first] - '0' == id)
                {
                    tankAlive = true;
                    break;
                }
            }
            if (!tankAlive)
            {
                deleteTankData(id);
            }
        }
    }

    void MyPlayer::deleteTankData(int tankId)
    {
        tanksPlannedActions.erase(tankId);
        tanksPlannedPaths.erase(tankId);
        tanksRemainingShells.erase(tankId);
        tankRoles.erase(tankId);
    }

    EnemyScanResult MyPlayer::assignRole(int tankId, std::pair<int, int> pos, std::set<int> shells, std::set<int> enemyTanks, int numFriendlyTanks)
    {

        EnemyScanResult scan = scanVisibleEnemies(pos.first, pos.second);

        bool surrounded = true;
        bool isStuck = true;
        for (int dy = -1; dy <= 1; ++dy)
        {
            for (int dx = -1; dx <= 1; ++dx)
            {
                if (dx == 0 && dy == 0)
                    continue;

                int nx = (pos.first + dx + playerGameWidth) % playerGameWidth;
                int ny = (pos.second + dy + playerGameHeight) % playerGameHeight;

                if (lastSatellite[ny][nx] != '#' && lastSatellite[ny][nx] != '@')
                    surrounded = false;

                if (lastSatellite[ny][nx] != '@')
                    isStuck = false;
            }
        }
        scan.surroundedByWalls = surrounded;
        scan.isStuck = isStuck;

        auto it = tankRoles.find(tankId);
        if (it != tankRoles.end())
        {

            if (shouldKeepRole(tankId, pos, it->second, scan, shells, enemyTanks, numFriendlyTanks))
            {
                scan.ShouldKeepRole = true;
            }
        }

        return scan;
    }

    std::set<std::pair<int, int>> MyPlayer::getCalculatedPathsSet()
    {
        std::set<std::pair<int, int>> bannedPositionsSet;
        for (const auto &entry : tanksPlannedPaths)
        {
            const auto &path = entry.second;
            for (const auto &pos : path)
            {
                bannedPositionsSet.insert(pos);
            }
        }

        return bannedPositionsSet;
    }

    EnemyScanResult MyPlayer::scanVisibleEnemies(int x0, int y0) const
    {

        const char enemyChar = (player_index == 1) ? '2' : '1';
        EnemyScanResult result;
        for (int y = 0; y < (int)lastSatellite.size(); ++y)
        {
            for (int x = 0; x < (int)lastSatellite[0].size(); ++x)
            {
                if (lastSatellite[y][x] == enemyChar)
                {
                    int dist = manhattanDistance(x0, y0, x, y);
                    if (dist < result.closestDistance)
                        result.closestDistance = dist;
                    if ((x == x0 || y == y0) && isClearLine(x0, y0, x, y))
                        result.hasLineOfSight = true;
                }
            }
        }
        return result;
    }

    int MyPlayer::manhattanDistance(int x1, int y1, int x2, int y2) const
    {
        return std::abs(x1 - x2) + std::abs(y1 - y2);
    }

    bool MyPlayer::isClearLine(int x1, int y1, int x2, int y2) const
    {
        int dx = x2 - x1;
        int dy = y2 - y1;

        // Normalize direction to one of 8 (if not aligned, return false)
        if (dx != 0)
            dx /= std::abs(dx);
        if (dy != 0)
            dy /= std::abs(dy);

        if ((x1 + dx + (int)playerGameWidth) % (int)playerGameWidth == x2 &&
            (y1 + dy + (int)playerGameHeight) % (int)playerGameHeight == y2)
        {
            return true; // adjacent cell
        }

        int x = (x1 + dx + (int)playerGameWidth) % (int)playerGameWidth;
        int y = (y1 + dy + (int)playerGameHeight) % (int)playerGameHeight;

        while (x != x2 || y != y2)
        {
            if (lastSatellite[y][x] == '#')
                return false;

            x = (x + dx + playerGameWidth) % playerGameWidth;
            y = (y + dy + playerGameHeight) % playerGameHeight;
        }
        return true;
    }

    bool MyPlayer::isInOpen(int x, int y) const
    {
        int wallCount = 0;
        for (int dy = -1; dy <= 1; ++dy)
        {
            for (int dx = -1; dx <= 1; ++dx)
            {
                if (dx == 0 && dy == 0)
                    continue;

                int nx = (x + dx + playerGameWidth) % playerGameWidth;
                int ny = (y + dy + playerGameHeight) % playerGameHeight;

                if (lastSatellite[ny][nx] == '#')
                    wallCount++;
            }
        }
        return wallCount <= 3;
    }

    std::pair<int, int> MyPlayer::prepareInfoForBattleInfo(std::set<int> &mines, std::set<int> &walls, std::set<int> &shells, std::set<int> &friendlyTanks, std::set<int> &enemyTanks, SatelliteView &satellite_view)
    {
        int myX = -1, myY = -1;

        lastSatellite.assign(playerGameHeight, std::vector<char>(playerGameWidth, ' '));
        for (size_t i = 0; i < playerGameHeight; ++i)
        {
            for (size_t j = 0; j < playerGameWidth; ++j)
            {
                char object = satellite_view.getObjectAt(j, i);
                int id = UC::bijection(j, i);
                lastSatellite[i][j] = object;
                if (object == '%')
                {
                    friendlyTanks.insert(id);
                    myX = j;
                    myY = i;
                }
                else if (object == '@')
                    mines.insert(id);
                else if (object == '#')
                    walls.insert(id);
                else if (object == '*')
                    shells.insert(id);
                else if (object >= '0' && object <= '9')
                {
                    int tankOwner = object - '0';

                    if (tankOwner == player_index)
                        friendlyTanks.insert(id);
                    else
                        enemyTanks.insert(id);
                }
            }
        }
        return std::make_pair(myX, myY);
    }

    bool MyPlayer::isInRedZone(int x, int y, std::set<int> shellsPositions, std::set<int> enemies) const
    {
        std::pair<int, int> pos = {x, y};

        for (const auto &shellPos : shellsPositions)
        {
            std::pair<int, int> shell = UC::inverseBijection(shellPos);
            if (manhattanDistance(pos.first, pos.second, shell.first, shell.second) <= 3)
                return true;
        }

        for (const auto &enemyId : enemies)
        {
            std::pair<int, int> enemyPos = UC::inverseBijection(enemyId);
            if (manhattanDistance(x, y, enemyPos.first, enemyPos.second) <= 4)
                return true;
        }

        return false;
    }

    int MyPlayer::getTankId(std::pair<int, int> tankPos)
    {
        for (auto &[id, path] : tanksPlannedPaths)
        {
            for (auto pos : path)
            {
                if (pos.first == tankPos.first && pos.second == tankPos.second)
                {
                    return id;
                }
            }
        }
        return 0; // shouldn't get here
    }

    void MyPlayer::initializeTanksData()
    {
        int tankId = 0;
        for (size_t i = 0; i < playerGameHeight; ++i)
        {
            for (size_t j = 0; j < playerGameWidth; ++j)
            {
                if (lastSatellite[i][j] == '%' || lastSatellite[i][j] - '0' == player_index)
                {
                    tankRoles[tankId] = "Unknown";
                    tanksPlannedPaths[tankId] = {std::make_pair(j, i)};
                    tanksPlannedActions[tankId] = {ActionRequest::GetBattleInfo};
                    tanksRemainingShells[tankId] = num_shells;
                    tanksDirection[tankId] = (player_index == 1) ? UC::Direction::L : UC::Direction::R;
                    tankId++;
                }
            }
        }
        gotBattleInfo = true;
    }

    UC::Direction MyPlayer::updateTankDirection(int tankId)
    {

        UC::Direction lastDir = tanksDirection[tankId];
        for (auto action : tanksPlannedActions[tankId])
        {
            if (action == ActionRequest::RotateLeft45)
            {
                lastDir += 0.875;
            }
            else if (action == ActionRequest::RotateLeft90)
            {
                lastDir += 0.75;
            }
            else if (action == ActionRequest::RotateRight90)
            {
                lastDir += 0.25;
            }
            else if (action == ActionRequest::RotateRight45)
            {
                lastDir += 0.125;
            }
        }
        return lastDir;
    }
}
