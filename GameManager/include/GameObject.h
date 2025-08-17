#pragma once
#include <memory>
#include "UserCommon/DirectionUtils.h"

namespace UC = UserCommon_212788293_212497127;


namespace GameManager_212788293_212497127
{
    class GameManager; // Forward declaration
    // ========================= CLASS: GameObject =========================
    class GameObject
    {
    protected:
        int x;
        int y;
        UC::Direction direction;
        GameManager_212788293_212497127::GameManager* game;

    public:
        GameObject(int x, int y, UC::Direction dir, GameManager* game);
        int getX();
        int getY();
        virtual ~GameObject() = default;

        UC::Direction getDirection() { return direction; }
        virtual bool checkForAWall() = 0;
        bool moveForward();
        void updatePosition(UC::Direction dir);
    };
}