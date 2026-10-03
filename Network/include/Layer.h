#pragma once

#include <ActivationType.h>
#include <Matrix.h>
#include <vector>


class Layer {
    std::vector<double> values;
    std::vector<double> prevValues;
    std::vector<double> zValues;

    Matrix weights; // [to][from]
    std::vector<double> bias;

    ActivationType activationType;

public:
    Layer(int numNeurons, int numPrevNeurons, ActivationType aType);
    Layer(Matrix weights, std::vector<double> bias, ActivationType activationType);
    std::vector<double> forward(const std::vector<double>& input);
    std::vector<double> backprop(const std::vector<double>& delta, double lr);

    const std::vector<double>& getValues() const { return values; }
    const std::vector<double>& getBias() const { return bias; }
    const Matrix& getWeights() const { return weights; }
    const ActivationType& getActivationType() const { return activationType; }

    void addFromWeight();
    void removeFromWeight();
    void addNode();
    void removeNode();

    void setActivationType(ActivationType type);

    void randomizeWeights();
    void resetValues();
    void resetBias();

    friend std::ostream& operator<<(std::ostream& os, const Layer& layer);
};