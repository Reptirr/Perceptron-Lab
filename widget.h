#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>


#include "controlpanel.h"
#include "rightpanel.h"
#include "uidriver.h"
#include "inputfield.h"


class Widget : public QWidget
{
    Q_OBJECT

private:
    ControlPanel *controlPanel;

    InputField *inputField;
    RightPanel *rightPanel;
    UIDriver *uiDriver;

    void initializeConnects();


public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget() override;

};

#endif // WIDGET_H
