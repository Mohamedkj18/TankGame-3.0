#include <iostream>
#include <unordered_map>
#include <memory>
#include "Shell.h"
#include "GameManager.h"

namespace GameManager_212788293_212497127
{
    // ------------------------ Shell ------------------------

    Shell::Shell(int x, int y, UC::Direction dir, GameManager* game)
        : GameObject(x, y, dir, game) {}

    bool Shell::checkForAWall()
    {
        Wall *currWall;
        int wallPos;
        std::string n;
        std::unordered_map<int, Wall> &walls = game->getWalls();
        wallPos = game->bijection(x, y);
        if (walls.count(wallPos))
        {
            currWall = &walls[wallPos];
            currWall->health -= 1;
            updatePosition(direction);
            return true;
        }
        return false;
    }
}