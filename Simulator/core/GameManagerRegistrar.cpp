#include "GameManagerRegistrar.h"

GameManagerRegistrar GameManagerRegistrar::registrar;
size_t GameManagerRegistrar::GameManagerCount = 0;
GameManagerRegistrar& GameManagerRegistrar::getGameManagerRegistrar() {
    return registrar;
}
