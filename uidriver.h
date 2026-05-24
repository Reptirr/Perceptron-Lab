#ifndef UIDRIVER_H
#define UIDRIVER_H

#include <vector>
#include <QObject>
#include <filesystem>

#include "NeuroNetwork.h"
#include "model.h"
#include "defaultmodel.h"
#include "samples.h"

/*
посредник между ui и perceptron
главная цель - проверка данных
*/
class UIDriver : public QObject
{
    Q_OBJECT
private:
    int epochValue = DefaultModel.epochs;
    double lr = DefaultModel.lr;

    NeuroNetwork nn;
    SharedModel model; // 1 раз создали и изменяем ее состояние, и отдаем в начале программы ссылку
    Samples samples; // даем inputfield и читаем при train

    bool checkIsAllowed();

    void resetValues();
    void randWeigts();
    void resetBias();

public:
    UIDriver(QObject *parent);

    SharedModel& getModel() { return model; }
    Samples& getSamples() { return samples; }

public slots:
    // сигналы от кнопок (реквесты)
    void onPredictRequest(std::vector<double> inputData);
    void onTrainRequest();
    void onResetRequest();
    void onImportRequest(std::string pathToFile);
    void onExportRequest(std::string pathToFile);

    // сигналы о изменении значения
    void onEpochValueChanged(int newValue);
    void onLrValueChanged(double val);
    void onActivationTypeChanged(ActivationType type);

    void onAddLayer();
    void onRemoveLayer();
    void onAddNode(int layerIndex); // включая input
    void onRemoveNode(int layerIndex);


    // дата менеджемент
    void onCreateSampleRequest();
    void onRemoveSampleRequest(int index);


signals:
    void sharedModelUpdated(); // обновление модели после train/predict/editing
    void samplesUpdated(); // обновление модели только для inputfield

    // сигналы об ошибках
    void wrongInputValues();
    void wrongTargetValues();
};

#endif // UIDRIVER_H