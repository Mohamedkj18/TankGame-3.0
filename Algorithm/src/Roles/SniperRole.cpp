
#include "SniperRole.h"
#include "MyTankAlgorithm.h"

namespace Algorithm_212788293_212497127
{
    std::vector<std::pair<int, int>> SniperRole::prepareActions(TankAlgorithm_212788293_212497127 &algo)
    {
        nextMoves.clear();

        Direction currentDirection = algo.getCurrentDirection();
        std::pair<int, int> pos = algo.getCurrentPosition();

        if (algo.shouldShoot(currentDirection, pos))
        {
            nextMoves.push_back(ActionRequest::Shoot);
        }
        else
        {
            nextMoves.push_back(ActionRequest::RotateRight45);
            currentDirection += 0.125;
        }

        algo.setCurrentDirection(currentDirection);
        algo.setNextMoves(nextMoves);
        return std::vector<std::pair<int, int>>{algo.getCurrentPosition()};
    }
}