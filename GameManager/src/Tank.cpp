#include <memory>
#include <iostream>
#include "Tank.h"
#include "Shell.h"
#include "GameManager.h"
#include "ActionRequest.h"

// ------------------------ Tank ------------------------

Tank::Tank(int x, int y, Direction dir, std::shared_ptr<GameManager> game, int playerId, int shells, int tankId, int tankGlobalId)
    : GameObject(x, y, dir, game), playerId(playerId), artilleryShells(shells), tankId(tankId), tankGlobalId(tankGlobalId)
{
    cantShoot = 0;
    destroyed = false;
    reverseQueued = false;
    reverseReady = false;
    reverseCharge = 0;
    lastMove = ActionRequest::DoNothing;
}

int Tank::getPlayerId() { return playerId; }

int Tank::getTankId() { return tankId; }

ActionRequest Tank::getLastMove() { return lastMove; }

void Tank::setLastMove(ActionRequest currentMove)
{
    lastMove = currentMove;
}

void Tank::ignoreMove()
{
    lastMove = ActionRequest::DoNothing;
}

int Tank::getTankGlobalId()
{
    return tankGlobalId;
}

void Tank::setDirection(std::string directionStr)
{
    if (DirectionsUtils::stringToDirection.find(directionStr) != DirectionsUtils::stringToDirection.end())
    {
        this->direction = DirectionsUtils::stringToDirection[directionStr];
    }
    else
    {
        std::cerr << "Invalid direction string: " << directionStr << std::endl;
    }
}

void Tank::moveBackwards()
{
    direction = DirectionsUtils::reverseDirection[direction];
    moveForward();
    direction = DirectionsUtils::reverseDirection[direction];
}

bool Tank::checkForAWall()
{
    updatePosition(direction);
    if (game->getWalls().count(game->bijection(x, y)) == 1)
    {
        updatePosition(DirectionsUtils::reverseDirection[direction]);
        lastMove = ActionRequest::DoNothing;

        return true;
    }
    updatePosition(DirectionsUtils::reverseDirection[direction]);
    if (reverseReady)
        lastMove = ActionRequest::MoveBackward;
    return false;
}

void Tank::rotateTank(double angle)
{
    if (angle == 0)
        return;
    direction += angle;
}

void Tank::fire()
{
    if (artilleryShells > 0)
    {
        artilleryShells--;
        auto shell = std::make_unique<Shell>(x, y, direction, game);
        shell->moveForward();
        game->addShell(std::move(shell));
    }
}

void Tank::hit()
{
    destroyed = true;
    game->removeTank(tankId);
}

void Tank::incrementCantShoot() { cantShoot += 1; }

void Tank::resetCantShoot() { cantShoot = 0; }

bool Tank::canShoot() { return artilleryShells > 0 && cantShoot == 0; }

int Tank::getCantShoot() { return cantShoot; }

int Tank::getReverseCharge() const { return reverseCharge; }
bool Tank::isReverseQueued() const { return reverseQueued; }
bool Tank::isReverseReady() const { return reverseReady; }

void Tank::queueReverse()
{
    reverseQueued = true;
    reverseCharge = 1;
}

void Tank::incrementReverseCharge()
{
    if (reverseCharge < 5)
        reverseCharge++;
    else
        reverseReady = true;
}

void Tank::resetReverseState()
{
    reverseQueued = false;
    reverseCharge = 0;
    reverseReady = false;
}

void Tank::executeReverse()
{
    moveBackwards();
    reverseQueued = false;
    reverseCharge = 0;
    setLastMove(ActionRequest::MoveBackward);
    reverseReady = true;
}

TankAlgorithm *Tank::getTankAlgorithm()
{
    return this->tankAlgorithm.get();
}

void Tank::setTankAlgorithm(std::unique_ptr<TankAlgorithm> algorithm)
{
    this->tankAlgorithm = std::move(algorithm);
}