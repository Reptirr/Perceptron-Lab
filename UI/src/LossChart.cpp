
#include <QMouseEvent>
#include <QWheelEvent>
#include <QToolTip>
#include <QCursor>
#include <QLocale>
#include <QPen>
#include <QPainter>
#include <algorithm>
#include <cmath>
#include <LossChart.h>

static double niceStep(double range, int target) {
    double raw = range / target;
    if (raw <= 0) return 1;
    double mag = std::pow(10.0, std::floor(std::log10(raw)));
    double n = raw / mag;
    double s = n <= 1 ? 1 : n <= 2 ? 2 : n <= 5 ? 5 : 10;
    return s * mag;
}

LossChart::LossChart(QWidget *parent) : QChartView(parent) {
    m_series = new QLineSeries();
    QPen pen(QColor("#4a90d9"));
    pen.setWidth(2);
    m_series->setPen(pen);
    m_series->setPointsVisible(true);

    m_lastPt = new QScatterSeries();
    m_lastPt->setColor(QColor("#e74c3c"));
    m_lastPt->setBorderColor(Qt::white);
    m_lastPt->setMarkerSize(12);

    m_bestPt = new QScatterSeries();
    m_bestPt->setColor(QColor("#2ecc71"));
    m_bestPt->setBorderColor(Qt::white);
    m_bestPt->setMarkerSize(12);

    m_chart = new QChart();
    m_chart->legend()->hide();
    m_chart->setMargins(QMargins(0, 0, 0, 0));
    m_chart->setContentsMargins(0, 0, 0, 0);
    m_chart->setTitle("Ожидание данных...");
    m_chart->addSeries(m_series);
    m_chart->addSeries(m_bestPt);
    m_chart->addSeries(m_lastPt);

    m_axisX = new QValueAxis();
    m_axisX->setTitleText("Эпохи");
    m_axisX->setTickType(QValueAxis::TicksDynamic);
    m_axisX->setTickAnchor(0);
    m_axisX->setLabelFormat("%.0f");

    m_axisY = new QValueAxis();
    m_axisY->setTitleText("Ошибка");
    m_axisY->setLabelFormat("%.3g");
    m_axisY->setMinorTickCount(1);
    m_axisY->setMinorGridLineVisible(true);

    // создаётся сразу, но в график добавляется только при включении лог-шкалы
    m_axisYLog = new QLogValueAxis();
    m_axisYLog->setTitleText("Ошибка (лог.)");
    m_axisYLog->setLabelFormat("%.1e");
    m_axisYLog->setBase(10);
    m_axisYLog->setMinorTickCount(-1);

    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);
    for (auto *s : allSeries()) {
        s->attachAxis(m_axisX);
        s->attachAxis(m_axisY);
    }

    connect(m_series, &QLineSeries::hovered, this, [](const QPointF &p, bool state) {
        if (state)
            QToolTip::showText(QCursor::pos(),
                QString("Эпоха: %1\nОшибка: %2")
                    .arg(QLocale().toString(qRound(p.x())))
                    .arg(p.y(), 0, 'g', 5));
        else
            QToolTip::hideText();
    });

    setChart(m_chart);
    setRenderHint(QPainter::Antialiasing);

    applyAutoRange();
}

LossChart::~LossChart() {
    // ось, не добавленная в график, им не владеет
    if (m_logY) delete m_axisY;
    else delete m_axisYLog;
}

QList<QAbstractSeries *> LossChart::allSeries() const {
    return {m_series, m_lastPt, m_bestPt};
}

void LossChart::updateXTicks() {
    double step = std::max(1.0, niceStep(m_axisX->max() - m_axisX->min(), 8));
    m_axisX->setTickInterval(step);
}

void LossChart::applyAutoRange() {
    m_axisX->setRange(0, m_maxX > 0 ? m_maxX * 1.1 : 10);
    updateXTicks();

    if (m_logY) {
        const bool has = std::isfinite(m_minPosY);
        m_axisYLog->setRange(has ? m_minPosY / 2 : 0.1,
                             has ? std::max(m_maxY, m_minPosY) * 2 : 10);
    } else {
        m_axisY->setRange(0, m_maxY > 0 ? m_maxY * 1.2 : 1.0);
        m_axisY->applyNiceNumbers();
    }
}

void LossChart::updateTitle(int epoch, double err) {
    m_chart->setTitle(QString("Эпоха %1  |  ошибка %2  |  лучшая %3 (эпоха %4)")
        .arg(QLocale().toString(epoch))
        .arg(err, 0, 'g', 4)
        .arg(m_bestY, 0, 'g', 4)
        .arg(QLocale().toString(m_bestEpoch)));
}

void LossChart::addPoint(int epoch, double err) {
    if (!std::isfinite(err)) return;

    m_series->append(epoch, err);
    m_maxX = std::max(m_maxX, static_cast<double>(epoch));
    m_maxY = std::max(m_maxY, err);
    if (err > 0) m_minPosY = std::min(m_minPosY, err);

    if (err < m_bestY) {
        m_bestY = err;
        m_bestEpoch = epoch;
        m_bestPt->clear();
        m_bestPt->append(epoch, err);
    }
    m_lastPt->clear();
    m_lastPt->append(epoch, err);

    m_series->setPointsVisible(m_series->count() <= 200);
    updateTitle(epoch, err);

    if (m_autoRange) applyAutoRange();
}

void LossChart::reset() {
    m_series->clear();
    m_lastPt->clear();
    m_bestPt->clear();
    m_maxX = 0;
    m_maxY = 0;
    m_minPosY = std::numeric_limits<double>::infinity();
    m_bestY = std::numeric_limits<double>::infinity();
    m_bestEpoch = 0;
    m_chart->setTitle("Ожидание данных...");
    m_autoRange = true;
    applyAutoRange();
}

void LossChart::setLogScale(bool on) {
    if (on == m_logY) return;
    QAbstractAxis *from = m_logY ? static_cast<QAbstractAxis *>(m_axisYLog) : m_axisY;
    QAbstractAxis *to   = on     ? static_cast<QAbstractAxis *>(m_axisYLog) : m_axisY;

    for (auto *s : allSeries()) s->detachAxis(from);
    m_chart->removeAxis(from);
    m_chart->addAxis(to, Qt::AlignLeft);
    for (auto *s : allSeries()) s->attachAxis(to);

    m_logY = on;
    m_autoRange = true;
    applyAutoRange();
}

// Сдвигает/сжимает [lo, hi], чтобы он не выходил за [limLo, limHi]
// и не был уже minSpan. Если диапазон не меньше допустимого, берёт весь.
static void clampRange(double &lo, double &hi, double limLo, double limHi, double minSpan) {
    double span = hi - lo;
    if (span < minSpan) {
        const double c = (lo + hi) / 2;
        lo = c - minSpan / 2;
        hi = c + minSpan / 2;
        span = minSpan;
    }
    if (span >= limHi - limLo) {
        lo = limLo;
        hi = limHi;
        return;
    }
    if (lo < limLo) { hi += limLo - lo; lo = limLo; }
    if (hi > limHi) { lo -= hi - limHi; hi = limHi; }
}

void LossChart::clampRanges() {
    // X: от 0 до полного вида
    {
        double lo = m_axisX->min(), hi = m_axisX->max();
        const double limHi = m_maxX > 0 ? m_maxX * 1.1 : 10;
        clampRange(lo, hi, 0, limHi, std::min(1.0, limHi));
        m_axisX->setRange(lo, hi);
        updateXTicks();
    }

    // Y
    if (m_logY) {
        const bool has = std::isfinite(m_minPosY);
        const double limLo = std::log10(has ? m_minPosY / 2 : 0.1);
        const double limHi = std::log10(has ? std::max(m_maxY, m_minPosY) * 2 : 10);
        double lo = std::log10(m_axisYLog->min());
        double hi = std::log10(m_axisYLog->max());
        clampRange(lo, hi, limLo, limHi, std::min(0.01, limHi - limLo));
        m_axisYLog->setRange(std::pow(10.0, lo), std::pow(10.0, hi));
    } else {
        double lo = m_axisY->min(), hi = m_axisY->max();
        const double limHi = m_maxY > 0 ? m_maxY * 1.2 : 1.0;
        clampRange(lo, hi, 0, limHi, limHi * 1e-6);
        m_axisY->setRange(lo, hi);
    }
}

void LossChart::wheelEvent(QWheelEvent *event) {
    const int dy = event->angleDelta().y();
    if (dy == 0) {
        event->ignore();
        return;
    }
    const double factor = dy > 0 ? 0.8 : 1.25;

    QPointF v = m_chart->mapToValue(
        m_chart->mapFromScene(mapToScene(event->position().toPoint())), m_series);
    // курсор мог оказаться на подписях осей: берём ближайшую точку внутри диапазона
    v.setX(std::clamp(v.x(), m_axisX->min(), m_axisX->max()));

    if (event->modifiers() & Qt::ControlModifier) {
        // масштаб по Y вокруг курсора
        if (m_logY) {
            const double lo = std::log10(m_axisYLog->min());
            const double hi = std::log10(m_axisYLog->max());
            const double c = std::clamp(std::log10(std::max(v.y(), 1e-300)), lo, hi);
            m_axisYLog->setRange(std::pow(10.0, c - (c - lo) * factor),
                                 std::pow(10.0, c + (hi - c) * factor));
        } else {
            const double c = std::clamp(v.y(), m_axisY->min(), m_axisY->max());
            m_axisY->setRange(c - (c - m_axisY->min()) * factor,
                              c + (m_axisY->max() - c) * factor);
        }
    } else {
        // масштаб по X вокруг курсора
        m_axisX->setRange(v.x() - (v.x() - m_axisX->min()) * factor,
                          v.x() + (m_axisX->max() - v.x()) * factor);
    }
    clampRanges();
    m_autoRange = false;
    event->accept();
}

void LossChart::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_lastPos = event->pos();
        event->accept();
        return;
    }
    QChartView::mousePressEvent(event);
}

void LossChart::mouseMoveEvent(QMouseEvent *event) {
    if (!m_dragging) {
        // без перетаскивания отдаём событие базе, чтобы работал hover
        QChartView::mouseMoveEvent(event);
        return;
    }
    const QPoint d = event->pos() - m_lastPos;
    m_lastPos = event->pos();
    m_chart->scroll(-d.x(), d.y());
    clampRanges();
    m_autoRange = false;
    event->accept();
}

void LossChart::mouseReleaseEvent(QMouseEvent *event) {
    if (m_dragging) {
        m_dragging = false;
        event->accept();
        return;
    }
    QChartView::mouseReleaseEvent(event);
}

void LossChart::mouseDoubleClickEvent(QMouseEvent *event) {
    m_autoRange = true;
    applyAutoRange();
    event->accept();
}