
#include <NeuroNetwork.h>
#include <utils.h>

NeuroNetwork::NeuroNetwork(const std::vector<int> conf, ActivationType activationType1) : activationType(activationType1)
{
    for (size_t i = 1; i < conf.size(); i++) {
        layers.emplace_back(conf[i], conf[i - 1], activationType);
    }
    inputSize = conf[0];
}

std::vector<double> NeuroNetwork::forward(const std::vector<double>& input)
{
    std::vector<double> out = input;

    for (auto& layer : layers) {
        out = layer.forward(out);
    }

    return out;
}

double NeuroNetwork::loss(const std::vector<double>& out,
    const std::vector<double>& target)
{
    double l = 0.0;

    for (size_t i = 0; i < out.size(); i++) {
        double d = out[i] - target[i];
        l += d * d;
    }

    return l / out.size();
}

double NeuroNetwork::train(const std::vector<double>& input,
    const std::vector<double>& target,
    double lr)
{
    // forward
    std::vector<double> out = forward(input);

    // loss считаем ДО backprop
    double currentLoss = loss(out, target);

    // initial delta (MSE)
    std::vector<double> delta(out.size());

    for (size_t i = 0; i < out.size(); i++) {
        delta[i] = 2.0 * (out[i] - target[i]);
    }

    // backprop
    for (int i = (int)layers.size() - 1; i >= 0; i--) {
        delta = layers[i].backprop(delta, lr);
    }

    return currentLoss;
}


std::ostream& operator<<(std::ostream& os, const NeuroNetwork& net)
{
    os << "NeuroNetwork:\n";

    for (size_t i = 0; i < net.layers.size(); i++) {
        os << "Layer " << i << ":\n";
        os << net.layers[i] << "\n";
    }

    return os;
}

void NeuroNetwork::trainBatch(
    const std::vector<std::vector<double>>& inputs,
    const std::vector<std::vector<double>>& targets,
    int epochs,
    double lr
)
{
    if (inputs.size() != targets.size())
        throw std::invalid_argument("NeuroNetwork: Inputs and targets size mismatch");

    for (int e = 0; e < epochs; e++) {

        double totalLoss = 0.0;

        for (size_t i = 0; i < inputs.size(); i++) {

            std::vector<double> out = forward(inputs[i]);

            totalLoss += loss(out, targets[i]);

            // один шаг обучения
            train(inputs[i], targets[i], lr);
        }

        totalLoss /= inputs.size();

        std::cout << "Epoch " << std::setw(4) << e
            << " | Loss: " << totalLoss << "\n";
    }
}

void NeuroNetwork::randomizeWeights() {
    for (int i = 0; i < layers.size(); i++) {
        Layer& layer = layers[i];
        layer.randomizeWeights();
    }
}
void NeuroNetwork::resetValues() {
    for (int layerI = 0; layerI < layers.size(); layerI++) {
        Layer& layer = layers[layerI];

        layer.resetValues();
    }
}
void NeuroNetwork::resetBias() {
    for (int i = 0; i < layers.size(); i++) {
        Layer& layer = layers[i];

        layer.resetBias();
    }
}

// +/- commands
void NeuroNetwork::addLayer() {
    if (layers.size() < 1) return;   // нужен хотя бы выходной слой
    const int DEFAULT_HIDDEN_NEURONS  = 3;

    // Сохраняем размер входа для нового слоя (число нейронов предыдущего слоя)
    int prevSize = layers.size() == 1 ? inputSize
                                      : layers[layers.size() - 2].getValues().size();

    // Вставляем новый слой перед выходным
    layers.insert(layers.end() - 1,
                  Layer(DEFAULT_HIDDEN_NEURONS, prevSize, activationType));

    // ПОСЛЕ вставки получаем актуальную ссылку на выходной слой
    Layer& output = layers.back();

    // Удаляем все старые столбцы весов выходного слоя (связи со старым предыдущим слоем)
    int oldCols = output.getWeights().getCols();
    for (int i = 0; i < oldCols; ++i)
        output.removeFromWeight();

    // Добавляем новые столбцы для связи с новым слоем
    for (int i = 0; i < DEFAULT_HIDDEN_NEURONS; ++i)
        output.addFromWeight();
}
void NeuroNetwork::removeLayer() {
    if (layers.size() <= 1) return;   // нельзя удалить единственный слой

    int removedIndex = (int)layers.size() - 2;   // удаляем предпоследний (скрытый)

    // Число нейронов в слое, который станет предыдущим для выходного
    int newPrevSize = (removedIndex > 0)
                          ? layers[removedIndex - 1].getValues().size()
                          : layers[0].getWeights().getCols();   // для входного слоя

    // Удаляем слой
    layers.erase(layers.begin() + removedIndex);

    // Теперь выходной слой – последний
    Layer& output = layers.back();

    // Удаляем старые столбцы весов (они вели к удалённому слою)
    int oldCols = output.getWeights().getCols();
    for (int i = 0; i < oldCols; ++i)
        output.removeFromWeight();

    // Добавляем новые столбцы для связи с новым предыдущим слоем
    for (int i = 0; i < newPrevSize; ++i)
        output.addFromWeight();
}
void NeuroNetwork::addNode(int layerIndex)
{
    if (layerIndex == 0)
    {
        // Добавление входного нейрона
        inputSize++;
        // Если есть хотя бы один слой, добавить ему столбец весов для нового входа
        if (!layers.empty())
            layers.front().addFromWeight();
    }
    else
    {
        // индекс слоя: 1 – первый скрытый, 2 – второй скрытый и т.д.
        // layers.size() соответствует количеству скрытых + выходной слои
        // layerIndex - 1 – индекс в векторе layers.
        if (layerIndex - 1 < 0 || layerIndex - 1 >= (int)layers.size())
            return; // некорректный индекс

        // Добавляем нейрон в выбранный слой
        layers[layerIndex - 1].addNode();

        // Если существует следующий слой, добавляем ему веса от нового нейрона
        if (layerIndex < (int)layers.size())
            layers[layerIndex].addFromWeight();
    }
}

void NeuroNetwork::removeNode(int layerIndex)
{
    if (layerIndex == 0)
    {
        // Удаление входного нейрона (нельзя оставить входной слой без нейронов)
        if (inputSize <= 1)
            return;

        inputSize--;
        // Убираем столбец весов в первом слое, соответствующий удалённому входу
        if (!layers.empty())
            layers.front().removeFromWeight();
    }
    else
    {
        if (layerIndex - 1 < 0 || layerIndex - 1 >= (int)layers.size())
            return;

        // Нельзя удалить последний нейрон в слое
        if (layers[layerIndex - 1].getValues().size() <= 1)
            return;

        // Удаляем нейрон из выбранного слоя
        layers[layerIndex - 1].removeNode();

        // Если есть следующий слой, убираем у него веса от удалённого нейрона
        if (layerIndex < (int)layers.size())
            layers[layerIndex].removeFromWeight();
    }
}

/* Сохраням модель в .txt в виде:
(inputSize)
(activationType)
(layerCount) // без input

(size)       // кол-во нодов в слое
(weights)    // в 1 строчку
(bias)

...

*/

// import/export
void NeuroNetwork::saveModel(std::ostream &to) {
    to << inputSize << " ";
    to << toInt(activationType) << " ";
    to << layers.size() << "\n";


    for (const Layer &layer : layers) {
        to << layer.getValues().size() << "\n";
        to << layer.getWeights().toString() << "\n";
        to << joinVector(layer.getBias()) << "\n";
    }
}
void NeuroNetwork::loadModel(std::istream &from) {
    int a = 0;
    int layerCount = 0;

    // читаем базу
    if (!(from >> inputSize)) return;
    if (!(from >> a)) return;
    if (!(from >> layerCount)) return;

    ActivationType newActivation = fromInt(a);

    std::vector<Layer> newLayers;
    newLayers.reserve(layerCount);

    int prevSize = inputSize;

    for (int i = 0; i < layerCount; i++) {
        int size = 0;

        if (!(from >> size)) return;

        // защита от мусора в файле
        if (size <= 0 || size > 100000) return;

        Matrix m(size, prevSize);

        for (double &x : m.raw()) {
            if (!(from >> x)) return;
        }

        std::vector<double> b(size);
        for (double &x : b) {
            if (!(from >> x)) return;
        }

        newLayers.emplace_back(m, b, newActivation);

        prevSize = size;
    }

    // если дошли сюда — файл валидный
    activationType = newActivation;
    layers = std::move(newLayers);
}



