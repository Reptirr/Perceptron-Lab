// NeuroNetwork.h
#pragma once

#include <Layer.h>
#include <vector>

class NeuroNetwork {
private:
    std::vector<Layer> layers;
    ActivationType activationType;
    int inputSize = 1;

public:
    NeuroNetwork(const std::vector<int> conf, ActivationType activationType1);

    std::vector<double> forward(const std::vector<double>& input);
    double train(const std::vector<double>& input,
                 const std::vector<double>& target,
                 double lr);
    void trainBatch(const std::vector<std::vector<double>>& inputs,
                    const std::vector<std::vector<double>>& targets,
                    int epochs, double lr);
    double loss(const std::vector<double>& out, const std::vector<double>& target);

    // +/- commands
    void addLayer();
    void removeLayer();
    void addNode(int layerIndex);      // Добавление нейрона (0 = входной слой)
    void removeNode(int layerIndex);   // Удаление нейрона (0 = входной слой)

    const std::vector<Layer>& getLayers() const { return layers; }
    std::vector<Layer>& getLayers() { return layers; }
    int getMaxNodes();
    const int getInputSize() const { return inputSize; }  // новый геттер

    void setActivationType(ActivationType type) {
        activationType = type;

        for (auto &layer: layers) {
        }
    }

    void randomizeWeights();
    void resetValues();
    void resetBias();

    // import/export
    void saveModel(std::ostream& to);
    void loadModel(std::istream& from);

    friend std::ostream& operator<<(std::ostream& os, const NeuroNetwork& net);
};