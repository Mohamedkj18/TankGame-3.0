#include <memory>
#include "GameObject.h"
#include "GameManager.h"
#include "UserCommon/DirectionUtils.h"

namespace GameManager_212788293_212497127
{
    // ------------------------ MovingGameObject ------------------------

    GameObject::GameObject(int x, int y, UC::Direction dir, GameManager* game)
        : x(x), y(y), direction(dir), game(game) {}

    int GameObject::getX()
    {
        return x;
    }

    int GameObject::getY()
    {
        return y;
    }

    bool GameObject::moveForward()
    {
        updatePosition(direction);
        if (checkForAWall())
        {

            updatePosition(UC::DirectionsUtils::reverseDirection[direction]);
            return false;
        }

        return true;
    }

    void GameObject::updatePosition(UC::Direction dir)
    {
        std::array<int, 2> d = UC::DirectionsUtils::stringToIntDirection[dir];
        x = (x + d[0] + game->getWidth() * 2) % (game->getWidth() * 2);
        y = (y + d[1] + game->getHeight() * 2) % (game->getHeight() * 2);
    }
}