#include <ActivationType.h>
#include <bottompanel.h>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QValueAxis>
#include <QToolButton>

QChartView *BottomPanel::buildChart() {
    series = new QLineSeries();

    // y = x*x
    for (int x = 0; x < 100; x++)
        series->append(x, x*x);

    chart = new QChart();
    chart->legend()->hide();
    chart->setMargins(QMargins(0, 0, 0, 0));
    chart->setContentsMargins(0, 0, 0, 0);
    chart->addSeries(series);

    auto *axis_x = new QValueAxis();
    axis_x->setTitleText("Координата X");
    auto *axis_y = new QValueAxis();
    axis_y->setTitleText("Координата Y");

    chart->addAxis(axis_x, Qt::AlignBottom);
    chart->addAxis(axis_y, Qt::AlignLeft);
    series->attachAxis(axis_x);
    series->attachAxis(axis_y);

    axis_x->applyNiceNumbers();
    axis_y->applyNiceNumbers();

    auto *view = new QChartView(chart);
    view->setRenderHints(QPainter::Antialiasing);

    return view;
}

QVBoxLayout *BottomPanel::buildInputPanel() {
    auto *grid = new QGridLayout();

    // start
    auto *startBtn = new QToolButton();
    startBtn->setText("Запустить обучение");

    // stop
    auto *stopBtn = new QToolButton();
    stopBtn->setText("Остановить обучение");

    // set lr
    auto *lrLabel = new QLabel("Введите скорость обучения");
    auto *lrSpin = new QDoubleSpinBox();
    lrSpin->setValue(0.1);

    // set activation
    auto *activationLabel = new QLabel("Выберите тип активации:");
    auto *activationCombo = new QComboBox();

    activationCombo->setCurrentIndex(0);
    activationCombo->addItem("ReLu",        (int)ActivationType::RELU);
    activationCombo->addItem("Sigmoid",     (int)ActivationType::SIGMOID);
    activationCombo->addItem("Leaky ReLu",  (int)ActivationType::LEAKYRELU);
    activationCombo->addItem("Step",        (int)ActivationType::STEP);

    grid->addWidget(startBtn, 0, 0);
    grid->addWidget(stopBtn, 0, 1);

    grid->addWidget(lrLabel, 1, 0);
    grid->addWidget(lrSpin, 1, 1);

    grid->addWidget(activationLabel, 2, 0);
    grid->addWidget(activationCombo, 2, 1);



    auto *widget = new QWidget(this);
    widget->setLayout(grid);
    widget->setFixedWidth(300);

    auto *layout = new QVBoxLayout();
    layout->addWidget(widget);
    layout->addStretch();

    return layout;
}

void BottomPanel::onNewErrorPoint(int epoch_i, double err) {
    series->append(epoch_i, err);
}

BottomPanel::BottomPanel(QWidget *parent) : QWidget(parent) {
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    layout->addLayout(buildInputPanel());
    layout->addWidget(buildChart());
}