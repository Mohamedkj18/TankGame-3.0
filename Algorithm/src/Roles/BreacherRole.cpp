#include "Roles/BreacherRole.h"
#include "MyTankAlgorithm.h"

namespace Algorithm_212788293_212497127
{
    std::vector<std::pair<int, int>> BreacherRole::prepareActions(TankAlgorithm_212788293_212497127 &algo)
    {
        nextMoves.clear();

        // Snapshot current state
        std::pair<int, int> myPos = algo.getCurrentPosition();
        UC::Direction currentDirection = algo.getCurrentDirection();

        // Primary intent: pick a wall to breach (nearest to us)
        std::pair<int, int> wallTarget = algo.attackWall(myPos);

        // If there are no walls known (edge case), just do nothing this tick
        if (wallTarget.first == 0 && wallTarget.second == 0 && wallTarget == std::pair<int, int>{0, 0})
        {
            nextMoves.push_back(ActionRequest::GetBattleInfo);
            algo.setNextMoves(nextMoves);
            return {};
        }

        // Compute a path toward the wall to improve LOS if needed
        std::vector<std::pair<int, int>> pathToWall =
            algo.getPath(myPos, wallTarget, algo.getBannedPositionsForTank());
        algo.setBFSPath(pathToWall);

        int step = 0;
        std::pair<int, int> currPos = myPos;

        // 1) Try to rotate toward the wall and shoot it if possible
        UC::Direction desiredDir = getDirectionFromPosition(currPos, wallTarget);

        // If not facing wall yet, rotate as much as we can this tick
        if (currentDirection != desiredDir)
        {
            step = rotateTowards(currentDirection, desiredDir, step);
        }

        // If aligned, prefer to shoot (breach) before moving
        if (step < maxMovesPerUpdate && currentDirection == desiredDir)
        {
            if (algo.shouldShoot(currentDirection, currPos))
            {
                nextMoves.push_back(ActionRequest::Shoot);
                ++step;

                // Ask for fresh info after the shot so Player can recompute paths/roles
                if (step < maxMovesPerUpdate)
                {
                    nextMoves.push_back(ActionRequest::GetBattleInfo);
                    ++step;
                }

                // Commit and exit early — breaching is our whole purpose this tick
                algo.setCurrentDirection(currentDirection);
                algo.setCurrentPosition(currPos);
                algo.setNextMoves(nextMoves);
                return pathToWall;
            }
        }

        // 2) If we can’t shoot yet (no LOS or not aligned), micro-advance toward a better LOS
        //    Take at most one forward step from the planned path this tick.
        if (step < maxMovesPerUpdate && !pathToWall.empty())
        {
            // Ensure we’re facing the next path step
            const auto &nextStepPos = pathToWall.front();
            UC::Direction stepDir = getDirectionFromPosition(currPos, nextStepPos);

            if (currentDirection != stepDir)
            {
                step = rotateTowards(currentDirection, stepDir, step);
            }

            if (step < maxMovesPerUpdate)
            {
                nextMoves.push_back(ActionRequest::MoveForward);
                ++step;
                currPos = nextStepPos;
            }

            // After moving, if we still have budget, try to shoot (maybe LOS just opened)
            if (step < maxMovesPerUpdate && algo.shouldShoot(currentDirection, currPos))
            {
                nextMoves.push_back(ActionRequest::Shoot);
                ++step;
            }

            // Request updated info so the Player/Algorithm re-evaluates post-move posture
            if (step < maxMovesPerUpdate)
            {
                nextMoves.push_back(ActionRequest::GetBattleInfo);
                ++step;
            }
        }

        // 3) If we did nothing so far, don’t burn a tick idly
        if (nextMoves.empty())
        {
            // Try one rotate toward target (if we didn’t rotate earlier); else DoNothing
            if (currentDirection != desiredDir)
            {
                rotateTowards(currentDirection, desiredDir, 0);
            }
            else
            {
                nextMoves.push_back(ActionRequest::DoNothing);
            }
        }

        // Persist state for next tick
        algo.setCurrentDirection(currentDirection);
        algo.setCurrentPosition(currPos);
        algo.setNextMoves(nextMoves);
        return pathToWall;
    }
}
