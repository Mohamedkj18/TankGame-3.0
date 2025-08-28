#include "Roles/DefenderRole.h"
#include "MyTankAlgorithm.h"

namespace Algorithm_212788293_212497127
{
    std::vector<std::pair<int, int>> DefenderRole::prepareActions(TankAlgorithm_212788293_212497127 &algo)
    {
        nextMoves.clear();

        std::pair<int, int> myPos = algo.getCurrentPosition();
        UC::Direction currentDir = algo.getCurrentDirection();

        // 1) Scan for nearest enemy (within range 7)
        auto targetOpt = algo.findEnemyInRange(myPos, 20);
        // If no known enemy -> just request info
        if (!targetOpt.has_value())
        {
            nextMoves.push_back(ActionRequest::GetBattleInfo);
            algo.setNextMoves(nextMoves);
            return {};
        }

        std::pair<int, int> target = *targetOpt;
        // 2) Rotate toward target
        UC::Direction desiredDir = getDirectionFromPosition(myPos, target);
        if (currentDir != desiredDir)
        {
            rotateTowards(currentDir, desiredDir, 0); // pushes rotate actions
        }

        // 3) If aligned, try shooting
        if (algo.shouldShoot(currentDir, myPos))
        {
            nextMoves.push_back(ActionRequest::Shoot);
            nextMoves.push_back(ActionRequest::GetBattleInfo); // re-plan next tick
        }
        else
        {
            // 4) No shot yet, micro-advance toward target
            auto path = algo.getPath(myPos, target, algo.getBannedPositionsForTank());
            if (!path.empty())
            {
                UC::Direction stepDir = getDirectionFromPosition(myPos, path.front());
                if (currentDir != stepDir)
                    rotateTowards(currentDir, stepDir, 0);

                nextMoves.push_back(ActionRequest::MoveForward);
                nextMoves.push_back(ActionRequest::GetBattleInfo);
                myPos = path.front();
            }
        }

        // Persist
        algo.setCurrentDirection(currentDir);
        algo.setCurrentPosition(myPos);
        algo.setNextMoves(nextMoves);
        return {};
    }
}
