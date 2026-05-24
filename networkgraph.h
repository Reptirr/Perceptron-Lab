#ifndef NETWORKGRAPH_H
#define NETWORKGRAPH_H

#include <QWidget>
#include <QPainter>
#include <QtGlobal>
#include <QMutexLocker>
#include <algorithm>
#include <vector>
#include <QMouseEvent>
#include <cmath>
#include <QRectF>
#include <QHBoxLayout>
#include <QLabel>

#include "model.h"
#include "uidriver.h"

class NetworkGraph : public QWidget
{
    Q_OBJECT
public:
    explicit NetworkGraph(UIDriver *d, int &selectedL, QWidget *parent = nullptr);

private:
    SharedModel &model;

    int &selectedLayer;
    int hoveredLayer = -1;

    int layerAtPos(const QPointF& pos,
                                 int layers,
                                 double marginX,
                                 double marginY,
                                 double layerStepX,
                                 double usedHeight,
                                 int objectSize);

signals:
    void selectedLayerChanged();

public slots:
    void onSharedModelUpdated();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

};

#endif // NETWORKGRAPH_H
