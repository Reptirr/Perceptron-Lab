#pragma once
#include <model.h>
#include <QObject>
#include <samples.h>
#include <thread>


class Trainer : public QObject {
    Q_OBJECT

    Samples &samples;
    SharedModel &model;
    double lr{};

    int e{};
    bool need_stop{};
    std::thread trainThread{};

    std::chrono::milliseconds need_interval{300};

    void infiniteTrain();

signals:
    void sharedModelUpdated();
    void epocheFinish(int epoch, double err);

public slots:
    void onTrainRequest();
    void onStopTrain();
    void onLrChanged(double lr);
    void onReset();

public:
    Trainer(Samples &samples, SharedModel &shared_model, double lr);
};
