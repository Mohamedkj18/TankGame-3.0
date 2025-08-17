#include "Roles/Role.h"
#include "MyTankAlgorithm.h"

namespace Algorithm_212788293_212497127
{
    int Role::rotateTowards(UC::Direction &currentDirection, UC::Direction desiredDir, int step)
    {
        double angle = getAngleFromDirections(currentDirection, desiredDir);

        if (angle == 0.125)
            nextMoves.push_back(ActionRequest::RotateRight45);

        else if (angle == 0.25)
            nextMoves.push_back(ActionRequest::RotateRight90);

        else if (angle == 0.375)
        {
            nextMoves.push_back(ActionRequest::RotateRight45);
            step++;
            currentDirection += 0.125;
            if (step >= maxMovesPerUpdate)
                return step;
            nextMoves.push_back(ActionRequest::RotateRight90);
        }
        else if (angle == 0.5)
        {
            nextMoves.push_back(ActionRequest::RotateRight90);
            step++;
            currentDirection += 0.25;
            if (step >= maxMovesPerUpdate)
                return step;
            nextMoves.push_back(ActionRequest::RotateRight90);
        }
        else if (angle == 0.625)
        {
            nextMoves.push_back(ActionRequest::RotateLeft90);
            currentDirection += 0.75;
            step++;
            if (step >= maxMovesPerUpdate)
                return step;
            nextMoves.push_back(ActionRequest::RotateLeft45);
        }
        else if (angle == 0.75)
            nextMoves.push_back(ActionRequest::RotateLeft90);
        else if (angle == 0.875)
            nextMoves.push_back(ActionRequest::RotateLeft45);
        else
            nextMoves.push_back(ActionRequest::DoNothing);

        currentDirection = desiredDir;
        return ++step;
    }

    UC::Direction Role::getDirectionFromPosition(std::pair<int, int> current, std::pair<int, int> target)
    {
        // int xDiff = target.first - current.first;
        // xDiff = xDiff > 1 || xDiff == -1 ? -1 : (xDiff + gameWidth) % gameWidth;
        // int yDiff = target.second - current.second;
        // yDiff = yDiff > 1 || yDiff == -1 ? -1 : (yDiff + gameHeight) % gameHeight;
        // return pairToDirections[{xDiff, yDiff}];
        int dx = target.first - current.first;
        int dy = target.second - current.second;

        // Handle wrap-around (toroidal logic)
        if (dx > gameWidth / 2)
            dx -= gameWidth;
        if (dx < -gameWidth / 2)
            dx += gameWidth;

        if (dy > gameHeight / 2)
            dy -= gameHeight;
        if (dy < -gameHeight / 2)
            dy += gameHeight;

        // Normalize
        dx = (dx == 0) ? 0 : (dx > 0 ? 1 : -1);
        dy = (dy == 0) ? 0 : (dy > 0 ? 1 : -1);

        return UC::DirectionsUtils::pairToDirections[{dx, dy}];
    }

    double Role::getAngleFromDirections(UC::Direction &orgDir, UC::Direction &desiredDir)
    {
        UC::Direction dirToCheck;
        double angle, rotate = 0.125;
        for (int i = 0; i < 8; ++i)
        {
            angle = i * rotate;
            dirToCheck = orgDir;
            dirToCheck += (angle);
            if (dirToCheck == desiredDir)
            {
                return angle;
            }
        }
        return 0.0; // Should not happen, but just in case
    }

    ActionRequest Role::getNextAction()
    {
        if (nextMoves.empty())
        {
            return ActionRequest::GetBattleInfo;
        }
        ActionRequest action = nextMoves.front();
        nextMoves.erase(nextMoves.begin());
        return action;
    }
}