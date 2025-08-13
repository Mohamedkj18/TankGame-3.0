#include "MyBattleInfo.h"

MyBattleInfo::MyBattleInfo(int width, int height,
                           const std::set<int> &friendlyTanks,
                           const std::set<int> &enemyTanks,
                           const std::set<int> &mines,
                           const std::set<int> &walls,
                           const std::set<int> &shells)
    : width(width), height(height), myX(0), myY(0),
      friendlyTanks(friendlyTanks), enemyTanks(enemyTanks),
      mines(mines), walls(walls), shells(shells)
{
}

int MyBattleInfo::getWidth() const { return width; }
int MyBattleInfo::getHeight() const { return height; }

std::set<int> &MyBattleInfo::getFriendlyTanks() { return friendlyTanks; }
std::set<int> &MyBattleInfo::getEnemyTanks() { return enemyTanks; }
std::set<int> &MyBattleInfo::getMines() { return mines; }
std::set<int> &MyBattleInfo::getWalls() { return walls; }
std::set<int> &MyBattleInfo::getShells() { return shells; }

void MyBattleInfo::setMyXPosition(int x) { myX = x; }
void MyBattleInfo::setMyYPosition(int y) { myY = y; }

int MyBattleInfo::getMyXPosition() const { return myX; }
int MyBattleInfo::getMyYPosition() const { return myY; }

void MyBattleInfo::setPath(const std::vector<std::pair<int, int>> &path)
{
    this->path = path; // Store moves for the tank
}
std::vector<std::pair<int, int>> MyBattleInfo::getPath()
{
    return path;
}

void MyBattleInfo::setRole(std::unique_ptr<Role> &&newRole)
{
    role = std::move(newRole);
}

const Role &MyBattleInfo::getRole() const
{
    return *role;
}
bool MyBattleInfo::isMine(int x, int y) const
{
    return mines.count(bijection(x, y)) > 0;
}

bool MyBattleInfo::isWall(int x, int y) const
{
    return walls.count(bijection(x, y)) > 0;
}

bool MyBattleInfo::isShell(int x, int y) const
{
    return shells.count(bijection(x, y)) > 0;
}

bool MyBattleInfo::isEnemyTank(int x, int y) const
{
    return enemyTanks.count(bijection(x, y)) > 0;
}

bool MyBattleInfo::isFriendlyTank(int x, int y) const
{
    return friendlyTanks.count(bijection(x, y)) > 0;
}

int MyBattleInfo::bijection(int x, int y) const
{
    return ((x + y) * (x + y + 1)) / 2 + y;
}

std::unique_ptr<Role> MyBattleInfo::extractRole()
{
    return std::move(role); // Transfers ownership
}
