#pragma once

#include <QLineSeries>
#include <QChartView>
#include <QGridLayout>
#include <ActivationType.h>
#include <LossChart.h>
#include <QValueAxis>

#include "uidriver.h"

/*
 *                     top
 *                      |
 * +-------------------------------------------+
 * | Запуск Пауза Сброс  lr: [0.1] act: [relu] |
 * +-------------------------------------------+
 * |            ГРАФИК                         |
 * |                                           |
 * +-------------------------------------------+
 *                      |
 *                  chartView
 *
 */

class BottomPanel : public QWidget {
    Q_OBJECT

    LossChart *lossChart{};
    bool training{};

    QHBoxLayout *buildTop();
    QChartView *buildChart();

    void onReset();

signals:
    void startTrainRequest();
    void stopTrainRequest();
    void resetRequest();
    void lrChanged(double lr);
    void activationChanged(ActivationType type);

public slots:
    void onNewErrorPoint(int epoch, double err);

public:
    BottomPanel(QWidget *parent = nullptr);
};