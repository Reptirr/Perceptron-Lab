
#include <QSplitter>
#include <QHBoxLayout>
#include <QDebug>
#include <widget.h>

void Widget::initializeConnects() {
    // ControlPanel -> UIDriver
    // сигналы от кнопок
    connect(controlPanel->controlPanel(), &ControlPanelPrivate::predictRequest, uiDriver, &UIDriver::onPredictRequest);
    connect(controlPanel->controlPanel(), &ControlPanelPrivate::trainClicked, uiDriver, &UIDriver::onTrainRequest);
    connect(controlPanel->controlPanel(), &ControlPanelPrivate::resetClicked, uiDriver, &UIDriver::onResetRequest);
    // сигналы о изменении значения
    connect(controlPanel->controlPanel(), &ControlPanelPrivate::epochValueChanged, uiDriver, &UIDriver::onEpochValueChanged);
    connect(controlPanel->controlPanel(), &ControlPanelPrivate::lrValueChanged, uiDriver, &UIDriver::onLrValueChanged);
    connect(controlPanel->controlPanel(), &ControlPanelPrivate::activationTypeChanged, uiDriver, &UIDriver::onActivationTypeChanged);

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
    bottomPanel = new BottomPanel(this);

    // ресайз
    resize(1000, 700);

    auto *generalLayout = new QVBoxLayout(this);
    generalLayout->setContentsMargins(0, 0, 0, 0);
    generalLayout->setSpacing(0);

    // ----------
    // UP
    // ----------
    auto *upLayout = new QHBoxLayout(this);

    controlPanel = new ControlPanel(inputField, uiDriver, this);
    auto *split = new QWidget();
    rightPanel = new RightPanel(uiDriver, this);

    controlPanel->controlPanel()->setAutoFillBackground(true);
    QPalette pal = controlPanel->controlPanel()->palette();
    pal.setColor(QPalette::Window, QColor("#232323"));
    controlPanel->controlPanel()->setPalette(pal);
    split->setStyleSheet("background-color: #2C2C2C; border: none");

    controlPanel->controlPanel()->setFixedWidth(300);
    controlPanel->setFixedWidth(300);
    split->setFixedWidth(4);

    upLayout->setSpacing(0);
    upLayout->setContentsMargins(0, 0, 0, 0);
    controlPanel->controlPanel()->setContentsMargins(0, 0, 0, 0);
    rightPanel->setContentsMargins(0, 0, 0, 0);
    split->setContentsMargins(0, 0, 0, 0);

    upLayout->addWidget(controlPanel);
    upLayout->addWidget(split);
    upLayout->addWidget(rightPanel);

    // -----------
    // BOTTOM
    // -----------
    auto *bottomLayout = new QHBoxLayout();
    bottomLayout->setContentsMargins(0, 0, 0, 0);
    bottomLayout->setSpacing(0);

    // auto *split1 = new QSplitter(this);
    // split1->setFixedHeight(4);
    // split1->setStyleSheet("background-color: #2C2C2C; border: none");
    //
    // bottomPanel->setAutoFillBackground(true);
    // bottomPanel->setPalette(pal);
    // bottomLayout->addWidget(bottomPanel);

    generalLayout->addLayout(upLayout);
    // generalLayout->addWidget(split1);
    generalLayout->addLayout(bottomLayout);

    // коннектим
    initializeConnects();
}

Widget::~Widget() = default;
