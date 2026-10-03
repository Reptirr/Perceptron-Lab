
#include <controlpanel.h>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QComboBox>
#include <QString>
#include <QDoubleSpinBox>
#include <QFrame>
#include <uidriver.h>

#define DEBUG
#ifdef DEBUG
#include <QDebug>
#endif


// верхний виджет
QWidget* ControlPanel::createTopWidget(SharedModel &model, UIDriver *uiDriver) {
    auto *grid = new QGridLayout();

    // top widget
    // кнопка прохода (0)
    auto *predictButton = new QPushButton("Проход");

    // ввод данных для прохода (1)
    predictInput = new PredictInputField(model, inputValues);

    // кнопки сброса/рандома (2)
    auto *resetButton = new QPushButton("Сброс нейросети");

    // кнопки импорта/экспорта (3) (удалено)
    // auto *importButton = new QPushButton("Импорт");
    // auto *exportButton = new QPushButton("Экспорт");


    // расположения по лайауту
    grid->addWidget(predictButton, 0, 0, 1, 2);
    grid->addWidget(predictInput, 1, 0, 1, 2);
    grid->addWidget(resetButton, 2, 0, 1, 2);
    // grid->addWidget(importButton, 3, 0);
    // grid->addWidget(exportButton, 3, 1);

    // коннекты кликов к публичным сигналам ControlPanel
    connect(predictButton, &QPushButton::clicked,
            this, [&](){
        predictRequest(inputValues);
    });

    connect(resetButton, &QPushButton::clicked,
            this, &ControlPanel::resetClicked);

    // connect(importButton, &QPushButton::clicked,
    //         this, [&](){
    //     QString path = QFileDialog::getOpenFileName(
    //         this,
    //         "Выбрать файл для импорта",
    //         "",
    //         "Text files (*.txt)");

    //     if (path.isEmpty()) return;

    //     uiDriver->onImportRequest(path.toStdString());
    // });
    // connect(exportButton, &QPushButton::clicked,
    //         this, [&](){
    //     QString path = QFileDialog::getSaveFileName(
    //         this,
    //         "Выбрать файл для импорта",
    //         "",
    //         "Text files (*.txt)");

    //     if (path.isEmpty()) return;

    //     uiDriver->onExportRequest(path.toStdString());
    // });

    // сборка виджета
    auto *widget = new QWidget();
    widget->setLayout(grid);
    // widget->setFixedHeight(150);

    return widget;
}

QWidget* ControlPanel::createBottomWidget() {
    auto *grid = new QGridLayout();
    grid->setAlignment(Qt::AlignTop);

    // bottom widget
    // тренировка (0)
    auto *trainBtn = new QPushButton("Тренировать");

    // пустое пространство (1)
    auto *space = new QWidget();

    // параметр кол-ва эпох (2)
    auto *epochLabel = new QLabel("Введите кол-во эпох");
    auto *epochInput = new QSpinBox();

    // Параметр lr (3)
    auto *lrLabel = new QLabel("Введите скорость обучения: ");
    auto *lrInput = new QDoubleSpinBox();

    // Выбор функции активации (4)
    auto *activationLabel = new QLabel("Выберите тип активации:");
    auto *activationCombo = new QComboBox();

    activationCombo->setCurrentIndex(0);
    lrInput->setValue(0.01);
    activationCombo->addItem("ReLu",        (int)ActivationType::RELU);
    activationCombo->addItem("Sigmoid",     (int)ActivationType::SIGMOID);
    activationCombo->addItem("Leaky ReLu",  (int)ActivationType::LEAKYRELU);
    activationCombo->addItem("Step",        (int)ActivationType::STEP);


    // пара параметров
    space->setFixedHeight(10);
    trainBtn->setFixedHeight(30);
    epochInput->setRange(1, 1000000);
    epochInput->setValue(500);
    epochInput->setButtonSymbols(QAbstractSpinBox::NoButtons);

    // ввод в лайаут
    grid->addWidget(trainBtn, 0, 0, 1, 2);
    grid->addWidget(space, 1, 0, 1, 2);
    grid->addWidget(epochLabel, 2, 0);
    grid->addWidget(epochInput, 2, 1);
    grid->addWidget(lrLabel, 3, 0);
    grid->addWidget(lrInput, 3, 1);
    grid->addWidget(activationLabel, 4, 0);
    grid->addWidget(activationCombo, 4, 1);


    // коннекты кликов кнопок к публичным сигналам ControlPanel
    connect(trainBtn, &QPushButton::clicked,
            this, &ControlPanel::trainClicked);
    connect(epochInput, &QSpinBox::valueChanged,
            this, &ControlPanel::epochValueChanged);
    connect(lrInput, &QDoubleSpinBox::valueChanged,
            this, &ControlPanel::lrValueChanged);
    connect(activationCombo, &QComboBox::currentIndexChanged,
            this, [this, activationCombo](int index){
                emit activationTypeChanged(
                    static_cast<ActivationType>(activationCombo->itemData(index).toInt())
                    );
    });


#ifdef DEBUG
    connect(this, &ControlPanel::trainClicked,
            this, [] { qDebug() << "[DEBUG] trainClicked"; });
    connect(this, &ControlPanel::epochValueChanged,
            this, [](int x) { qDebug() << "[DEBUG] epochValueChanged to " << x; });
#endif


    auto *gridWidget = new QWidget();
    gridWidget->setLayout(grid);
    // gridWidget->setFixedHeight();
    gridWidget->setContentsMargins(0, 0, 0, 0);

    auto *vBox = new QVBoxLayout();
    vBox->setContentsMargins(0, 0, 0, 0);
    vBox->setSpacing(1);

    vBox->addWidget(gridWidget);
    vBox->addWidget(inputField);
    vBox->addStretch();


    auto *widget = new QWidget();
    widget->setLayout(vBox);
    return widget;
}




ControlPanel::ControlPanel(InputField *inputF, UIDriver *uiDriver, QWidget *parent) : QWidget(parent), inputField(inputF) {
    // левый виджет - панель с параметрами
    // сверху кнопки прохода/сброса/рандом/сохранить/загрузить
    // снизу поле тренировок, кнопка тренировки, ввод эпох и начальных значений x, y

    auto *layout = new QVBoxLayout();

    // палка-разделитель
    auto *split = new QWidget();
    split->setFixedHeight(4);
    split->setStyleSheet("background-color: #2C2C2C; border: none");
    split->setContentsMargins(0, 0, 0, 0);

    QWidget *top = createTopWidget(uiDriver->getModel(), uiDriver);
    QWidget *bottom = createBottomWidget();

    layout->setContentsMargins(0, 0, 0, 0);

    layout->addWidget(top);
    layout->addWidget(split);
    layout->addWidget(bottom);

    layout->setSpacing(0);
    setLayout(layout);

    // коннект
    connect(uiDriver, &UIDriver::sharedModelUpdated, predictInput, &PredictInputField::onSharedModelUpdated);

}


