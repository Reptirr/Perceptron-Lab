#pragma once
#include <random>

inline double getRandomWeight() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> dis(-0.5, 0.5);
    return dis(gen);
}