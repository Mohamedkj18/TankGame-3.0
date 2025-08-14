
#include "DefenderRole.h"
#include "MyTankAlgorithm.h"
#include "DirectionUtils.h"

namespace Algorithm_212788293_212497127
{
    std::vector<std::pair<int, int>> DefenderRole::prepareActions(TankAlgorithm_212788293_212497127 &algo)
    {
        nextMoves.clear();

        Direction currentDirection = algo.getCurrentDirection();
        std::pair<int, int> myPos = algo.getCurrentPosition();

        // Look for visible enemy tanks within range 4 (Manhattan distance)
        std::optional<std::pair<int, int>> target = algo.findEnemyInRange(myPos, 6);
        if (target.has_value())
        {
            Direction desiredDir = getDirectionFromPosition(myPos, target.value());

            if (desiredDir != currentDirection)
            {
                rotateTowards(currentDirection, desiredDir, 0);
            }
            if (algo.shouldShoot(currentDirection, myPos))
            {
                nextMoves.push_back(ActionRequest::Shoot);
            }
        }

        algo.setCurrentDirection(currentDirection);
        algo.setNextMoves(nextMoves);
        return std::vector<std::pair<int, int>>{myPos};
    }
}