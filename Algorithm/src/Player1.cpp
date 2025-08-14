#include "MyPlayer.h"
#include "common/PlayerRegistration.h"

namespace Algorithm_212788293_212497127
{

    REGISTER_PLAYER(Player_212788293_212497127);

    std::unique_ptr<Algorithm_212788293_212497127::Role> Player_212788293_212497127::createRole(int tankId, std::pair<int, int> pos, EnemyScanResult scan, std::set<int> shells, std::set<int> enemyTanks, int)
    {
        int x = pos.first, y = pos.second;

        int remainingShells = tanksRemainingShells[tankId];
        // If in red zone or under threat → Evasior
        if (isInRedZone(x, y, shells, enemyTanks) || scan.closestDistance <= 5)
        {
            tankRoles[tankId] = "Evasior";
            return std::make_unique<EvasiorRole>(5, playerGameWidth, playerGameHeight);
        }

        // If has LOS and is at decent range → Defender
        if (scan.hasLineOfSight && remainingShells > 0)
        {
            tankRoles[tankId] = "Defender";
            return std::make_unique<DefenderRole>(5, playerGameWidth, playerGameHeight);
        }

        // else → Decoy

        tankRoles[tankId] = "Decoy";
        return std::make_unique<DecoyRole>(5, playerGameWidth, playerGameHeight);
    }

    bool Player_212788293_212497127::shouldKeepRole(int tankId, const std::pair<int, int> &pos, const std::string &role, EnemyScanResult scan, std::set<int> shells, std::set<int> enemyTanks, int)
    {
        int remainingShells = tanksRemainingShells[tankId];

        if (isInRedZone(pos.first, pos.second, shells, enemyTanks) || scan.closestDistance <= 5)
            return false;

        if (role == "Defender")
            return scan.hasLineOfSight && scan.closestDistance >= 3 && remainingShells > 0;

        if (role == "Decoy")
            return isInOpen(pos.first, pos.second) && scan.closestDistance > 2;

        if (role == "Evasior")
            return isInRedZone(pos.first, pos.second, shells, enemyTanks) || scan.closestDistance <= 5;

        return false;
    }

    Player_212788293_212497127::~Player_212788293_212497127() = default;
}