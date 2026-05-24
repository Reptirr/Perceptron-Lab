#ifndef RIGHTPANEL_H
#define RIGHTPANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>

#include "networkgraph.h"

class RightPanel : public QWidget
{
    Q_OBJECT
public:
    explicit RightPanel(UIDriver *d, QWidget *parent = nullptr);

    int selectedLayer = -1;

    QPushButton *incrementNode;
    QPushButton *decrementNode;

public slots:
    void onSelectedLayerChanged();
};

#endif // RIGHTPANEL_H
