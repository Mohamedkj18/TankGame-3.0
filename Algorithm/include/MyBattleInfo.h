#pragma once

#ifndef MY_BATTLE_INFO_H
#define MY_BATTLE_INFO_H

#include <set>
#include <vector>
#include <string>
#include <memory>
#include "common/BattleInfo.h"
#include "Roles/Role.h"

namespace Algorithm_212788293_212497127 {

class Role;

class MyBattleInfo : public BattleInfo
{
private:
    int width, height;
    int myX, myY;
    std::unique_ptr<Algorithm_212788293_212497127::Role> role;
    bool shouldKeepRole = false;
    std::set<int> friendlyTanks;
    std::set<int> enemyTanks;
    std::set<int> mines;
    std::set<int> walls;
    std::set<int> shells;
    std::vector<std::pair<int, int>> path;
    std::vector<ActionRequest> planedActions;
    std::set<std::pair<int, int>> plannedPositions;

public:
    MyBattleInfo(int width, int height,
                 const std::set<int> &friendlyTanks,
                 const std::set<int> &enemyTanks,
                 const std::set<int> &mines,
                 const std::set<int> &walls,
                 const std::set<int> &shells);

    int getWidth() const;
    int getHeight() const;

    std::set<int> &getFriendlyTanks();
    std::set<int> &getEnemyTanks();
    std::set<int> &getMines();
    std::set<int> &getWalls();
    std::set<int> &getShells();

    void setMyXPosition(int x);
    void setMyYPosition(int y);
    int getMyXPosition() const;
    int getMyYPosition() const;
    std::vector<std::pair<int, int>> getPath();
    void setPath(const std::vector<std::pair<int, int>> &path);

    void setRole(std::unique_ptr<Algorithm_212788293_212497127::Role> &&newRole);
    const Algorithm_212788293_212497127::Role &getRole() const;

    bool isMine(int x, int y) const;
    bool isWall(int x, int y) const;

    bool isShell(int x, int y) const;
    bool isEnemyTank(int x, int y) const;
    bool isFriendlyTank(int x, int y) const;


    std::unique_ptr<Algorithm_212788293_212497127::Role> extractRole();
    bool getShouldKeepRole() const { return shouldKeepRole; }
    void setShouldKeepRole(bool value) { shouldKeepRole = value; }

    std::vector<ActionRequest> getPlannedActions() { return planedActions; }
    void setPlannedActions(std::vector<ActionRequest> actions) { planedActions = actions; }

    std::set<std::pair<int, int>> getPlannedPositions() { return plannedPositions; }
    void setPlannedPositions(std::set<std::pair<int, int>> positions) { plannedPositions = positions; }
};
}
#endif
