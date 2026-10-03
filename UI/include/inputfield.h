#pragma once

#include <model.h>
#include <QWidget>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <vector>
#include <QLabel>
#include <samples.h>
#include <uidriver.h>

class InputField : public QWidget
{
    Q_OBJECT

private:
    int inputSize = 0;
    int targetSize = 0;

    int currentSample = 0;
    int avaibleSamples = 1;

    QGridLayout* chooseLayout = nullptr;
    QGridLayout* inputLayout = nullptr;

    // векторы спинов только для правильного ребилда
    std::vector<QDoubleSpinBox*> inputSpins;
    std::vector<QDoubleSpinBox*> targetSpins;

    QLabel *label; // для доступа из разных участков

    Samples &samples;
    SharedModel &sharedModel;

    void buildChoosePanel();
    void rebuildInputGrid(); // постройка ввода данных инпута, таргета

    void update(); // обертка над функ ниже
    void updateInputGrid(); // апдейт значений спинов
    void updateChoosePanel();



public:
    InputField(int inputLength,
               int outputLength,
               UIDriver* driver,
               Samples &ptrSamples,
               QWidget* parent = nullptr);

public slots:
    void onSharedModelUpdated();
    void onSamplesUpdated();

signals:
    void createSampleRequest();
    void removeSampleRequest(int index);

};
