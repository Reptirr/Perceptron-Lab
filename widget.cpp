#include "widget.h"
#include <QSplitter>
#include <QHBoxLayout>
#include <QDebug>

void Widget::initializeConnects() {
    // ControlPanel -> UIDriver
    // сигналы от кнопок
    connect(controlPanel, &ControlPanel::predictRequest, uiDriver, &UIDriver::onPredictRequest);
    connect(controlPanel, &ControlPanel::trainClicked, uiDriver, &UIDriver::onTrainRequest);
    connect(controlPanel, &ControlPanel::resetClicked, uiDriver, &UIDriver::onResetRequest);
    // сигналы о изменении значения
    connect(controlPanel, &ControlPanel::epochValueChanged, uiDriver, &UIDriver::onEpochValueChanged);
    connect(controlPanel, &ControlPanel::lrValueChanged, uiDriver, &UIDriver::onLrValueChanged);
    connect(controlPanel, &ControlPanel::activationTypeChanged, uiDriver, &UIDriver::onActivationTypeChanged);

    // UIDriver <-> InputField
    // реквесты
    connect(inputField, &InputField::createSampleRequest, uiDriver, &UIDriver::onCreateSampleRequest);
    connect(inputField, &InputField::removeSampleRequest, uiDriver, &UIDriver::onRemoveSampleRequest);
    // сигналы о изменении значения
    connect(uiDriver, &UIDriver::samplesUpdated, inputField, &InputField::onSamplesUpdated);

    // UIDriver -> this
    connect(uiDriver, &UIDriver::wrongInputValues, this, [&](){ qDebug() << "[DEBUG] wrongInputValues"; });
    connect(uiDriver, &UIDriver::wrongTargetValues, this, [&](){ qDebug() << "[DEBUG] wrongTargetValues"; });    
}

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    // создаем компоненты
    uiDriver = new UIDriver(this);
    inputField = new InputField(DefaultModel.conf[0], DefaultModel.conf.back(), uiDriver, uiDriver->getSamples(), this);

    // ресайз
    resize(1000, 700);

    // место для главного лайаута
    auto *layout = new QHBoxLayout(this);

    controlPanel = new ControlPanel(inputField, uiDriver, this);
    auto *split = new QWidget();
    rightPanel = new RightPanel(uiDriver, this);

    controlPanel->setAutoFillBackground(true);
    QPalette pal = controlPanel->palette();
    pal.setColor(QPalette::Window, QColor("#232323"));
    controlPanel->setPalette(pal);
    split->setStyleSheet("background-color: #2C2C2C; border: none");

    controlPanel->setFixedWidth(300);
    split->setFixedWidth(4);

    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    controlPanel->setContentsMargins(0, 0, 0, 0);
    rightPanel->setContentsMargins(0, 0, 0, 0);
    split->setContentsMargins(0, 0, 0, 0);

    layout->addWidget(controlPanel);
    layout->addWidget(split);
    layout->addWidget(rightPanel);

    setLayout(layout);

    // коннектим
    initializeConnects();
}

Widget::~Widget() = default;
