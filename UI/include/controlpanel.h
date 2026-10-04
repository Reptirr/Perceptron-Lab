#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H
#include <inputfield.h>
#include <predictinputfield.h>
#include <QScrollArea>
#include <QScrollBar>
#include <QWidget>

class ControlPanelPrivate : public QWidget
{
    Q_OBJECT

    QWidget* createTopWidget(SharedModel &model, UIDriver *uiDriver);
    QWidget* createBottomWidget();

    PredictInputField *predictInput;
    InputField *inputField;

    std::vector<double> inputValues;
    int epochs = 0;

public:
    ControlPanelPrivate(InputField *InputField, UIDriver *uiDriver, QWidget *parent = nullptr);

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



class ControlPanel : public QScrollArea {
    ControlPanelPrivate *content{};

public:
    explicit ControlPanel(InputField *InputField, UIDriver *uiDriver, QWidget *parent = nullptr) :
    QScrollArea(parent),
    content(new ControlPanelPrivate(InputField, uiDriver, this)) {
        setWidget(content);

        setWidgetResizable(true);
        setFrameShape(NoFrame);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        verticalScrollBar()->setAutoFillBackground(false);
        verticalScrollBar()->setAttribute(Qt::WA_TranslucentBackground);
    }

    ControlPanelPrivate *controlPanel() {
        return content;
    }
};


#endif // CONTROLPANEL_H
