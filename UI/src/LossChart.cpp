#include <LossChart.h>

#include <QMouseEvent>
#include <QLabel>
#include <QCursor>
#include <QTimer>
#include <QLocale>
#include <QPen>
#include <QPainter>
#include <algorithm>
#include <cmath>

using lossmath::niceStep;

namespace {

// Палитра в стиле кнопок приложения
const QColor kAppBg("#232323");   // фон приложения
const QColor kBg("#1A1A1A");      // фон графика: чуть темнее приложения
const QColor kBorder("#3A3A3A");  // обводка графика
const QColor kText("#CCCCCC");
const QColor kGrid("#3A3A3A");
const QColor kGridMinor("#2A2A2A");
const QColor kAxisLine("#555555");

constexpr int kFrameMs = 16;       // период кадра
constexpr double kSmooth = 0.35;   // доля пути к цели за кадр

// Число без экспоненты: 4 значащие цифры, лишние нули справа убираются.
// 1.234e-10 -> "0.0000000001234", 2.5 -> "2.5", 0 -> "0".
QString plainNumber(double v) {
    if (v == 0 || !std::isfinite(v)) return QStringLiteral("0");
    const int mag = static_cast<int>(std::floor(std::log10(std::abs(v))));
    const int decimals = std::clamp(3 - mag, 0, 20);
    QString s = QString::number(v, 'f', decimals);
    if (s.contains('.')) {
        while (s.endsWith('0')) s.chop(1);
        if (s.endsWith('.')) s.chop(1);
    }
    return s;
}

void styleAxis(QAbstractAxis *axis) {
    axis->setLabelsBrush(QBrush(kText));
    axis->setTitleBrush(QBrush(kText));
    axis->setLinePen(QPen(kAxisLine, 1));
    axis->setGridLinePen(QPen(kGrid, 1));
    axis->setMinorGridLinePen(QPen(kGridMinor, 1));
}

} // namespace

LossChart::LossChart(QWidget *parent) : QChartView(parent) {
    m_series = new QLineSeries();
    QPen pen(QColor("#4a90d9"));
    pen.setWidth(2);
    m_series->setPen(pen);
    m_series->setPointsVisible(true);

    m_lastPt = new QScatterSeries();
    m_lastPt->setColor(QColor("#e74c3c"));
    m_lastPt->setBorderColor(kBg);
    m_lastPt->setMarkerSize(12);

    m_bestPt = new QScatterSeries();
    m_bestPt->setColor(QColor("#2ecc71"));
    m_bestPt->setBorderColor(kBg);
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
    m_axisX->setMinorTickCount(1);
    m_axisX->setMinorGridLineVisible(true);

    m_axisY = new QValueAxis();
    m_axisY->setTitleText("Ошибка");
    m_axisY->setTickType(QValueAxis::TicksDynamic);
    m_axisY->setTickAnchor(0);
    m_axisY->setLabelFormat("%.5g");
    m_axisY->setMinorTickCount(1);
    m_axisY->setMinorGridLineVisible(true);

    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);
    for (QAbstractSeries *s : {static_cast<QAbstractSeries *>(m_series),
                               static_cast<QAbstractSeries *>(m_bestPt),
                               static_cast<QAbstractSeries *>(m_lastPt)}) {
        s->attachAxis(m_axisX);
        s->attachAxis(m_axisY);
    }

    // Подсказка к точке: собственная метка вместо QToolTip (тот гаснет по таймеру).
    // Видна, пока курсор над точкой; скрывается при уходе курсора и при нажатии мыши.
    m_tip = new QLabel(viewport());
    m_tip->setStyleSheet(QString("QLabel { background:%1; color:%2; border:1px solid %3;"
                                 " border-radius:4px; padding:4px 6px; }")
                             .arg(kAppBg.name(), kText.name(), kAxisLine.name()));
    m_tip->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_tip->hide();

    connect(m_series, &QLineSeries::hovered, this, [this](const QPointF &p, bool state) {
        if (!state) {         // курсор ушёл с точки
            m_tip->hide();
            return;
        }
        m_tip->setText(QString("Эпоха: %1\nОшибка: %2")
                           .arg(QLocale().toString(qRound(p.x())))
                           .arg(p.y(), 0, 'g', 5));
        m_tip->adjustSize();
        QPoint pos = viewport()->mapFromGlobal(QCursor::pos()) + QPoint(14, 14);
        pos.setX(std::clamp(pos.x(), 0, std::max(0, viewport()->width() - m_tip->width())));
        pos.setY(std::clamp(pos.y(), 0, std::max(0, viewport()->height() - m_tip->height())));
        m_tip->move(pos);
        m_tip->show();
        m_tip->raise();
    });

    // стиль
    // толщина 2: половина пера обрезается краем виджета, видна 1px обводка
    m_chart->setBackgroundBrush(QBrush(kBg));
    m_chart->setBackgroundPen(QPen(kBorder, 2));
    m_chart->setBackgroundRoundness(6);
    m_chart->setPlotAreaBackgroundBrush(QBrush(kBg));
    m_chart->setPlotAreaBackgroundVisible(true);
    m_chart->setTitleBrush(QBrush(kText));
    styleAxis(m_axisX);
    styleAxis(m_axisY);

    setChart(m_chart);
    // углы со скруглением показывают фон view, он должен совпадать с фоном приложения
    setBackgroundBrush(QBrush(kAppBg));
    setFrameShape(QFrame::NoFrame);
    setRenderHint(QPainter::Antialiasing);

    // покадровое обновление: сброс буфера точек + плавный шаг диапазона
    m_frame = new QTimer(this);
    m_frame->setTimerType(Qt::PreciseTimer);
    m_frame->setInterval(kFrameMs);
    connect(m_frame, &QTimer::timeout, this, [this] { onFrame(); });

    m_target = fullRange();
    applyRange(m_target);
}

// ---------- диапазоны ----------

LossChart::Range LossChart::fullRange() const {
    return lossmath::fullRange(m_maxX, m_minY, m_maxY);
}

LossChart::Range LossChart::currentRange() const {
    return {m_axisX->min(), m_axisX->max(), m_axisY->min(), m_axisY->max()};
}

LossChart::Range LossChart::clamped(Range r) const {
    const Range full = fullRange();
    lossmath::clampRange(r.xLo, r.xHi, full.xLo, full.xHi);
    lossmath::clampRange(r.yLo, r.yHi, full.yLo, full.yHi);
    return r;
}

// Порядок важен: QValueAxis с TicksDynamic строит деления сразу при setRange,
// по текущему tickInterval. Если диапазон резко вырос, а шаг остался старым,
// получаются миллионы делений и GUI замирает. Поэтому при росте диапазона
// сначала меняем шаг, при сжатии сначала диапазон: число делений всегда мало.
void LossChart::applyRange(const Range &r) {
    if (!lossmath::isValid(r)) return;

    auto apply = [](QValueAxis *axis, double lo, double hi, double interval) {
        if (hi - lo >= axis->max() - axis->min()) {
            axis->setTickInterval(interval);
            axis->setRange(lo, hi);
        } else {
            axis->setRange(lo, hi);
            axis->setTickInterval(interval);
        }
    };
    // Густота делений по X: примерно одно на 55 px ширины области графика
    // (от 10 до 40). Шаг не меньше 1: эпохи целые.
    const double pw = m_chart->plotArea().width();
    const int xTicks = pw > 0 ? std::clamp(static_cast<int>(pw / 55.0), 10, 40) : 16;
    apply(m_axisX, r.xLo, r.xHi, std::max(1.0, niceStep(r.xHi - r.xLo, xTicks)));
    apply(m_axisY, r.yLo, r.yHi, niceStep(r.yHi - r.yLo, 6));
}

void LossChart::ensureFrame() {
    if (!m_frame->isActive()) m_frame->start();
}

void LossChart::onFrame() {
    if (!m_pending.isEmpty()) {
        m_series->append(m_pending);
        m_count += m_pending.size();
        m_pending.clear();
        m_series->setPointsVisible(m_count <= 200);
        m_lastPt->replace(QList<QPointF>{QPointF(m_lastEpoch, m_lastErr)});
        if (m_bestDirty) {
            m_bestPt->replace(QList<QPointF>{QPointF(m_bestEpoch, m_bestY)});
            m_bestDirty = false;
        }
        updateTitle(m_lastEpoch, m_lastErr);
    }

    if (m_autoRange) m_target = fullRange();
    const Range cur = currentRange();
    const Range next = m_autoRange ? lossmath::stepAuto(cur, m_target, kSmooth)
                                   : lossmath::stepManual(cur, m_target, kSmooth);
    applyRange(next);

    if (lossmath::converged(next, m_target)) {
        applyRange(m_target);
        m_frame->stop();
    }
}

void LossChart::updateTitle(int epoch, double err) {
    m_chart->setTitle(QString("Эпоха %1  |  ошибка %2  |  лучшая %3 (эпоха %4)")
        .arg(QLocale().toString(epoch))
        .arg(plainNumber(err))
        .arg(plainNumber(m_bestY))
        .arg(QLocale().toString(m_bestEpoch)));
}

// ---------- данные ----------

void LossChart::addPoint(int epoch, double err) {
    if (!std::isfinite(err)) return;

    m_pending.append(QPointF(epoch, err));
    m_maxX = std::max(m_maxX, static_cast<double>(epoch));
    m_maxY = std::max(m_maxY, err);
    m_minY = std::min(m_minY, err);
    if (err < m_bestY) {
        m_bestY = err;
        m_bestEpoch = epoch;
        m_bestDirty = true;
    }
    m_lastEpoch = epoch;
    m_lastErr = err;

    ensureFrame();
}

void LossChart::reset() {
    m_tip->hide();
    m_frame->stop();
    m_pending.clear();
    m_count = 0;
    m_series->clear();
    m_lastPt->clear();
    m_bestPt->clear();
    m_maxX = 0;
    m_maxY = -std::numeric_limits<double>::infinity();
    m_minY = std::numeric_limits<double>::infinity();
    m_bestY = std::numeric_limits<double>::infinity();
    m_bestEpoch = 0;
    m_bestDirty = false;
    m_chart->setTitle("Ожидание данных...");
    m_autoRange = true;
    m_target = fullRange();
    applyRange(m_target);
}

// ---------- мышь ----------

void LossChart::resizeEvent(QResizeEvent *event) {
    QChartView::resizeEvent(event);
    applyRange(currentRange());   // пересчитать густоту делений под новую ширину
}

void LossChart::wheelEvent(QWheelEvent *event) {
    const int dy = event->angleDelta().y();
    if (dy == 0) {
        event->ignore();
        return;
    }
    // один «щелчок» колеса (120) = шаг 0.8; на тачпадах шаг пропорционально меньше
    const double factor = std::pow(0.8, dy / 120.0);

    const QRectF pa = m_chart->plotArea();
    if (pa.width() <= 0 || pa.height() <= 0) return;
    const QPointF p = m_chart->mapFromScene(mapToScene(event->position().toPoint()));
    // положение курсора внутри области графика (0..1); на подписях осей берём край
    const double fx = std::clamp((p.x() - pa.left()) / pa.width(), 0.0, 1.0);
    const double fy = std::clamp((pa.bottom() - p.y()) / pa.height(), 0.0, 1.0);

    // в ручном режиме наращиваем зум от цели, а не от промежуточного кадра
    Range r = m_autoRange ? currentRange() : m_target;

    if (event->modifiers() & Qt::ControlModifier) {
        const double a = r.yLo + fy * (r.yHi - r.yLo);   // значение под курсором
        r.yLo = a - (a - r.yLo) * factor;
        r.yHi = a + (r.yHi - a) * factor;
    } else {
        const double a = r.xLo + fx * (r.xHi - r.xLo);
        r.xLo = a - (a - r.xLo) * factor;
        r.xHi = a + (r.xHi - a) * factor;
    }

    m_autoRange = false;
    m_target = clamped(r);
    ensureFrame();
    event->accept();
}

void LossChart::mousePressEvent(QMouseEvent *event) {
    m_tip->hide();   // подсказка убирается нажатием мыши
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
    applyRange(clamped(currentRange()));
    m_autoRange = false;
    m_target = currentRange();   // чтобы кадровый таймер не тянул обратно
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
    ensureFrame();
    event->accept();
}