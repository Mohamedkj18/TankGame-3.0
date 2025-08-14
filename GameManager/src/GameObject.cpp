#include <memory>
#include "GameObject.h"
#include "GameManager.h"
#include "DirectionUtils.h"

namespace GameManager_212788293_212497127
{
    // ------------------------ MovingGameObject ------------------------

    GameObject::GameObject(int x, int y, Direction dir, std::shared_ptr<GameManager> game)
        : x(x), y(y), direction(dir), game(std::move(game)) {}

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

            updatePosition(DirectionsUtils::reverseDirection[direction]);
            return false;
        }

        return true;
    }

    void GameObject::updatePosition(Direction dir)
    {
        std::array<int, 2> d = DirectionsUtils::stringToIntDirection[dir];
        x = (x + d[0] + game->getWidth() * 2) % (game->getWidth() * 2);
        y = (y + d[1] + game->getHeight() * 2) % (game->getHeight() * 2);
    }
}