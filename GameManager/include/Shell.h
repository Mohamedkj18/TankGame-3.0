#pragma once

#include "GameObject.h"

// ========================= CLASS: Shell =========================

class Shell : public GameObject
{
public:
    Shell(int x, int y, Direction dir, std::shared_ptr<GameManager> game);
    bool checkForAWall();
};