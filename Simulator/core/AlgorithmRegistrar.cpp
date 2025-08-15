#include "AlgorithmRegistrar.h"

AlgorithmRegistrar AlgorithmRegistrar::registrar;
void AlgorithmRegistrar::initializeAlgoID(){
    algoID = 0;
}
AlgorithmRegistrar& AlgorithmRegistrar::getAlgorithmRegistrar() {
    return registrar;
}

size_t AlgorithmRegistrar::getAlgoID() {
    return algoID;
}