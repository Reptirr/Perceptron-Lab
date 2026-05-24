#include <QFrame>

#include "predictinputfield.h"
#include "utils.h"

PredictInputField::PredictInputField(SharedModel& m, std::vector<double> &inputVals) :
    model(m),
    inputSize(m.inputValues.size()),
    predictSize(m.nn->getLayers().back().getValues().size()),
    inputValues(inputVals)
{
    inputValues.assign(inputSize, 0);

    auto *block = new QFrame(this);
    block->setObjectName("predictInput");
    block->setWindowOpacity(1.0);
    block->setAttribute(Qt::WA_StyledBackground, true);

    grid = new QGridLayout(block);
    grid->setContentsMargins(5, 5, 5, 5);

    auto *outer = new QVBoxLayout(this);
    outer->addWidget(block);
    setLayout(outer);
    rebuildGrid();
}

void PredictInputField::rebuildGrid() {
    // такая же логика как в спинах семплов из inputfield
    int needInputSpins = inputSize - spins.size();
    int spinsSize = spins.size();

    if (needInputSpins > 0) {
        for (int i = 0; i < needInputSpins; i++) {
            int realIndex = spinsSize+i;
            // колонки 1-2 (ввод)
            auto *newSpin = new QDoubleSpinBox();
            auto *inputLabel = new QLabel("Вход " + QString::number(realIndex + 1) + ": ");

            // пара параметров
            newSpin->setRange(-1000, 1000);
            newSpin->setSingleStep(0.05);
            newSpin->setDecimals(3);

            // добавление label в ui
            grid->addWidget(inputLabel, realIndex, 0);

            // добавление спина в ui
            grid->addWidget(newSpin, realIndex, 1);

            // создание коннекта на inputValues
            connect(newSpin, &QDoubleSpinBox::valueChanged,
                    this, [this, realIndex](double x){
                inputValues[realIndex] = x;
            });

            // добавление в вектор
            inputLabels.push_back(inputLabel);
            spins.push_back(newSpin);
        }
    } else if (needInputSpins < 0) {
        for (int i = 0; i < std::abs(needInputSpins); i++) {
            auto *removedSpin = takeLastSpin(spins);
            auto *removedLabel = takeLastLabel(inputLabels);

            grid->removeWidget(removedLabel);
            grid->removeWidget(removedSpin);

            delete removedLabel;
            delete removedSpin;
        }
    }

    int needPredictLabels = predictSize - predictLabels.size();
    int labelsSize = predictLabels.size();

    // тут логика для предикт лабелов
    if (needPredictLabels > 0) {
        for (int i = 0; i < needPredictLabels; i++) {

            int realIndex = labelsSize + i;

            auto *label = new QLabel("Выход " + QString::number(i+1) + ": " +  QString::number(0, 'f', 3));

            label->setAlignment(Qt::AlignCenter);

            grid->addWidget(label, realIndex, 2, 1, 2);

            predictLabels.push_back(label);
        }
    } else if (needPredictLabels < 0) {
        for (int i = 0; i < std::abs(needPredictLabels); i++) {

            auto *label = takeLastLabel(predictLabels);

            grid->removeWidget(label);
            delete label;
        }
    }
}

void PredictInputField::updatePredictLabels(const std::vector<double>& prediction)
{
    int n = std::min((int)predictLabels.size(), (int)prediction.size());

    for (int i = 0; i < n; i++) {
        predictLabels[i]->setText("Выход " + QString::number(i+1) + ": " +  QString::number(prediction[i], 'f', 3));
    }
}

// слоты
void PredictInputField::onSharedModelUpdated() {
    inputSize = model.inputValues.size();
    predictSize = model.nn->getLayers().back().getValues().size();

    inputValues.resize(inputSize);

    rebuildGrid();
    updatePredictLabels(model.nn->getLayers().back().getValues());
}