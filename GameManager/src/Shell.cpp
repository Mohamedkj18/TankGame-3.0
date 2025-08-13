#include <iostream>
#include <unordered_map>
#include <memory>
#include "Shell.h"
#include "GameManager.h"

// ------------------------ Shell ------------------------

Shell::Shell(int x, int y, Direction dir, std::shared_ptr<GameManager> game)
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
