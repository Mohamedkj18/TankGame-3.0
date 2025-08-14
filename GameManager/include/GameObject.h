#pragma once
#include <memory>
#include "DirectionUtils.h"

namespace GameManager_212788293_212497127
{
    class GameManager; // Forward declaration
    // ========================= CLASS: GameObject =========================
    class GameObject
    {
    protected:
        int x;
        int y;
        Direction direction;
        std::shared_ptr<GameManager_212788293_212497127::GameManager> game;

    public:
        GameObject(int x, int y, Direction dir, std::shared_ptr<GameManager> game);
        int getX();
        int getY();
        virtual ~GameObject() = default;

        Direction getDirection() { return direction; }
        virtual bool checkForAWall() = 0;
        bool moveForward();
        void updatePosition(Direction dir);
    };
}