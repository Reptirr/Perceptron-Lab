#pragma once

#include <QChartView>
#include <QChart>
#include <QLineSeries>
#include <QScatterSeries>
#include <QValueAxis>
#include <QLogValueAxis>
#include <QPoint>
#include <QList>
#include <limits>

// График ошибки обучения: автомасштаб, лог-шкала, зум/перетаскивание мышью,
// маркеры последней (красная) и лучшей (зелёная) точек.
class LossChart : public QChartView {
    Q_OBJECT

public:
    explicit LossChart(QWidget *parent = nullptr);
    ~LossChart() override;

public slots:
    void addPoint(int epoch, double err);
    void reset();
    void setLogScale(bool on);

protected:
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    void applyAutoRange();
    void clampRanges();
    void updateXTicks();
    void updateTitle(int epoch, double err);
    QList<QAbstractSeries *> allSeries() const;

    QChart *m_chart = nullptr;
    QLineSeries *m_series = nullptr;
    QScatterSeries *m_lastPt = nullptr;
    QScatterSeries *m_bestPt = nullptr;
    QValueAxis *m_axisX = nullptr;
    QValueAxis *m_axisY = nullptr;
    QLogValueAxis *m_axisYLog = nullptr;

    bool m_logY = false;
    bool m_autoRange = true;
    bool m_dragging = false;
    QPoint m_lastPos;

    double m_maxX = 0;
    double m_maxY = 0;
    double m_minPosY = std::numeric_limits<double>::infinity();
    double m_bestY = std::numeric_limits<double>::infinity();
    int m_bestEpoch = 0;
};