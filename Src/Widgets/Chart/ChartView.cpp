#include "ChartView.h"

namespace Widgets::Chart {
ChartView::ChartView(QWidget *parent)
    : QChartView(parent)
{
    m_update = true;

    m_xaxisTime = new QDateTimeAxis(this);
    m_xaxisValue = new QValueAxis(this);

    //    QFont font_time = Xaxis_Time->labelsFont();
    //    font_time.setPixelSize(13);
    //    Xaxis_Time->setLabelsFont(font_time);

    //    QFont font_value = Xaxis_Time->labelsFont();
    //    font_value.setPixelSize(13);
    //    Xaxis_Time->setLabelsFont(font_value);

    m_xaxis = m_xaxisValue;
    m_minRange = m_minR;

    m_xaxisValue->setLabelFormat("%.2f mA");
    m_xaxisValue->setRange(0, m_minR);
    m_xaxisValue->setMinorTickCount(4);

    m_xaxisTime->setFormat("mm:ss.zzz");
    m_xaxisTime->setRange(QDateTime::fromMSecsSinceEpoch(0),
                          QDateTime::fromMSecsSinceEpoch(m_minRTime));

    chart()->addAxis(m_xaxis, Qt::AlignBottom);

    m_empty = true;
    m_zoomed = false;

    chart()->addSeries(&m_marker_X);
    chart()->addSeries(&m_marker_Y);

    m_marker_X.attachAxis(m_xaxis);
    m_marker_Y.attachAxis(m_xaxis);

    m_marker_X.setColor(Qt::gray);
    m_marker_Y.setColor(Qt::gray);

    chart()->legend()->markers(&m_marker_X)[0]->setVisible(false);
    chart()->legend()->markers(&m_marker_Y)[0]->setVisible(false);

    m_marker_X.setUseOpenGL();
    m_marker_Y.setUseOpenGL();

    m_coordItem = new QGraphicsSimpleTextItem(this->chart());

    QFont font;
    font.setPointSize(10);
    m_coordItem->setFont(font);
    m_coordItem->show();
    m_coordItem->setVisible(false);

    QOpenGLWidget *glWidget = this->findChild<QOpenGLWidget *>();
    glWidget->setAttribute(Qt::WA_TransparentForMouseEvents);

    m_axisTimer.setInterval(100); // 10 Hz
    connect(&m_axisTimer, &QTimer::timeout, this, &ChartView::updateAxes);
    m_axisTimer.start();

    m_markerTimer.start();
}

int ChartView::seriesCount() const
{
    return m_mySeries.size();
}

bool ChartView::isSeriesVisible(int series) const
{
    if (series < 0 || series >= m_mySeries.size())
        return false;

    return m_mySeries[series]->isVisible();
}

bool ChartView::allowMarkerUpdate()
{
    if (!m_markerTimer.isValid()) {
        m_markerTimer.start();
        return true;
    }
    if (m_markerTimer.elapsed() < 16)
        return false;
    m_markerTimer.restart();
    return true;
}

QVector<Series*>& ChartView::series()
{
    return m_mySeries;
}

void ChartView::updateAxes()
{
    if (!m_axesDirty) return;
    if (m_zoomed) { m_axesDirty = false; return; }
    if (!m_update) { m_axesDirty = false; return; }

    m_axisX_min = m_min_X;
    m_axisX_max = qMax(m_max_X, m_min_X + m_minRange);

    if (m_xaxis == m_xaxisValue) {
        m_axisX_min = qMax<qreal>(qFloor(m_axisX_min), 0.0);

        m_axisX_max = qCeil(m_axisX_max);
        m_xaxis->setRange(m_axisX_min, m_axisX_max);
    } else {
        m_axisX_min = qMax<qreal>(qFloor(m_axisX_min / 1000.0), 0.0) * 1000.0;
        m_axisX_max = qMax(m_axisX_min + 1000.0, qCeil(m_axisX_max / 1000.0) * 1000.0);
        m_xaxis->setRange(QDateTime::fromMSecsSinceEpoch(qCeil(m_axisX_min)),
                          QDateTime::fromMSecsSinceEpoch(qCeil(m_axisX_max)));
    }

    autoScale(m_axisX_min, m_axisX_max);
    m_axesDirty = false;
}

void ChartView::drawMarkers(QPoint pos)
{
    QSet<quint8> set;
    QList<Series*> mySeries;

    set.clear();

    for (Series* mySerial : std::as_const(m_mySeries)) {
        if (!mySerial->isVisible())
            continue;

        if (set.contains(mySerial->getAxisN()))
            continue;

        set.insert(mySerial->getAxisN());
        mySeries.push_back(mySerial);
    }

    QPointF curVal = this->chart()->mapToValue(pos);

    QString format;
    QString coordStr = "";

    for (Series* mySerial : mySeries) {
        format = m_yaxis.at(mySerial->getAxisN())->labelFormat();
        coordStr += QString::asprintf(format.toLocal8Bit(),
                                      this->chart()->mapToValue(pos, mySerial).y())
                    + "\n";
    }

    if (m_xaxis == m_xaxisValue) {
        format = m_xaxisValue->labelFormat();
        coordStr += QString::asprintf(format.toLocal8Bit(), curVal.x());
    } else {
        QTime time = QTime::fromMSecsSinceStartOfDay(curVal.x());
        coordStr += time.toString(m_xaxisTime->format());
    }

    m_coordItem->setText(coordStr);
    m_coordItem->setPos(pos.x() + 10, pos.y() - 20 * (set.count() + 1));

    m_marker_X.clear();

    qreal minXaxisValue;
    qreal maxXaxisValue;

    if (m_xaxis == m_xaxisValue) {
        minXaxisValue = m_xaxisValue->min();
        maxXaxisValue = m_xaxisValue->max();
    } else {
        minXaxisValue = m_xaxisTime->min().toMSecsSinceEpoch();
        maxXaxisValue = m_xaxisTime->max().toMSecsSinceEpoch();
    }

    m_marker_X.append(minXaxisValue, curVal.y());
    m_marker_X.append(maxXaxisValue, curVal.y());
    m_marker_X.setVisible(true);

    m_marker_Y.clear();
    m_marker_Y.append(curVal.x(), m_yaxis.last()->min());
    m_marker_Y.append(curVal.x(), m_yaxis.last()->max());
    m_marker_Y.setVisible(true);

    chart()->legend()->markers(&m_marker_X)[0]->setVisible(false);
    chart()->legend()->markers(&m_marker_Y)[0]->setVisible(false);
}

void ChartView::setLabelXformat(QString format)
{
    m_xaxisValue->setLabelFormat(format);
}

void ChartView::autoScale(qreal min, qreal max)
{
    for (int ax = 0; ax < m_yaxis.count(); ax++) {
        bool first = true;
        qreal Ymin = 0;
        qreal Ymax = 0;

        foreach (const Series* mySerial, m_mySeries) {
            if ((mySerial->getAxisN() == ax) && (mySerial->isVisible())) {
                foreach (QPointF value, mySerial->points()) {
                    if ((value.x() >= min) && (value.x() <= max)) {
                        if (first) {
                            first = false;
                            Ymin = value.y();
                            Ymax = value.y();
                        } else {
                            Ymin = qMin(value.y(), Ymin);
                            Ymax = qMax(value.y(), Ymax);
                        }
                    }
                }
            }
        }

        foreach (const Series* mySerial, m_mySeriesDubl) {
            if ((mySerial->getAxisN() == ax) && (mySerial->isVisible())) {
                foreach (QPointF value, mySerial->points()) {
                    if ((value.x() >= min) && (value.x() <= max)) {
                        if (first) {
                            first = false;
                            Ymin = value.y();
                            Ymax = value.y();
                        } else {
                            Ymin = qMin(value.y(), Ymin);
                            Ymax = qMax(value.y(), Ymax);
                        }
                    }
                }
            }
        }

        qreal Llim = 0;

        qreal Hlim = qMax(1.0, Ymax * 1.1);
        Hlim = qCeil(Hlim);

        m_yaxis[ax]->setRange(Llim, Hlim);
    }

    if (m_coordItem) {
        if (m_coordItem->isVisible()) {
            drawMarkers(m_markersPos);
        }
    }
}

void ChartView::autoUpdate(bool update)
{
    m_update = update;

    if (update) {
        m_axisX_min = m_min_X;
        m_axisX_max = qMax(m_max_X, m_min_X + m_minRange);

        if (m_xaxis == m_xaxisValue) {
            m_axisX_min = qMax(qFloor(m_axisX_min), 0);
            m_axisX_max = qCeil(m_axisX_max);
            m_xaxis->setRange(m_axisX_min, m_axisX_max);
        } else {
            m_axisX_min = qMax(qFloor(m_axisX_min / 1000.0), 0) * 1000.0;
            qMax(m_axisX_min + 1000.0, m_axisX_max = qCeil(m_axisX_max / 1000.0) * 1000.0);
            m_xaxis->setRange(QDateTime::fromMSecsSinceEpoch(qCeil(m_axisX_min)),
                              QDateTime::fromMSecsSinceEpoch(qCeil(m_axisX_max)));
        }

        autoScale(m_min_X, m_max_X);
    }
}

void ChartView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        int nearSeriesN = -1;
        const int nearPtIdx = findNearestPointIndex(event->pos(), &nearSeriesN);
        if (nearPtIdx >= 0) {
            m_drag = { true, static_cast<quint8>(nearSeriesN), nearPtIdx,
                       chart()->mapToValue(event->pos(), m_mySeries[nearSeriesN]) };
            setCursor(Qt::ClosedHandCursor);
            event->accept(); return;
        }
        const int nearLine = findNearestDraggableSeries(event->pos());
        if (nearLine >= 0) {
            m_drag = { true, static_cast<quint8>(nearLine), -1,
                       chart()->mapToValue(event->pos(), m_mySeries[nearLine]) };
            setCursor(Qt::ClosedHandCursor);
            event->accept(); return;
        }
        this->setRubberBand(QChartView::HorizontalRubberBand);
        m_X1 = chart()->mapToValue(event->pos()).x();
    }

    QChartView::mousePressEvent(event);
}

void ChartView::mouseMoveEvent(QMouseEvent *event)
{
    if (m_drag.active) {
        Series* s = m_mySeries[m_drag.seriesN];
        const QPointF cur = chart()->mapToValue(event->pos(), s);
        if (m_drag.pointIdx >= 0) {
            QList<QPointF> pts = s->points();
            if (m_drag.pointIdx < pts.size()) {
                pts[m_drag.pointIdx] = cur;
                s->replace(pts);
            }
        } else {
            const QPointF delta = cur - m_drag.prevValue;
            m_drag.prevValue = cur;
            QList<QPointF> pts = s->points();
            for (auto& p : pts) p += delta;
            s->replace(pts);
        }
        event->accept(); return;
    }
    if (!m_draggableSeries.isEmpty()) {
        int nearSeriesN = -1;
        const int nearPt = findNearestPointIndex(event->pos(), &nearSeriesN);
        setCursor(nearPt >= 0 ? Qt::SizeAllCursor :
                  (findNearestDraggableSeries(event->pos()) >= 0 ?
                   Qt::SizeAllCursor : Qt::ArrowCursor));
    }
    if (m_mySeries.count()) {
        if (allowMarkerUpdate()) {
            m_coordItem->setVisible(true);
            m_markersPos = event->pos();
            drawMarkers(m_markersPos);
        }
    }
    QChartView::mouseMoveEvent(event);
}

void ChartView::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_drag.active && event->button() == Qt::LeftButton) {
        m_drag.active = false;
        setCursor(Qt::ArrowCursor);
        const QList<QPointF> pts = m_mySeries[m_drag.seriesN]->points();
        if (!pts.isEmpty()) emit seriesDragged(m_drag.seriesN, pts);
        event->accept(); return;
    }
    if (event->button() == Qt::RightButton) {
        auto dot = [](QPointF a, QPointF b) { return a.x()*b.x() + a.y()*b.y(); };
        // 1. Удалить точку
        int nearSeriesN = -1;
        const int nearPtIdx = findNearestPointIndex(event->pos(), &nearSeriesN);
        if (nearPtIdx >= 0) {
            Series* s = m_mySeries[nearSeriesN];
            QList<QPointF> pts = s->points();
            if (pts.size() > 2) {
                pts.removeAt(nearPtIdx);
                s->replace(pts);
                highlightBendPoints(static_cast<quint8>(nearSeriesN));
                emit seriesDragged(static_cast<quint8>(nearSeriesN), pts);
            }
            this->setRubberBand(QChartView::NoRubberBand);
            event->accept(); return;
        }
        // 2. Вставить точку
        const int nearLine = findNearestDraggableSeries(event->pos());
        if (nearLine >= 0) {
            Series* s = m_mySeries[nearLine];
            const QPointF newVal = chart()->mapToValue(event->pos(), s);
            QList<QPointF> pts = s->points();
            int insertIdx = pts.size();
            qreal minDist = qInf();
            for (int i = 0; i + 1 < pts.size(); ++i) {
                const QPointF A = chart()->mapToPosition(pts[i], s);
                const QPointF B = chart()->mapToPosition(pts[i+1], s);
                const QPointF M = event->pos();
                const QPointF AB = B - A;
                const qreal len2 = dot(AB, AB);
                if (qFuzzyIsNull(len2)) continue;
                const qreal t = qBound(0.0, dot(M - A, AB) / len2, 1.0);
                const qreal dist = QLineF(M, A + t * AB).length();
                if (dist < minDist) { minDist = dist; insertIdx = i + 1; }
            }
            pts.insert(insertIdx, newVal);
            s->replace(pts);
            highlightBendPoints(static_cast<quint8>(nearLine));
            emit seriesDragged(static_cast<quint8>(nearLine), pts);
            this->setRubberBand(QChartView::NoRubberBand);
            event->accept(); return;
        }
        // 3. Zoom out
        if (m_mySeries.count()) zoomOut();
        this->setRubberBand(QChartView::NoRubberBand);
        QChartView::mouseReleaseEvent(event); return;
    }
    // Левая кнопка — zoom in
    if (m_mySeries.count() && event->button() == Qt::LeftButton) {
        m_X2 = chart()->mapToValue(event->pos()).x();
        zoomIn(qMin(m_X1, m_X2), qMax(m_X1, m_X2));
    }
    this->setRubberBand(QChartView::NoRubberBand);
    QChartView::mouseReleaseEvent(event);
}

void ChartView::leaveEvent(QEvent *)
{
    m_coordItem->setVisible(false);
    m_marker_X.setVisible(false);
    m_marker_Y.setVisible(false);
}

void ChartView::useTimeaxis(bool useTime)
{
    m_marker_X.detachAxis(m_xaxis);
    m_marker_Y.detachAxis(m_xaxis);

    chart()->removeAxis(m_xaxis);

    if (useTime) {
        m_xaxis = m_xaxisTime;
        m_minRange = m_minRTime;
    } else {
        m_xaxis = m_xaxisValue;
        m_minRange = m_minR;
    }

    chart()->addAxis(m_xaxis, Qt::AlignBottom);

    m_marker_X.attachAxis(m_xaxis);
    m_marker_Y.attachAxis(m_xaxis);
}

void ChartView::addAxis(QString format)
{
    if (m_yaxis.count()) {
        m_marker_X.detachAxis(m_yaxis.last());
        m_marker_Y.detachAxis(m_yaxis.last());
    }

    m_yaxis.emplace_back(new QValueAxis(this));
    m_yaxis.last()->setLabelFormat(format);
    chart()->addAxis(m_yaxis.last(), Qt::AlignLeft);

    m_yaxis.last()->setRange(0, 0.01);
    m_yaxis.last()->setMinorTickCount(4);

    //    QFont font = Yaxis.last()->labelsFont();
    //    font.setPixelSize(13);
    //    Yaxis.last()->setLabelsFont(font);

    m_marker_X.attachAxis(m_yaxis.last());
    m_marker_Y.attachAxis(m_yaxis.last());
}

void ChartView::setSeriesMarkersOnly(quint8 seriesN, bool on)
{
    if (seriesN >= m_mySeries.size()) return;
    m_mySeries[seriesN]->setMarkersOnly(on);

    if (seriesN < m_mySeriesDubl.size())
        m_mySeriesDubl[seriesN]->setMarkersOnly(on);
}

void ChartView::addSeries(quint8 axisN, QString name, QColor color)
{
    if (axisN >= m_yaxis.count()) {
        return;
    }

    m_mySeriesDubl.emplace_back(new Series(this, axisN));

    chart()->addSeries(m_mySeriesDubl.last());

    m_mySeriesDubl.last()->setColor(color.lighter(170));

    m_mySeriesDubl.last()->attachAxis(m_yaxis[axisN]);
    m_mySeriesDubl.last()->attachAxis(m_xaxis);

    chart()->legend()->markers(m_mySeriesDubl.last())[0]->setVisible(false);

    // this->thread()->msleep(50);

    m_mySeries.push_back(new Series(this, axisN));

    chart()->addSeries(m_mySeries.last());

    m_mySeries.last()->setName(name);
    m_mySeries.last()->setColor(color);
    m_mySeriesDubl.last()->setUseOpenGL(true);
    m_mySeries.last()->setUseOpenGL(true);

    m_mySeries.last()->attachAxis(m_yaxis[axisN]);
    m_mySeries.last()->attachAxis(m_xaxis);
}

void ChartView::addPoint(quint8 seriesN, qreal X, qreal Y)
{
    if (seriesN >= m_mySeries.count()) return;

    if (m_empty) {
        m_min_X = X; m_max_X = X;
        m_empty = false;
    } else {
        m_min_X = qMin(m_min_X, X);
        m_max_X = qMax(m_max_X, X);
    }

    if (m_maxRange != 0)
        m_min_X = qMax(m_min_X, m_max_X - m_maxRange);

    // 1) добавляем точку
    m_mySeries[seriesN]->append(X, Y);

    // 2) просим обновить оси таймером
    if (!m_zoomed && m_update)
        m_axesDirty = true;
}

void ChartView::duplicateChartSeries(quint8 seriesN)
{
    if (seriesN >= m_mySeries.count()) {
        return;
    }

    if (m_mySeries[seriesN]->points().empty()) {
        return;
    }

    m_mySeriesDubl[seriesN]->replace(m_mySeries.at(seriesN)->points());

    m_mySeries[seriesN]->clear();
}

void ChartView::clear()
{
    for (Series *mySerial : std::as_const(m_mySeries)) {
        mySerial->clear();
#if QT_VERSION >= QT_VERSION_CHECK(6,6,0)
        mySerial->clearPointsConfiguration();
#endif
    }

    for (Series *mySerial : std::as_const(m_mySeriesDubl)) {
        mySerial->clear();
    }

    m_draggableSnapshot.clear();

    m_empty = true;
    m_zoomed = false;

    m_xaxisValue->setRange(0, m_minR);
    m_xaxisTime->setRange(QDateTime::fromMSecsSinceEpoch(0),
                          QDateTime::fromMSecsSinceEpoch(m_minRTime));
}

void ChartView::visible(quint8 seriesN, bool visible)
{
    bool showaxis = visible;

    if (seriesN >= m_mySeries.count()) {
        return;
    }

    m_mySeries[seriesN]->setVisible(visible);

    m_mySeriesDubl[seriesN]->setVisible(visible);

    chart()->legend()->markers(m_mySeriesDubl[seriesN])[0]->setVisible(false);

    if (!visible)
        for (int i = 0; i < m_mySeries.count(); i++) {
            if (i == seriesN) {
                continue;
            }

            if (m_mySeries.at(i)->getAxisN() == m_mySeries.at(seriesN)->getAxisN()) {
                if (m_mySeries.at(i)->isVisible()) {
                    showaxis = true;
                    break;
                }
            }
        }

    m_yaxis[m_mySeries.at(seriesN)->getAxisN()]->setVisible(showaxis);

    autoScale(m_axisX_min, m_axisX_max);
}

void ChartView::showDots(bool show)
{
    for (int i = 0; i < m_mySeries.size(); ++i) {
        Series* s = m_mySeries[i];
        const bool keep = s->isMarkersOnly() ||
                          m_draggableSeries.contains(static_cast<quint8>(i));
        s->setPointsVisible(keep || show);
    }

    for (Series* s : std::as_const(m_mySeriesDubl)) {
        if (s->isMarkersOnly())
            s->setPointsVisible(true);
        else
            s->setPointsVisible(show);
    }
}

void ChartView::setSeriesDraggable(quint8 seriesN, bool draggable)
{
    if (draggable) {
        m_draggableSeries.insert(seriesN);
        if (seriesN < m_mySeries.size())
            m_mySeries[seriesN]->setPointsVisible(true);
    } else {
        m_draggableSeries.remove(seriesN);
    }
}

void ChartView::highlightBendPoints(quint8 seriesN)
{
#if QT_VERSION >= QT_VERSION_CHECK(6,6,0)
    if (seriesN >= m_mySeries.size()) return;

    Series* s = m_mySeries[seriesN];
    s->clearPointsConfiguration();

    const auto pts = s->points();

    // Крайние точки лежат на границе графика, "сгиб" — точки между ними
    for (int i = 1; i + 1 < pts.size(); ++i) {
        s->setPointConfiguration(i, QXYSeries::PointConfiguration::Size, 16.0);
        s->setPointConfiguration(i, QXYSeries::PointConfiguration::Color, QColor(255, 190, 0));
    }
#else
    Q_UNUSED(seriesN);
#endif
}

void ChartView::snapshotDraggableSeries()
{
    m_draggableSnapshot.clear();
    for (quint8 idx : std::as_const(m_draggableSeries)) {
        if (idx < m_mySeries.size()) {
            m_draggableSnapshot.insert(idx, m_mySeries.at(idx)->points());
            highlightBendPoints(idx);
        }
    }
}

void ChartView::restoreDraggableSeries()
{
    for (auto it = m_draggableSnapshot.cbegin(); it != m_draggableSnapshot.cend(); ++it) {
        if (it.key() < m_mySeries.size()) {
            m_mySeries[it.key()]->replace(it.value());
            highlightBendPoints(it.key());
        }
    }
}

int ChartView::findNearestPointIndex(QPoint mousePos, int* outSeriesN) const
{
    constexpr qreal kHitPx = 10.0;
    int nearestPt = -1, nearestSeries = -1;
    qreal minDist = kHitPx + 1.0;
    for (quint8 idx : m_draggableSeries) {
        if (idx >= m_mySeries.size()) continue;
        Series* s = m_mySeries.at(idx);
        if (!s->isVisible()) continue;
        const auto& pts = s->points();
        for (int i = 0; i < pts.size(); ++i) {
            const QPointF screenPt = chart()->mapToPosition(pts[i], s);
            const qreal dist = QLineF(QPointF(mousePos), screenPt).length();
            if (dist < minDist) {
                minDist = dist;
                nearestPt = i;
                nearestSeries = static_cast<int>(idx);
            }
        }
    }
    if (outSeriesN) *outSeriesN = nearestSeries;
    return nearestPt;
}

int ChartView::findNearestDraggableSeries(QPoint mousePos) const
{
    constexpr qreal kLinePx = 8.0;
    auto dot = [](QPointF a, QPointF b) { return a.x()*b.x() + a.y()*b.y(); };
    int result = -1;
    qreal minDist = kLinePx + 1.0;
    for (quint8 idx : m_draggableSeries) {
        if (idx >= m_mySeries.size()) continue;
        Series* s = m_mySeries.at(idx);
        if (!s->isVisible()) continue;
        const auto& pts = s->points();
        for (int i = 0; i + 1 < pts.size(); ++i) {
            const QPointF A = chart()->mapToPosition(pts[i], s);
            const QPointF B = chart()->mapToPosition(pts[i+1], s);
            const QPointF M = mousePos;
            const QPointF AB = B - A;
            const qreal len2 = dot(AB, AB);
            if (qFuzzyIsNull(len2)) continue;
            const qreal t = qBound(0.0, dot(M - A, AB) / len2, 1.0);
            const qreal dist = QLineF(M, A + t * AB).length();
            if (dist < minDist) {
                minDist = dist;
                result = static_cast<int>(idx);
            }
        }
    }
    return result;
}

void ChartView::zoomIn(qreal min, qreal max)
{
    min = qMax(min, m_min_X);
    max = qMin(max, m_max_X);

    if ((max - min) <= m_minRange) {
        min -= (m_minRange - max + min) / 2;
        max = min + m_minRange;
    } if (max > m_max_X) {
        min -= (max - m_max_X);
        max = m_max_X;
    } if (min < m_min_X) {
        max += (m_min_X - min);
        min = m_min_X;
    } if (m_xaxis == m_xaxisValue) {
        m_xaxis->setRange(min, max);
    } else {
        m_xaxis->setRange(QDateTime::fromMSecsSinceEpoch(min),
                          QDateTime::fromMSecsSinceEpoch(qCeil(max)));
    }

    m_axisX_min = min;
    m_axisX_max = max;

    m_zoomed = ((m_axisX_max < m_max_X) or (m_axisX_min > m_min_X));

    autoScale(min, max);
}

void ChartView::zoomOut()
{
    if (!m_zoomed) {
        return;
    }

    qreal min = m_axisX_min - (m_axisX_max - m_axisX_min) / 2;
    qreal max = m_axisX_max + (m_axisX_max - m_axisX_min) / 2;

    if ((max - min) > (m_max_X - m_min_X)) {
        m_zoomed = false;
        min = m_min_X;
        max = m_min_X + qMax(m_minRange, m_max_X - m_min_X);
    } if (max > m_max_X) {
        min -= (max - m_max_X);
        max = m_max_X;
    } if (min < m_min_X) {
        max += (m_min_X - min);
        min = m_min_X;
    } if (m_xaxis == m_xaxisValue) {
        min = qMax(qFloor(min), 0);
        max = qCeil(max);
        m_xaxis->setRange(min, max);
    } else {
        min = qMax(qFloor(min / 1000.0), 0) * 1000.0;
        max = qMax(min + 1000.0, qCeil(max / 1000.0) * 1000.0);
        m_xaxis->setRange(QDateTime::fromMSecsSinceEpoch(qCeil(min)),
                          QDateTime::fromMSecsSinceEpoch(qCeil(max)));
    }

    m_axisX_min = min;
    m_axisX_max = max;

    autoScale(min, max);
}

void ChartView::setName(QString name)
{
    m_name = name;
}

void ChartView::setMaxRange(qreal value)
{
    m_maxRange = value;
}

QString ChartView::getname() const
{
    return m_name;
}

QPair<QList<QPointF>, QList<QPointF>>
ChartView::getPoints(quint8 seriesN) const
{
    return qMakePair(m_mySeries.at(seriesN)->points(), m_mySeriesDubl.at(seriesN)->points());
}

void ChartView::saveToStream(QDataStream &stream) const
{
    stream << m_name;
    stream << (m_xaxis == m_xaxisValue);
    stream << m_xaxisValue->labelFormat();

    stream << m_yaxis.size();

    for (const QValueAxis *yaxis : m_yaxis) {
        stream << yaxis->labelFormat();
    }

    stream << m_mySeries.size();

    for (const auto mySerial : m_mySeries) {
        stream << mySerial->getAxisN();
        stream << mySerial->name();
        stream << mySerial->color();
    }

    for (const auto mySerial : m_mySeries) {
        stream << mySerial->points();
    }

    for (const auto mySerial : m_mySeriesDubl) {
        stream << mySerial->points();
    }

    stream << m_mySeries.at(0)->pointsVisible();
}

void ChartView::loadFromStream(QDataStream &dataStream)
{
    QString stream;
    dataStream >> stream;
    setName(stream);

    bool valueAxis;
    dataStream >> valueAxis;
    useTimeaxis(!valueAxis);

    dataStream >> stream;
    setLabelXformat(stream);

    qsizetype Y_size;
    dataStream >> Y_size;

    for (quint32 i = 0; i < Y_size; ++i) {
        dataStream >> stream;
        addAxis(stream);
    }

    qsizetype S_size;
    dataStream >> S_size;

    for (quint32 i = 0; i < S_size; ++i) {
        quint8 axisN;
        QString name;
        QColor color;
        dataStream >> axisN >> name >> color;
        addSeries(axisN, name, color);
    }

    const auto& mySeries = m_mySeries;
    for (const auto mySerial : mySeries) {
        QList<QPointF> points;
        dataStream >> points;
        mySerial->append(points);
    }

    const auto& mySeriesDubls = m_mySeriesDubl;
    for (const auto mySeriesDubl : mySeriesDubls) {
        QList<QPointF> points;
        dataStream >> points;
        mySeriesDubl->append(points);
    }

    autoUpdate(true);
}
}