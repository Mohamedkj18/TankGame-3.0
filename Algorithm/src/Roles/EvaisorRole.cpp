#include "Roles/EvasiorRole.h"
#include "MyTankAlgorithm.h"

namespace Algorithm_212788293_212497127
{
    std::vector<std::pair<int, int>> EvasiorRole::prepareActions(TankAlgorithm_212788293_212497127 &algo)
    {
        std::vector<std::pair<int, int>> path;
        std::pair<int, int> myPos = algo.getCurrentPosition();
        std::set<std::pair<int, int>> shells = algo.getShells();

        std::set<std::pair<int, int>> redZone = createRedZone(shells, 5);
        concatenateSets(redZone, algo.getBannedPositionsForTank());
        concatenateSets(redZone, createRedZone(transformToPairs(algo.getEnemyTanks()), 2));
        std::pair<int, int> target = algo.findFirstLegalLocationToFlee(myPos, redZone);
        path = algo.getPath(myPos, target, redZone);
        if (path.empty())
        {
            if (algo.shouldShoot(algo.getCurrentDirection(), algo.getCurrentPosition()))
                nextMoves.push_back(ActionRequest::Shoot);
            algo.setNextMoves(nextMoves);
            return path;
        }
        nextMoves = getNextMoves(path, algo);

        algo.setNextMoves(nextMoves);
        return path;
    }

    std::set<std::pair<int, int>> EvasiorRole::createRedZone(std::set<std::pair<int, int>> shells, int distFromTarget)
    {

        std::set<std::pair<int, int>> redZone;
        for (auto pos : shells)
        {
            for (int k = 1; k < distFromTarget; k++)
            {
                for (int i = -1; i <= 1; ++i)
                {
                    for (int j = -1; j <= 1; ++j)
                    {
                        if (i == 0 && j == 0)
                            continue;

                        int x = (pos.first + i * k + gameWidth * k) % gameWidth;
                        int y = (pos.second + j * k + gameHeight * k) % gameHeight;
                        redZone.insert(std::make_pair(x, y));
                    }
                }
            }
        }
        return redZone;
    }

    std::vector<ActionRequest> EvasiorRole::getNextMoves(std::vector<std::pair<int, int>> path, TankAlgorithm_212788293_212497127 &algo)
    {
        std::pair<int, int> pos = algo.getCurrentPosition();
        UC::Direction currentDirection = algo.getCurrentDirection();
        int step = 0;

        for (const auto &pathStep : path)
        {
            if (step >= maxMovesPerUpdate)
                break;

            // Determine the direction to this path step
            UC::Direction desiredDir = getDirectionFromPosition(pos, pathStep);

            // If not facing desired direction, rotate first
            if (currentDirection != desiredDir)
            {
                step = rotateTowards(currentDirection, desiredDir, step);
                if (step >= maxMovesPerUpdate)
                    break;
            }
            if (step < maxMovesPerUpdate)
            {
                nextMoves.push_back(ActionRequest::MoveForward);
                step++;
            }

            pos = pathStep;
        }

        nextMoves.push_back(ActionRequest::GetBattleInfo);
        algo.setCurrentPosition(pos);
        algo.setCurrentDirection(currentDirection);
        return nextMoves;
    }

    void EvasiorRole::concatenateSets(std::set<std::pair<int, int>> targetSet, std::set<std::pair<int, int>> setToBeAdded)
    {
        for (const auto &pos : setToBeAdded)
        {
            targetSet.insert(pos);
        }
    }

    std::set<std::pair<int, int>> EvasiorRole::transformToPairs(std::set<int> toBeTransformed)
    {
        std::set<std::pair<int, int>> pairsSet;
        for (const auto pos : toBeTransformed)
        {
            pairsSet.insert(UC::inverseBijection(pos));
        }
        return pairsSet;
    }
}