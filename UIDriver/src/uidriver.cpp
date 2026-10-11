
#include <cmath>
#include <fstream>
#include <QCoreApplication>
#include <ostream>
#include <QDebug>
#include <uidriver.h>
#include <utils.h>

#define DEBUG

UIDriver::UIDriver(QObject *parent) : QObject(parent), nn(DefaultModel.conf, DefaultModel.activationType) {
    model.nn = &nn;
    model.inputValues = std::vector(DefaultModel.conf[0], 0.0);

    // ресайз samples под defaultmodel
    samples.inputs[0].resize(DefaultModel.conf[0]);
    samples.targets[0].resize(DefaultModel.conf.back());

    lr = DefaultModel.lr;
    trainer = new Trainer(samples, model, lr);

    initConnects();
}

// дата менеджемент
void UIDriver::onCreateSampleRequest() {
    samples.inputs.emplace_back(samples.inputs[0].size(), 0.0);
    samples.targets.emplace_back(samples.targets[0].size(), 0.0);

    emit samplesUpdated();
}
void UIDriver::onRemoveSampleRequest(int index) {
    if (index < 0 || index >= samples.inputs.size()) return;
    samples.inputs.erase(samples.inputs.begin()+index);
    samples.targets.erase(samples.targets.begin()+index);
    emit samplesUpdated();
}

// реквесты
void UIDriver::onPredictRequest(std::vector<double> inputData) {
#ifdef DEBUG
    qDebug() << "[DEBUG] onPredictRequest";
#endif
    // чекаем на разрешенность
    if (!checkIsAllowed()) return;
    if (inputData.size() != nn.getLayers()[0].getWeights().getCols()) return;

    QMutexLocker locker(&model.mtx);

    nn.forward(inputData);
    emit sharedModelUpdated();
}

void UIDriver::onTrainRequest() {
    emit trainRequest();
}

void UIDriver::onStopTrainRequest() {
    emit stopTrainRequest();
}


void UIDriver::onImportRequest(std::string pathToFile) {
    qDebug() << "[DEBUG] importRequest to" << pathToFile;

    std::ifstream from(pathToFile);
    if (!from.is_open()) { qDebug() << "[DEBUG] error: can`t open " << pathToFile << " for import"; return; }
    nn.loadModel(from);
}
void UIDriver::onExportRequest(std::string pathToFile) {
    qDebug() << "[DEBUG] exportRequest to" << pathToFile;

    std::ofstream to(pathToFile);
    if (!to.is_open()) { qDebug() << "[DEBUG] error: can`t open" << pathToFile << " for export"; return; }
    nn.saveModel(to);
}

void UIDriver::onLrValueChanged(double val) {
    emit lrChanged(val);
}


void UIDriver::onResetRequest() {
    resetValues();
    randWeigts();
    resetBias();

    trainer->onReset();
}

void UIDriver::resetValues() {
    nn.resetValues();
    emit sharedModelUpdated();
}
void UIDriver::randWeigts() {
    nn.randomizeWeights();
    emit sharedModelUpdated();
}
void UIDriver::resetBias() {
    nn.resetBias();
    emit sharedModelUpdated();
}

void UIDriver::onActivationTypeChanged(ActivationType type) {
    qDebug() << "[DEBUG] onActivationTypeChanged to" << (int)type;
    nn.setActivationType(type);
}

void UIDriver::onAddLayer() {
    nn.addLayer();
    resetValues();
    emit sharedModelUpdated();
}
void UIDriver::onRemoveLayer() {
    nn.removeLayer();
    resetValues();
    emit sharedModelUpdated();
}
void UIDriver::onAddNode(int layerIndex)
{
    nn.addNode(layerIndex);
    // Синхронизируем вектор входов модели с новым размером
    model.inputValues.resize(nn.getInputSize());
    if (layerIndex==0) {
        for (std::vector<double>& vals : samples.inputs) {
            vals.push_back(0.0);
        }
    }
    if (layerIndex==nn.getLayers().size()) {
        for (std::vector<double>& vals : samples.targets) {
            vals.push_back(0.0);
        }
    }
    emit samplesUpdated();
    emit sharedModelUpdated();
}

void UIDriver::onRemoveNode(int layerIndex)
{
    nn.removeNode(layerIndex);
    model.inputValues.resize(nn.getInputSize());
    if (layerIndex==0 && samples.inputs[0].size() > 1) {
        for (std::vector<double>& vals : samples.inputs) {
            vals.pop_back();
        }
    }
    if (layerIndex==nn.getLayers().size()  && samples.targets[0].size() > 1) {
        for (std::vector<double>& vals : samples.targets) {
            vals.pop_back();
        }
    }
    emit samplesUpdated();
    emit sharedModelUpdated();
}

#include <QDebug>

bool UIDriver::checkIsAllowed()
{
    qDebug() << "[DEBUG] "; samples.debugPrint();
    qDebug() << "[checkIsAllowed] start";

    // 1. базовая проверка наличия данных
    if (samples.inputs.empty() || samples.targets.empty())
    {
        qWarning() << "[checkIsAllowed] Empty data:"
                   << "inputs =" << samples.inputs.size()
                   << ", targets =" << samples.targets.size();

        emit wrongInputValues();
        emit wrongTargetValues();
        return false;
    }

    // 2. одинаковое количество входов и таргетов
    if (samples.inputs.size() != samples.targets.size())
    {
        qWarning() << "[checkIsAllowed] Size mismatch:"
                   << "inputs =" << samples.inputs.size()
                   << ", targets =" << samples.targets.size();

        emit wrongInputValues();
        emit wrongTargetValues();
        return false;
    }

    // 3. проверка размерностей входов
    const size_t inputSize = samples.inputs[0].size();
    qDebug() << "[checkIsAllowed] inputSize =" << inputSize;

    if (inputSize == 0)
    {
        qWarning() << "[checkIsAllowed] inputSize is 0";

        emit wrongInputValues();
        return false;
    }

    for (size_t i = 0; i < samples.inputs.size(); ++i)
    {
        const auto& in = samples.inputs[i];

        if (in.size() != inputSize)
        {
            qWarning() << "[checkIsAllowed] Input size mismatch at index" << i
                       << ": expected =" << inputSize
                       << ", got =" << in.size()
                       << "; "
                       << "inputVec=" << vecToString(samples.inputs)
                       << "targetVec=" << vecToString(samples.targets);

            emit wrongInputValues();
            return false;
        }
    }

    // 4. проверка размерностей таргетов
    const size_t targetSize = samples.targets[0].size();
    qDebug() << "[checkIsAllowed] targetSize =" << targetSize;

    if (targetSize == 0)
    {
        qWarning() << "[checkIsAllowed] targetSize is 0";

        emit wrongTargetValues();
        return false;
    }

    for (size_t i = 0; i < samples.targets.size(); ++i)
    {
        const auto& t = samples.targets[i];

        if (t.size() != targetSize)
        {
            qWarning() << "[checkIsAllowed] Target size mismatch at index" << i
                       << ": expected =" << targetSize
                       << ", got =" << t.size();

            emit wrongTargetValues();
            return false;
        }
    }

    // 5. базовая проверка сети
    const auto& layers = nn.getLayers();
    qDebug() << "[checkIsAllowed] layers count =" << layers.size();

    if (layers.empty())
    {
        qWarning() << "[checkIsAllowed] Neural network has no layers";
        return false;
    }

    qDebug() << "[checkIsAllowed] OK";
    return true;
}

void UIDriver::initConnects() {
    // UIDriver -> Trainer
    connect(this, &UIDriver::trainRequest, trainer, [this] {
        trainer->onTrainRequest();
    });
    connect(this, &UIDriver::stopTrainRequest, trainer, [this] {
        trainer->onStopTrain();
    });
    connect(this, &UIDriver::lrChanged, trainer, [this](double lr) {
        trainer->onLrChanged(lr);
    });

    // Trainer -> UIDriver
    connect(trainer, &Trainer::sharedModelUpdated, this, &UIDriver::sharedModelUpdated);
    connect(trainer, &Trainer::epocheFinish, this, &UIDriver::epocheFinish);

}
