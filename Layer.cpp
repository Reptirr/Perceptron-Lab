#include "Layer.h"
#include "utils.h"
#include "ActivationApplicator.h"

Layer::Layer(int numNeurons, int numPrevNeurons, ActivationType aType)
    : activationType(aType),
    weights(numNeurons, numPrevNeurons),
    bias(numNeurons, 0.0)
{
    values.resize(numNeurons);
    prevValues.resize(numPrevNeurons);
    zValues.resize(numNeurons);

    weights.fillRandom();
}

Layer::Layer(Matrix w, std::vector<double> b, ActivationType a)
    : weights(w), bias(b), activationType(a), values(b.size())
{

}

std::vector<double> Layer::forward(const std::vector<double>& input)
{
    // сохраняем вход слоя чтобы потом использовать в backprop
    prevValues = input;

    // считаем взвешенную сумму входов (w * x)
    Matrix m = weights.scaleColumns(input);

    // складываем по строкам, получаем входы в нейроны до активации
    std::vector<double> z = m.sumRows();

    for (size_t i = 0; i < z.size(); i++) {

        // добавляем bias к каждому нейрону
        z[i] += bias[i];

        // сохраняем значение до функции активации (нужно для обучения)
        zValues[i] = z[i];

        // прогоняем через функцию активации
        z[i] = applyActivation(activationType, z[i]);
    }

    // сохраняем итоговый результат слоя
    values = z;

    // возвращаем выход слоя
    return values;
}

std::vector<double> Layer::backprop(const std::vector<double>& delta, double lr)
{
    // проверяем, что размер ошибки совпадает с размером выхода слоя
    if (delta.size() != values.size())
        throw std::invalid_argument("wrong delta size");

    // сюда собираем ошибку для предыдущего слоя
    std::vector<double> prevDelta(prevValues.size(), 0.0);

    for (size_t i = 0; i < values.size(); i++) {

        // считаем производную функции активации для текущего нейрона
        double dActivation = applyActivationDerivative(activationType, zValues[i]);

        // локальный градиент, полученный по правилу цепочки
        double localGrad = delta[i] * dActivation;

        // градиент по смещению
        double gradB = localGrad;

        // обновляем смещение по правилу градиентного спуска
        bias[i] -= lr * gradB;

        for (size_t j = 0; j < prevValues.size(); j++) {

            // сохраняем старое значение веса, чтобы им передать ошибку назад
            double oldWeight = weights(i, j);

            // градиент по весу
            double gradW = localGrad * prevValues[j];

            // обновляем вес по правилу градиентного спуска
            weights(i, j) -= lr * gradW;

            // передаем ошибку в предыдущий слой
            prevDelta[j] += oldWeight * localGrad;
        }
    }

    // возвращаем ошибку для предыдущего слоя
    return prevDelta;
}

void Layer::randomizeWeights() {
    weights.fillRandom();
}
void Layer::resetValues() {
    values.assign(values.size(), 0);
}
void Layer::resetBias() {
    bias.assign(bias.size(), 0);
}

#include <ostream>

std::ostream& operator<<(std::ostream& os, const Layer& layer)
{
    os << "Values: [";
    for (size_t i = 0; i < layer.values.size(); i++) {
        os << layer.values[i];
        if (i + 1 != layer.values.size()) os << ", ";
    }
    os << "]\n";

    return os;
}

// +/- commands
// Добавление нейрона в *этот* слой
void Layer::addNode() {
    // 1) Новая строка весов (связи от всех prev-нейронов)
    weights.addRow(std::vector<double>(weights.getCols(), 0.0));

    // 2) Новый bias
    bias.push_back(0.0);

    // 3) Состояния нового нейрона
    values.push_back(0.0);
    zValues.push_back(0.0);

}

// Удаление последнего нейрона из *этого* слоя
void Layer::removeNode() {
    if (values.size() <= 1) return;   // нельзя удалить единственный нейрон

    // 1) Удаляем последнюю строку весов
    weights.removeRow(weights.getRows() - 1);

    // 2) Удаляем bias
    bias.pop_back();

    // 3) Удаляем состояния нейрона
    values.pop_back();
    zValues.pop_back();
}

// добавляем вес к слою. вызываем при увеличивании кол-ва нод в предыдущем слое
void Layer::addFromWeight() {
    weights.addColumn(std::vector<double>(weights.getRows(), 0.0));

    // prevValues должно отражать новое количество входов
    prevValues.resize(weights.getCols(), 0.0);
}

// та же логика что в прошлом но наоборот
void Layer::removeFromWeight() {
    weights.removeColumn(weights.getCols() - 1);

    // Сокращаем prevValues до нового размера входов
    prevValues.resize(weights.getCols());
}