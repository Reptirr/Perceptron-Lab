#include <ActivationType.h>
#include <bottompanel.h>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QValueAxis>
#include <QToolButton>

#include "uidriver.h"

QChartView *BottomPanel::buildChart() {
    lossChart = new LossChart();

    return lossChart;
}

void BottomPanel::onReset() {
    lossChart->reset();
}

QHBoxLayout *BottomPanel::buildTop() {
    auto *layout = new QHBoxLayout();

    // start
    auto *startBtn = new QToolButton();
    startBtn->setText("Запуск");

    // pause
    auto *pauseBtn = new QToolButton();
    pauseBtn->setText("Пауза");

    // reset
    auto *resetBtn = new QToolButton();
    resetBtn->setText("Сбросить");

    // set lr
    auto *lrLabel = new QLabel("Введите скорость обучения");
    auto *lrSpin = new QDoubleSpinBox();
    lrSpin->setValue(0.01);

    // set activation
    auto *activationLabel = new QLabel("Выберите тип активации:");
    auto *activationCombo = new QComboBox();

    activationCombo->setCurrentIndex(0);
    activationCombo->addItem("ReLu",        (int)ActivationType::RELU);
    activationCombo->addItem("Sigmoid",     (int)ActivationType::SIGMOID);
    activationCombo->addItem("Leaky ReLu",  (int)ActivationType::LEAKYRELU);
    activationCombo->addItem("Step",        (int)ActivationType::STEP);

    layout->addWidget(startBtn);
    layout->addWidget(pauseBtn);
    layout->addWidget(resetBtn);

    layout->addStretch();

    layout->addWidget(lrLabel);
    layout->addWidget(lrSpin);

    layout->addWidget(activationLabel);
    layout->addWidget(activationCombo);

    // init connects
    connect(startBtn, &QToolButton::clicked,
            this, &BottomPanel::startTrainRequest);
    connect(startBtn, &QToolButton::clicked,
            this, [startBtn, pauseBtn, this] {
                training = true;

                startBtn->setEnabled(false);
                pauseBtn->setEnabled(true);
            });
    connect(pauseBtn, &QToolButton::clicked,
            this, &BottomPanel::stopTrainRequest);
    connect(pauseBtn, &QToolButton::clicked,
            this, [startBtn, pauseBtn, this] {
                training = false;

                startBtn->setEnabled(true);
                pauseBtn->setEnabled(false);
            });
    connect(resetBtn, &QToolButton::clicked,
            this, &BottomPanel::resetRequest);
    connect(resetBtn, &QToolButton::clicked,
            this, [pauseBtn, this] {
                if (training) pauseBtn->click();

                onReset();
            });
    connect(lrSpin, &QDoubleSpinBox::valueChanged,
            this, &BottomPanel::lrChanged);
    connect(activationCombo, &QComboBox::currentIndexChanged,
            this, [this, activationCombo](int index) {
                emit activationChanged(
                    static_cast<ActivationType>(activationCombo->itemData(index).toInt())
                );
            });

    return layout;
}
void BottomPanel::onNewErrorPoint(int epoch, double err) {
    lossChart->addPoint(epoch, err);
}

BottomPanel::BottomPanel(QWidget *parent) : QWidget(parent) {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    layout->addLayout(buildTop());
    layout->addWidget(buildChart());
}
