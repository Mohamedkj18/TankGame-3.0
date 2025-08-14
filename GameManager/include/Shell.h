#pragma once

#include "GameObject.h"

namespace GameManager_212788293_212497127
{
    // ========================= CLASS: Shell =========================

    class Shell : public GameObject
    {
    public:
        Shell(int x, int y, Direction dir, std::shared_ptr<GameManager> game);
        bool checkForAWall();
    };
}