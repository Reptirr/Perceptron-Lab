#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QWidget>
#include <filesystem>

#include "inputfield.h"
#include "predictinputfield.h"

class ControlPanel : public QWidget
{
    Q_OBJECT
private:
    QWidget* createTopWidget(SharedModel &model, UIDriver *uiDriver);
    QWidget* createBottomWidget();

    PredictInputField *predictInput;
    InputField *inputField;

    std::vector<double> inputValues;
    int epochs = 0;

public:
    ControlPanel(InputField *InputField, UIDriver *uiDriver, QWidget *parent = nullptr);

signals:
    // сигналы от кнопок
    void predictRequest(std::vector<double> inputData);
    void resetClicked();
    void trainClicked();
    void randClicked();

    // сигналы о изменении значения
    void epochValueChanged(int newValue);
    void lrValueChanged(double newValue);
    void activationTypeChanged(ActivationType type);
};

#endif // CONTROLPANEL_H
