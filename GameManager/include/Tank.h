#pragma once

#include <string>
#include <set>
#include "GameObject.h"
#include "GameManager.h"
#include "common/TankAlgorithm.h"

namespace GameManager_212788293_212497127
{
    class GameManager;
    // ========================= CLASS: Tank =========================

    class Tank : public GameObject
    {
    private:
        int playerId;
        int artilleryShells;
        int tankId;
        int tankGlobalId;
        bool destroyed;
        int cantShoot;
        int reverseCharge = 0;
        bool reverseQueued = false;
        bool reverseReady = false;
        std::unique_ptr<TankAlgorithm> tankAlgorithm;
        ActionRequest lastMove;

    public:
        Tank(int x, int y, UC::Direction dir, GameManager* game, int playerId, int shells, int tankId, int tankGlobalId);

        // Position and state
        int getPlayerId();
        int getTankId();
        int getTankGlobalId();
        ActionRequest getLastMove();
        bool checkForAWall();

        // Movement
        void moveBackwards();
        void rotateTank(double angle);
        void setDirection(std::string directionStr);
        void setLastMove(ActionRequest currentMove);
        TankAlgorithm *getTankAlgorithm();
        void setTankAlgorithm(std::unique_ptr<TankAlgorithm> algorithm);

        // Firing
        void fire();

        // Damage
        void hit();

        // Shooting cooldown
        void incrementCantShoot();
        void resetCantShoot();
        bool canShoot();
        int getCantShoot();

        // 🔁 Reverse state
        bool isReverseQueued() const;
        bool isReverseReady() const;
        int getReverseCharge() const;
        void queueReverse();
        void incrementReverseCharge();
        void resetReverseState();
        void executeReverse();
        void ignoreMove();
    };
}