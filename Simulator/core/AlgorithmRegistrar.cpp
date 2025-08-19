#include "AlgorithmRegistrar.h"

AlgorithmRegistrar AlgorithmRegistrar::registrar;
size_t AlgorithmRegistrar::algoID = 0;
void AlgorithmRegistrar::initializeAlgoID(){
    algoID = 0;
}
AlgorithmRegistrar& AlgorithmRegistrar::getAlgorithmRegistrar() {
    return registrar;
}

size_t AlgorithmRegistrar::getAlgoID() {
    return algoID;
}