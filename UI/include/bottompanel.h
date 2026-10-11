#pragma once

#include <QLineSeries>
#include <QChartView>
#include <QGridLayout>

class BottomPanel : public QWidget {
    Q_OBJECT

    QChart *chart{};
    QLineSeries *series{};

    QChartView *buildChart();

    QVBoxLayout *buildInputPanel();

    signals:
        void startTrainRequest();
    void stopTrainRequest();

public slots:
    void onNewErrorPoint(int epoch_i, double err);

public:
    BottomPanel(QWidget *parent = nullptr);
};