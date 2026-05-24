#include "networkgraph.h"

#define DEBUG

#ifdef DEBUG
#include <QDebug>
#endif

NetworkGraph::NetworkGraph(UIDriver *d, int &selectedL, QWidget *parent)
    : QWidget(parent), model(d->getModel()), selectedLayer(selectedL)
{
    setMouseTracking(true);
}

// slots
void NetworkGraph::onSharedModelUpdated() {
#ifdef DEBUG
    qDebug() << "[DEBUG] onSharedModelUpdated";
#endif
    if (selectedLayer > model.nn->getLayers().size()+1) selectedLayer--;
    update();
}

int NetworkGraph::layerAtPos(const QPointF &pos,
                             int layers,
                             double marginX,
                             double marginY,
                             double layerStepX,
                             double usedHeight,
                             int objectSize)
{
    for (int i = 0; i < layers; ++i) {
        double left = marginX + layerStepX * i;
        QRectF columnRect(left, marginY, objectSize, usedHeight);
        if (columnRect.contains(pos))
            return i;
    }
    return -1;
}

void NetworkGraph::mouseMoveEvent(QMouseEvent *event)
{
    QPointF pos = event->pos();
    QMutexLocker locker(&model.mtx);

    int w = width();
    int h = height();

    const auto& layersStd = model.nn->getLayers();

    // ---------- расчёт геометрии (точная копия paintEvent) ----------
    int maxNodes = 1;
    maxNodes = std::max(maxNodes, (int)model.inputValues.size());
    for (const Layer& l : layersStd)
        maxNodes = std::max(maxNodes, (int)l.getValues().size());

    int layers = std::max(1, (int)layersStd.size() + 1);
    int minMargin = 80;

    double objectWidth  = (double)std::max(1, w - minMargin * 2) / (2 * layers - 1);
    double objectHeight = (double)std::max(1, h - minMargin * 2) / (2 * maxNodes - 1);

    int objectSize = std::min(125,
                              std::max(4, (int)std::floor(std::min(objectWidth, objectHeight))));

    double usableWidth = std::max(0.0, (double)w - minMargin * 2 - objectSize);
    double layerStepX  = usableWidth / std::max(1, layers - 1);

    double usedWidth   = objectSize + layerStepX * (layers - 1);
    double usedHeight  = objectSize * (2 * maxNodes - 1);

    double marginX = std::max(0.0, (w - usedWidth) / 2);
    double marginY = std::max(0.0, (h - usedHeight) / 2);
    // ----------------------------------------------------------------

    int layer = layerAtPos(pos,
                           layers,
                           marginX,      // правильный отступ по X
                           marginY,      // правильный отступ по Y
                           layerStepX,
                           usedHeight,   // полная высота колонки
                           objectSize);

    if (layer != hoveredLayer) {
        hoveredLayer = layer;
        update();
    }
}

void NetworkGraph::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;

    QPointF pos = event->pos();
    QMutexLocker locker(&model.mtx);

    int w = width();
    int h = height();

    const auto& layersStd = model.nn->getLayers();

    // ---------- расчёт геометрии (точная копия paintEvent) ----------
    int maxNodes = 1;
    maxNodes = std::max(maxNodes, (int)model.inputValues.size());
    for (const Layer& l : layersStd)
        maxNodes = std::max(maxNodes, (int)l.getValues().size());

    int layers = std::max(1, (int)layersStd.size() + 1);
    int minMargin = 80;

    double objectWidth  = (double)std::max(1, w - minMargin * 2) / (2 * layers - 1);
    double objectHeight = (double)std::max(1, h - minMargin * 2) / (2 * maxNodes - 1);

    int objectSize = std::min(125,
                              std::max(4, (int)std::floor(std::min(objectWidth, objectHeight))));

    double usableWidth = std::max(0.0, (double)w - minMargin * 2 - objectSize);
    double layerStepX  = usableWidth / std::max(1, layers - 1);

    double usedWidth   = objectSize + layerStepX * (layers - 1);
    double usedHeight  = objectSize * (2 * maxNodes - 1);

    double marginX = std::max(0.0, (w - usedWidth) / 2);
    double marginY = std::max(0.0, (h - usedHeight) / 2);
    // ----------------------------------------------------------------

    int layer = layerAtPos(pos,
                           layers,
                           marginX,
                           marginY,
                           layerStepX,
                           usedHeight,
                           objectSize);

    // Повторный клик по уже выбранному слою снимает выделение
    if (layer == selectedLayer) {
        selectedLayer = -1;
        update();
        return;
    }

    selectedLayer = layer;
    emit selectedLayerChanged();
    update();
}


// paintEvent; нужно нарисовать ноды, весы по model
void NetworkGraph::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QMutexLocker locker(&model.mtx);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing); // сглаживание

#ifdef DEBUG
    qDebug() << "[DEBUG] paintEvent";
#endif



    // фон
    p.fillRect(rect(), QColor(30, 30, 30));

    int w = width();
    int h = height();

    const auto& layersStd = model.nn->getLayers();

    int maxNodes = 1;
    maxNodes = std::max(maxNodes, (int)model.inputValues.size());

    for (const Layer& l : layersStd)
        maxNodes = std::max(maxNodes, (int)l.getValues().size());

    int layers = std::max(1, (int)layersStd.size() + 1);

    int minMargin = 80;

    // геометрия
    double objectWidth = (double)std::max(1, w - minMargin * 2) / (2 * layers - 1);
    double objectHeight = (double)std::max(1, h - minMargin * 2) / (2 * maxNodes - 1);

    int objectSize = std::min(
                             125,
                             std::max(4, (int)std::floor(std::min(objectWidth, objectHeight)))
                             );

    double usableWidth = std::max(0.0, (double)w - minMargin * 2 - objectSize);
    double layerStepX = usableWidth / std::max(1, layers - 1);

    double usedWidth = objectSize + layerStepX * (layers - 1);
    double usedHeight = objectSize * (2 * maxNodes - 1);

    double marginX = std::max(0.0, (w - usedWidth) / 2);
    double marginY = std::max(0.0, (h - usedHeight) / 2);



    // =========================
    // РИСОВАНИЕ ВЕСОВ
    // =========================

    std::vector<QPointF> prevNodesPos;

    for (int layerI = 0; layerI < layers; layerI++) {

        const std::vector<double>& values =
            (layerI == 0)
                ? model.inputValues
                : model.nn->getLayers()[layerI - 1].getValues();

        int nodeCount = (int)values.size();

        double columnHeight = objectSize * (2 * maxNodes - 1);
        double usedColumnHeight = objectSize * (2 * nodeCount - 1);
        double offsetY = (columnHeight - usedColumnHeight) / 2.0;

        std::vector<QPointF> newPrevNodesPos(nodeCount);

        for (int nodeI = 0; nodeI < nodeCount; nodeI++) {

            int nodeY = (int)(marginY + offsetY + objectSize * (nodeI * 2));
            int nodeX = (int)(marginX + layerStepX * layerI);

            QPointF center(nodeX + objectSize / 2,
                           nodeY + objectSize / 2);
            newPrevNodesPos[nodeI] = center;

            if (layerI == 0) continue;

            for (int i = 0; i < prevNodesPos.size(); i++) {
                double weight = model.nn->getLayers()[layerI-1].getWeights()(nodeI, i);
                int alpha = std::clamp(
                    (int)(std::abs(weight) * 200),
                    30,
                    255
                    );
                QPen pen;
                pen.setBrush(QBrush( weight > 0 ? QColor(0, 255, 0, alpha)
                                               : QColor(255, 0, 0, alpha)));
                pen.setWidth(3);
                p.setPen(pen);
                p.drawLine(center, prevNodesPos[i]);
            }
        }

        prevNodesPos = newPrevNodesPos;
    }


    // =========================
    // РИСОВАНИЕ НОДОВ
    // =========================

    for (int layerI = 0; layerI < layers; layerI++) {

        const std::vector<double>& values =
            (layerI == 0)
                ? model.inputValues
                : model.nn->getLayers()[layerI - 1].getValues();

        int nodeCount = (int)values.size();

        double columnHeight = objectSize * (2 * maxNodes - 1);
        double usedColumnHeight = objectSize * (2 * nodeCount - 1);
        double offsetY = (columnHeight - usedColumnHeight) / 2.0;


        for (int nodeI = 0; nodeI < nodeCount; nodeI++) {

            int nodeY = (int)(marginY + offsetY + objectSize * (nodeI * 2));
            int nodeX = (int)(marginX + layerStepX * layerI);
            const double nodeValue = values[nodeI];

            // единая шкала от -1 (ярко‑синий) до +1 (ярко‑красный)
            // 0 — нейтральный серый; интенсивность = |v| / (|v| + 1)
            double v = std::clamp(nodeValue, -1.0, 1.0);
            double abs_v = std::abs(v);
            double intensity = abs_v / (abs_v + 1.0);   // [0..1]

            QColor color;
            if (v > 0.0) {
                // Положительное → тёплый (красный) с нарастающей насыщенностью/яркостью
                int red   = 255;
                int green = (int)(255 * (1.0 - intensity));
                int blue  = (int)(255 * (1.0 - intensity));
                color = QColor(red, green, blue);
            } else if (v < 0.0) {
                // Отрицательное → холодный (синий)
                int red   = (int)(255 * (1.0 - intensity));
                int green = (int)(255 * (1.0 - intensity));
                int blue  = 255;
                color = QColor(red, green, blue);
            } else {
                // Ноль → серый (покой)
                int gray = 180;   // средне‑серый
                color = QColor(gray, gray, gray);
            }

            // Никакой прозрачности
            color.setAlpha(255);

            // Основной круг нейрона
            p.setBrush(color);

            // Контрастная обводка
            p.setPen(QPen(Qt::black, 2));
            p.drawEllipse(nodeX, nodeY, objectSize, objectSize);
        }
    }

    // ==========
    // РИСОВАНИЕ ОБВОДКИ ВЫБРАННОГО СЛОЯ (нейтральный фокус)
    // ==========
    if (selectedLayer >= 0) {
        double layerX = marginX + layerStepX * selectedLayer;

        QRectF baseRect(
            layerX - 12,
            marginY - 4,
            objectSize + 24,
            usedHeight + 8
            );

        // 1. Тонкая белая рамка с высокой непрозрачностью (основной индикатор)
        p.setBrush(Qt::NoBrush);
        QPen focusPen(QColor(220, 220, 220, 230), 2);   // почти белый, чуть прозрачный
        p.setPen(focusPen);
        p.drawRoundedRect(baseRect, 10, 10);

        // 2. Внешнее едва заметное свечение (белое)
        p.setPen(Qt::NoPen);
        QRectF glowRect = baseRect.adjusted(-3, -3, 3, 3);
        p.setBrush(QColor(255, 255, 255, 25));
        p.drawRoundedRect(glowRect, 12, 12);

        // 3. Лёгкое высветление фона внутри колонки (отделяет от остальной сетки)
        p.setBrush(QColor(255, 255, 255, 12));   // почти прозрачный белый
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(baseRect, 10, 10);

        // 4. Маленький маркер сверху (полоска‑индикатор активного слоя)
        double markW = 16;
        double markH = 4;
        QRectF markRect(
            baseRect.center().x() - markW / 2,
            baseRect.top() - markH - 3,
            markW, markH
            );
        p.setBrush(QColor(230, 230, 230, 220));
        p.drawRoundedRect(markRect, 2, 2);
    }

    // =========
    // РИСОВАНИЕ ОБВОДКИ ХОВЕРЕННОГО СЛОЯ
    // =========
    if (hoveredLayer >= 0 && hoveredLayer != selectedLayer) {
        double layerX = marginX + layerStepX * hoveredLayer;

        QRectF hoverRect(
            layerX - 10,
            marginY - 2,
            objectSize + 20,
            usedHeight + 4
            );

        // Лёгкая фоновая подсветка
        p.setBrush(QColor(255, 255, 255, 15));
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(hoverRect, 10, 10);

        // Пунктирный контур — временное наведение
        QPen hoverPen(QColor(200, 200, 200, 140), 1.5, Qt::DotLine);
        p.setPen(hoverPen);
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(hoverRect, 10, 10);
    }

}





