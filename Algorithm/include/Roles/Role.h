#pragma once

#include "UserCommon/DirectionUtils.h"
#include <string>
#include <set>
#include <memory>

class TankAlgorithm;
namespace UC = UserCommon_212788293_212497127;

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

        int rotateTowards(UC::Direction &currentDirection, UC::Direction desiredDir, int step);
        UC::Direction getDirectionFromPosition(std::pair<int, int> current, std::pair<int, int> target);
        double getAngleFromDirections(UC::Direction &directionStr, UC::Direction &desiredDir);
    };
}