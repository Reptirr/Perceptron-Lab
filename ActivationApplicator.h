#pragma once
#include <cmath>
#include <stdexcept>

#include "ActivationType.h"

// --- активации ---

inline double sigmoid(double x) {
    return 1.0 / (1.0 + std::exp(-x));
}

inline double relu(double x) {
    return x > 0 ? x : 0;
}

inline double leakyRelu(double x) {
    return x > 0 ? x : 0.01 * x;
}

inline double step(double x) {
    return x >= 0.0 ? 1.0 : 0.0;
}


// --- производные (через x) ---

inline double sigmoidDerivative(double x) {
    double s = sigmoid(x);
    return s * (1.0 - s);
}

inline double reluDerivative(double x) {
    return x > 0 ? 1.0 : 0.0;
}

inline double leakyReluDerivative(double x) {
    return x > 0 ? 1.0 : 0.01;
}

inline double stepDerivative(double x) {
    return 0.0;
}

// --- (опционально) производная sigmoid через output (быстрее) ---
inline double sigmoidDerivativeFromOutput(double z) {
    return z * (1.0 - z);
}

// --- dispatcher для активации ---

inline double applyActivation(ActivationType activate, double x) {
    switch (activate) {
    case ActivationType::SIGMOID:
        return sigmoid(x);

    case ActivationType::RELU:
        return relu(x);

    case ActivationType::LEAKYRELU:
        return leakyRelu(x);

    case ActivationType::STEP:
        return step(x);

    case ActivationType::NONE:
        throw std::invalid_argument("Activation NONE is not working");

    default:
        throw std::invalid_argument("Undefined ActivationType");

    }
}

// --- dispatcher для производной ---

inline double applyActivationDerivative(ActivationType activate, double x) {
    switch (activate) {
    case ActivationType::SIGMOID:
        return sigmoidDerivative(x);

    case ActivationType::RELU:
        return reluDerivative(x);

    case ActivationType::LEAKYRELU:
        return leakyReluDerivative(x);

    case ActivationType::STEP:
        return stepDerivative(x);

    case ActivationType::NONE:
        throw std::invalid_argument("Derivative for NONE is not working");

    default:
        throw std::invalid_argument("Undefined ActivationType");
    }
}