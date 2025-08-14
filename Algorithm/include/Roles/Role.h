#pragma once

#include "ActionRequest.h"
#include "DirectionUtils.h"
#include <string>
#include <set>
#include <memory>

namespace Algorithm_212788293_212497127
{
    class TankAlgorithm_212788293_212497127;

    class Role
    {

    public:
        virtual ~Role() = default;
        Role(int maxMovesPerUpdate, int gameWidth = 0, int gameHeight = 0)
            : maxMovesPerUpdate(maxMovesPerUpdate), gameWidth(gameWidth), gameHeight(gameHeight) {}
        virtual std::vector<std::pair<int, int>> prepareActions(TankAlgorithm_212788293_212497127 &algo) = 0;

        virtual std::string getRoleName() const = 0;
        virtual std::unique_ptr<Role> clone() const = 0;

        ActionRequest getNextAction();

    protected:
        int maxMovesPerUpdate;
        std::vector<ActionRequest> nextMoves;
        int gameWidth;
        int gameHeight;

        int rotateTowards(Direction &currentDirection, Direction desiredDir, int step);
        Direction getDirectionFromPosition(std::pair<int, int> current, std::pair<int, int> target);
        double getAngleFromDirections(Direction &directionStr, Direction &desiredDir);
    };
}