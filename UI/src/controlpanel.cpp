
#include <controlpanel.h>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QComboBox>
#include <QString>
#include <uidriver.h>

#define DEBUG
#ifdef DEBUG
#include <QDebug>
#endif


// верхний виджет
QWidget* ControlPanelPrivate::createTopWidget(SharedModel &model, UIDriver *uiDriver) {
    auto *grid = new QGridLayout();

    // top widget
    // кнопка прохода (0)
    auto *predictButton = new QPushButton("Проход");

    // ввод данных для прохода (1)
    predictInput = new PredictInputField(model, inputValues);

    // расположения по лайауту
    grid->addWidget(predictButton, 0, 0, 1, 2);
    grid->addWidget(predictInput, 1, 0, 1, 2);
    // коннекты кликов к публичным сигналам ControlPanel
    connect(predictButton, &QPushButton::clicked,
            this, [&](){
        predictRequest(inputValues);
    });

    // сборка виджета
    auto *widget = new QWidget();
    widget->setLayout(grid);

    return widget;
}

class MyDoubleSpinBox : public QDoubleSpinBox {
public:
    using QDoubleSpinBox::QDoubleSpinBox;

    QString textFromValue(double value) const override {
        QString s = locale().toString(value, 'f', decimals());
        const QString dp = locale().decimalPoint();
        if (s.contains(dp)) {
            while (s.endsWith('0'))
                s.chop(1);
            if (s.endsWith(dp))
                s.chop(dp.size());
        }
        return s;
    }
};

QWidget* ControlPanelPrivate::createBottomWidget() {
    auto *grid = new QGridLayout();
    grid->setAlignment(Qt::AlignTop);


    auto *gridWidget = new QWidget();
    gridWidget->setLayout(grid);
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




ControlPanelPrivate::ControlPanelPrivate(InputField *inputF, UIDriver *uiDriver, QWidget *parent) : QWidget(parent), inputField(inputF) {
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


