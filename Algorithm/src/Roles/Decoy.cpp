#include "DecoyRole.h"
#include "MyTankAlgorithm.h"
#include "DirectionUtils.h"

namespace Algorithm_212788293_212497127
{
    std::vector<std::pair<int, int>> DecoyRole::prepareActions(TankAlgorithm_212788293_212497127 &algo)
    {
        nextMoves.clear();

        std::vector<std::pair<int, int>> path;
        std::pair<int, int> myPos = algo.getCurrentPosition();
        bool isInOpenArea = algo.isInOpen(myPos);
        bool threatIsClose = algo.findEnemyInRange(myPos, 2) != std::nullopt;

        if (isInOpenArea && !threatIsClose)
        {
            // Stay visible, don't move
            nextMoves.push_back(ActionRequest::GetBattleInfo);
            path = {myPos};
        }
        else
        {
            std::pair<int, int> target;
            if (threatIsClose)
            {
                target = algo.findNearestFriendlyTank(myPos);
            }
            else
            {
                target = {algo.getGameWidth() / 2, algo.getGameHeight() / 2};
            }

            path = algo.getPath(myPos, target, algo.getBannedPositionsForTank());
            if (!path.empty())
                path.pop_back();

            nextMoves = getNextMoves(path, algo);
        }

        algo.setNextMoves(nextMoves);
        return path;
    }

    std::vector<ActionRequest> DecoyRole::getNextMoves(std::vector<std::pair<int, int>> path, TankAlgorithm_212788293_212497127 &algo)
    {
        std::pair<int, int> pos = algo.getCurrentPosition();
        Direction currentDirection = algo.getCurrentDirection();
        int maxMovesPerUpdate = algo.getMaxMovesPerUpdate();
        int step = 0;

        for (const auto &pathStep : path)
        {
            if (step >= maxMovesPerUpdate)
                break;

            // Determine the direction to this path step
            Direction desiredDir = getDirectionFromPosition(pos, pathStep);

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
}