#pragma once

#include <QChartView>
#include <QChart>
#include <QLineSeries>
#include <QScatterSeries>
#include <QValueAxis>
#include <QTimer>
#include <QList>
#include <QPoint>
#include <QPointF>
#include <algorithm>
#include <cmath>
#include <limits>

// ---- Чистая математика диапазонов (без Qt) ----
namespace lossmath {

// Максимальное приближение относительно полного вида.
constexpr double kMaxZoom = 200.0;

struct Range {
    double xLo = 0, xHi = 10;
    double yLo = 0, yHi = 1;
};

inline bool isValid(const Range &r) {
    return std::isfinite(r.xLo) && std::isfinite(r.xHi) &&
           std::isfinite(r.yLo) && std::isfinite(r.yHi) &&
           r.xHi > r.xLo && r.yHi > r.yLo;
}

// «Красивый» шаг делений: число делений на диапазоне всегда <= target.
inline double niceStep(double range, int target) {
    double raw = range / target;
    if (!(raw > 0) || !std::isfinite(raw)) return 1;
    double mag = std::pow(10.0, std::floor(std::log10(raw)));
    double n = raw / mag;
    double s = n <= 1 ? 1 : n <= 2 ? 2 : n <= 5 ? 5 : 10;
    return s * mag;
}

// Сдвигает/сжимает [lo, hi], чтобы он не выходил за [limLo, limHi]
// и не был уже, чем (limHi - limLo) / kMaxZoom.
inline void clampRange(double &lo, double &hi, double limLo, double limHi) {
    const double limSpan = limHi - limLo;
    if (!(limSpan > 0)) return;
    const double minSpan = limSpan / kMaxZoom;
    double span = hi - lo;
    if (span < minSpan) {
        const double c = (lo + hi) / 2;
        lo = c - minSpan / 2;
        hi = c + minSpan / 2;
        span = minSpan;
    }
    if (span >= limSpan) { lo = limLo; hi = limHi; return; }
    if (lo < limLo) { hi += limLo - lo; lo = limLo; }
    if (hi > limHi) { lo -= hi - limHi; hi = limHi; }
}

// Полный вид: все данные + отступы. Всегда валиден и всегда содержит
// [minY, maxY] с запасом, чтобы маркеры не упирались в край.
inline Range fullRange(double maxX, double minY, double maxY) {
    Range r;
    r.xLo = 0;
    r.xHi = maxX > 0 ? maxX * 1.1 : 10;

    if (!std::isfinite(minY) || !std::isfinite(maxY) || maxY < minY) {
        r.yLo = 0; r.yHi = 1;
        return r;
    }
    const double span = maxY - minY;
    double pad;
    if (span > 0) pad = span * 0.15;
    else pad = std::abs(maxY) > 0 ? std::abs(maxY) * 0.1 : 1.0;

    r.yLo = minY - pad;
    r.yHi = maxY + pad;

    // Защита от вырожденного диапазона (шаг делений меньше ulp -> бесконечный
    // цикл генерации делений внутри QValueAxis).
    const double minSpan = std::max(1e-12, 1e-9 * std::max(std::abs(r.yLo), std::abs(r.yHi)));
    if (r.yHi - r.yLo < minSpan) {
        const double c = (r.yLo + r.yHi) / 2;
        r.yLo = c - minSpan / 2;
        r.yHi = c + minSpan / 2;
    }
    return r;
}

inline double approach(double cur, double tgt, double k) { return cur + (tgt - cur) * k; }

// Автомасштаб: расширение мгновенное (данные никогда не выходят из кадра),
// сужение плавное.
inline Range stepAuto(const Range &c, const Range &t, double k) {
    Range r;
    r.xLo = t.xLo < c.xLo ? t.xLo : approach(c.xLo, t.xLo, k);
    r.xHi = t.xHi > c.xHi ? t.xHi : approach(c.xHi, t.xHi, k);
    r.yLo = t.yLo < c.yLo ? t.yLo : approach(c.yLo, t.yLo, k);
    r.yHi = t.yHi > c.yHi ? t.yHi : approach(c.yHi, t.yHi, k);
    return r;
}

// Ручной зум: всё плавно.
inline Range stepManual(const Range &c, const Range &t, double k) {
    return {approach(c.xLo, t.xLo, k), approach(c.xHi, t.xHi, k),
            approach(c.yLo, t.yLo, k), approach(c.yHi, t.yHi, k)};
}

inline bool converged(const Range &c, const Range &t) {
    const double sx = t.xHi - t.xLo, sy = t.yHi - t.yLo;
    return std::abs(c.xLo - t.xLo) <= 1e-3 * sx && std::abs(c.xHi - t.xHi) <= 1e-3 * sx &&
           std::abs(c.yLo - t.yLo) <= 1e-3 * sy && std::abs(c.yHi - t.yHi) <= 1e-3 * sy;
}

} // namespace lossmath

// ---- Виджет ----
class QLabel;

// График ошибки обучения: автомасштаб по данным, плавный зум колесом,
// перетаскивание, маркеры последней (красная) и лучшей (зелёная) точек.
//
// addPoint() только складывает точку в буфер (O(1)); в график данные и
// диапазон попадают раз в кадр (~60 Гц) из onFrame(). Так быстрое обучение
// не заваливает GUI-поток перерисовками.
class LossChart : public QChartView {
    Q_OBJECT

public:
    explicit LossChart(QWidget *parent = nullptr);

public slots:
    void addPoint(int epoch, double err);
    void reset();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    using Range = lossmath::Range;

    Range fullRange() const;            // весь график с отступами
    Range currentRange() const;         // то, что сейчас на осях
    Range clamped(Range r) const;       // в границах полного вида и не слишком мелко
    void applyRange(const Range &r);    // сразу, без анимации
    void ensureFrame();                 // запустить покадровый таймер
    void onFrame();                     // сброс буфера в график + шаг диапазона
    void updateTitle(int epoch, double err);

    QChart *m_chart = nullptr;
    QLineSeries *m_series = nullptr;
    QScatterSeries *m_lastPt = nullptr;
    QScatterSeries *m_bestPt = nullptr;
    QValueAxis *m_axisX = nullptr;
    QValueAxis *m_axisY = nullptr;

    QLabel *m_tip = nullptr;            // подсказка к точке (пока курсор над ней)
    QTimer *m_frame = nullptr;
    Range m_target;                     // куда плавно едем (ручной зум)
    QList<QPointF> m_pending;           // точки, ещё не попавшие в серию
    qsizetype m_count = 0;

    bool m_autoRange = true;
    bool m_dragging = false;
    QPoint m_lastPos;

    double m_maxX = 0;
    double m_maxY = -std::numeric_limits<double>::infinity();
    double m_minY = std::numeric_limits<double>::infinity();
    double m_bestY = std::numeric_limits<double>::infinity();
    int m_bestEpoch = 0;
    bool m_bestDirty = false;
    int m_lastEpoch = 0;
    double m_lastErr = 0;
};