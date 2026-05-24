#ifndef DEFAULTMODEL_H
#define DEFAULTMODEL_H

#include <vector>

#include "ActivationType.h"

struct ModelConfig {
    std::vector<int> conf{2,4,5,6,7,1};
    double lr = 0.01;
    int epochs = 5000;
    ActivationType activationType = ActivationType::LEAKYRELU;

};

inline ModelConfig DefaultModel;

#endif // DEFAULTMODEL_H
