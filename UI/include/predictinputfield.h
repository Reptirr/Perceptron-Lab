#ifndef PREDICTINPUTFIELD_H
#define PREDICTINPUTFIELD_H

#include <model.h>
#include <QWidget>
#include <QGridLayout>
#include <QDoubleSpinBox>
#include <QLabel>


// выбор данных для предиктов
class PredictInputField : public QWidget
{
    Q_OBJECT
private:
    int inputSize;
    int predictSize;
    SharedModel& model;

    std::vector<QLabel*> inputLabels;
    std::vector<QDoubleSpinBox*> spins;
    std::vector<QLabel*> predictLabels;

    // все управление этим у predictinputfield
    std::vector<double>& inputValues; // думаю надо истинну у этого класса, но все таки родитель главней

    QGridLayout *grid;

    void rebuildGrid();
    void updatePredictLabels(const std::vector<double>& prediction);
public:
    PredictInputField(SharedModel& m, std::vector<double> &inputVals);

public slots:
    void onSharedModelUpdated(); // для изменения inputSize
};

#endif // PREDICTINPUTFIELD_H
