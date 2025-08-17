#pragma once

#include "GameObject.h"

namespace GameManager_212788293_212497127
{
    // ========================= CLASS: Shell =========================

    class Shell : public GameObject
    {
    public:
        Shell(int x, int y, UC::Direction dir, GameManager* game);
        bool checkForAWall();
    };
}