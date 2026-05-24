#include "inputfield.h"
#include "utils.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

#define DEBUG

#ifdef DEBUG
#include <QDebug>
#endif

void InputField::buildChoosePanel() {
    chooseLayout->setSizeConstraint(QLayout::SetFixedSize);
    // управляет chooseLayout
    // три кнопки в ряд: < ; (номер семпла) ;  >

    // название (0)
    auto *title = new QLabel("Создание примеров");

    // (1) 1-2
    auto *removeSample = new QPushButton("Удалить");
    auto *createSample = new QPushButton("Создать");

    // (1) 3-4
    auto *decrementBtn = new QPushButton("<");
    label = new QLabel("1");
    auto *incrementBtn = new QPushButton(">");

    title->setContentsMargins(1, 3, 1, 2);
    label->setAlignment(Qt::AlignCenter);
    title->setAlignment(Qt::AlignCenter);

    chooseLayout->addWidget(title, 0, 0, 1, 5);

    // добавление главных элементов
    chooseLayout->addWidget(decrementBtn, 1, 0);
    chooseLayout->addWidget(label, 1, 1);
    chooseLayout->addWidget(incrementBtn, 1, 2);
    chooseLayout->addWidget(removeSample, 1, 4);
    chooseLayout->addWidget(createSample, 1, 3);

    // коннекты (они тут т.к. они чисто внутри этих элементов)

    // remove&create btns -> maxSamples
    connect(removeSample, &QPushButton::clicked,
            this, [&](){
        /*
        должен удалять выбранный семпл
        */

        if (avaibleSamples <= 1) {
            return;
        }

        if (currentSample < 0 || currentSample >= avaibleSamples) {
            return;
        }

        int removed = currentSample;

        if (currentSample == avaibleSamples - 1) {
            currentSample--;
        }

        avaibleSamples--;

        emit removeSampleRequest(removed);

    });
    connect(createSample, &QPushButton::clicked,
            this, [&](){
        currentSample = avaibleSamples++;
        emit createSampleRequest();
    });

    // decrement&increment btns -> currentSample
    connect(decrementBtn, &QPushButton::clicked,
            this, [this](){
        if (currentSample > 0) {
            currentSample--;
            label->setText(QString::number(currentSample + 1));
            updateInputGrid();
        }
    });
    connect(incrementBtn, &QPushButton::clicked,
            this, [this](){
        if (currentSample < avaibleSamples - 1) {
            currentSample++;
            label->setText(QString::number(currentSample + 1));
            updateInputGrid();
        }

    });


}


InputField::InputField(int inputLength,
                       int outputLength,
                       UIDriver* driver, // ! можно через ссылку
                       Samples& ptrSamples,
                       QWidget* parent)
    : QWidget(parent),
    inputSize(inputLength),
    targetSize(outputLength),
    sharedModel(driver->getModel()),
    samples(ptrSamples)
{
    auto *vBox = new QVBoxLayout();


    chooseLayout = new QGridLayout();
    buildChoosePanel();
    vBox->addLayout(chooseLayout);

    inputLayout = new QGridLayout();
    inputLayout->setSizeConstraint(QLayout::SetFixedSize);
    inputLayout->setVerticalSpacing(4);
    inputLayout->setHorizontalSpacing(5);
    inputLayout->setContentsMargins(0, 3, 0, 0);

    // заголовки как часть грида
    auto *inputTitle = new QLabel("Вход");
    auto *targetTitle = new QLabel("Требуется");

    inputLayout->addWidget(inputTitle, 0, 0, Qt::AlignCenter);
    inputLayout->addWidget(targetTitle, 0, 1, Qt::AlignCenter);

    vBox->addLayout(inputLayout);

    setLayout(vBox);
    rebuildInputGrid();
}

void InputField::updateInputGrid() {
    for (int inputI = 0; inputI < inputSpins.size(); inputI++) {
        auto *spin = inputSpins[inputI];
        spin->setValue(samples.inputs[currentSample][inputI]);
    }

    for (int targetI = 0; targetI < targetSpins.size(); targetI++) {
        auto *spin = targetSpins[targetI];
        spin->setValue(samples.targets[currentSample][targetI]);
    }
}

void InputField::rebuildInputGrid() {
    // =============
    // Работа с input спинами
    // =============
    int inputSpinsSize = inputSpins.size();
    int needInput = inputSize - inputSpins.size();

    if (needInput > 0) {
        for (int inputI = 0; inputI < needInput; inputI++) {
            int realIndex = inputI + inputSpinsSize;

            auto *spin = new QDoubleSpinBox();
            spin->setRange(-100.0, 100.0);
            spin->setDecimals(3);
            spin->setSingleStep(0.05);
            inputSpins.push_back(spin);

            connect(spin, &QDoubleSpinBox::valueChanged,
                    this, [this, realIndex](double x) {
                        samples.inputs[currentSample][realIndex] = x;
                    });

            inputLayout->addWidget(spin, realIndex + 1, 0);
        }
    } else if (needInput < 0) {
        for (int inputI = needInput; inputI < 0; inputI++) {
            auto *spin = takeLastSpin(inputSpins);

            inputLayout->removeWidget(spin);
            delete spin;
        }
    }


    // =============
    // Работа с target спинами
    // =============

    int targetSpinsSize = targetSpins.size();
    int needTarget = targetSize - targetSpins.size();

    if (needTarget > 0) {
        for (int targetI = 0; targetI < needTarget; targetI++) {
            int realIndex = targetI + targetSpinsSize;

            auto *spin = new QDoubleSpinBox();
            spin->setRange(-100000.0, 100000.0);
            spin->setDecimals(3);
            spin->setSingleStep(0.05);
            targetSpins.push_back(spin);

            connect(spin, &QDoubleSpinBox::valueChanged,
                    this, [this, realIndex](double x) {
                        samples.targets[currentSample][realIndex] = x;
                    });

            inputLayout->addWidget(spin, realIndex + 1, 1);
        }
    } else if (needTarget < 0) {
        for (int targetI = needTarget; targetI < 0; targetI++) {
            auto *spin = takeLastSpin(targetSpins);

            inputLayout->removeWidget(spin);
            delete spin;
        }
    }
}

void InputField::updateChoosePanel() {
    label->setText(QString::number(currentSample + 1));
}

void InputField::update() {
    updateInputGrid();
    updateChoosePanel();
}

// слоты
void InputField::onSamplesUpdated() {
#ifdef DEBUG
    qDebug() << "[DEBUG] currentSample = " << currentSample << "; avaibleSample = " << avaibleSamples;
    qDebug() << "[DEBUG] onSamplesUpdated: "; samples.debugPrint();
#endif
    inputSize = samples.inputs[0].size();
    targetSize= samples.targets[0].size();
    rebuildInputGrid();
    update();
}

void InputField::onSharedModelUpdated() {
    // Правильно: берём число входов из model.inputValues
    inputSize  = sharedModel.inputValues.size();
    targetSize = sharedModel.nn->getLayers().back().getValues().size();

    // Синхронизация семплов с новой архитектурой
    for (auto& in : samples.inputs)
        in.resize(inputSize, 0.0);
    for (auto& t : samples.targets)
        t.resize(targetSize, 0.0);

    rebuildInputGrid();
    updateInputGrid();
}