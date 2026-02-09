#include "MyChart.h"

MyChart::MyChart(QWidget *parent)
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

    m_axisTimer.setInterval(100); // 10 Hz, можно 50..200
    connect(&m_axisTimer, &QTimer::timeout, this, &MyChart::updateAxes);
    m_axisTimer.start();

    m_markerTimer.start(); // для троттлинга маркеров
}

MyChart::~MyChart() {}

bool MyChart::allowMarkerUpdate()
{
    // ограничим до ~60 fps
    if (!m_markerTimer.isValid()) {
        m_markerTimer.start();
        return true;
    }
    if (m_markerTimer.elapsed() < 16)
        return false;
    m_markerTimer.restart();
    return true;
}

QVector<MySeries*>& MyChart::series()
{
    return m_mySeries;
}

void MyChart::updateAxes()
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

void MyChart::drawMarkers(QPoint pos)
{
    QSet<quint8> set;
    QList<MySeries *> mySeries;

    set.clear();

    for (MySeries *mySerial : qAsConst(m_mySeries)) {
        if (!set.contains(mySerial->getAxisN())) {
            if (mySerial->isVisible()) {
                set.insert(mySerial->getAxisN());
                mySeries.push_back(mySerial);
            }
        }
    }

    QPointF curVal = this->chart()->mapToValue(pos);

    QString format;
    QString coordStr = "";

    for (MySeries *mySerial : mySeries) {
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

void MyChart::setLabelXformat(QString format)
{
    m_xaxisValue->setLabelFormat(format);
}

void MyChart::autoScale(qreal min, qreal max)
{
    for (int ax = 0; ax < m_yaxis.count(); ax++) {
        bool first = true;
        qreal Ymin = 0;
        qreal Ymax = 0;

        foreach (const MySeries *mySerial, m_mySeries) {
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

        foreach (const MySeries *mySerial, m_mySeriesDubl) {
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

void MyChart::autoUpdate(bool update)
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

void MyChart::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        this->setRubberBand(QChartView::HorizontalRubberBand);
        QPointF curVal = this->chart()->mapToValue(event->pos());
        m_X1 = curVal.x();
    }

    QChartView::mousePressEvent(event);
}

void MyChart::mouseMoveEvent(QMouseEvent *event)
{
    if (m_mySeries.count()) {
        if (allowMarkerUpdate()) {
            m_coordItem->setVisible(true);
            m_markersPos = event->pos();
            drawMarkers(m_markersPos);
        }
    }
    QChartView::mouseMoveEvent(event);
}

void MyChart::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_mySeries.count()) {
        if (event->button() == Qt::LeftButton) {
            QPointF curVal = this->chart()->mapToValue(event->pos());
            m_X2 = curVal.x();

            zoomIn(qMin(m_X1, m_X2), qMax(m_X1, m_X2));
        }

        if (event->button() == Qt::RightButton) {
            zoomOut();
        }
    }

    this->setRubberBand(QChartView::NoRubberBand);
    QChartView::mouseReleaseEvent(event);
}

void MyChart::leaveEvent(QEvent *)
{
    m_coordItem->setVisible(false);
    m_marker_X.setVisible(false);
    m_marker_Y.setVisible(false);
}

void MyChart::useTimeaxis(bool useTime)
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

void MyChart::addAxis(QString format)
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

void MyChart::setSeriesMarkersOnly(quint8 seriesN, bool on)
{
    if (seriesN >= m_mySeries.size()) return;
    m_mySeries[seriesN]->setMarkersOnly(on);

    if (seriesN < m_mySeriesDubl.size())
        m_mySeriesDubl[seriesN]->setMarkersOnly(on);
}

void MyChart::addSeries(quint8 axisN, QString name, QColor color)
{
    if (axisN >= m_yaxis.count()) {
        return;
    }

    m_mySeriesDubl.emplace_back(new MySeries(this, axisN));

    chart()->addSeries(m_mySeriesDubl.last());

    m_mySeriesDubl.last()->setColor(color.lighter(170));

    m_mySeriesDubl.last()->attachAxis(m_yaxis[axisN]);
    m_mySeriesDubl.last()->attachAxis(m_xaxis);

    chart()->legend()->markers(m_mySeriesDubl.last())[0]->setVisible(false);

    // this->thread()->msleep(50);

    m_mySeries.push_back(new MySeries(this, axisN));

    chart()->addSeries(m_mySeries.last());

    m_mySeries.last()->setName(name);
    m_mySeries.last()->setColor(color);
    m_mySeriesDubl.last()->setUseOpenGL(true);
    m_mySeries.last()->setUseOpenGL(true);

    m_mySeries.last()->attachAxis(m_yaxis[axisN]);
    m_mySeries.last()->attachAxis(m_xaxis);
}

void MyChart::addPoint(quint8 seriesN, qreal X, qreal Y)
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

void MyChart::duplicateChartSeries(quint8 seriesN)
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

void MyChart::clear()
{
    for (MySeries *mySerial : m_mySeries) {
        mySerial->clear();
    }

    for (MySeries *mySerial : m_mySeriesDubl) {
        mySerial->clear();
    }

    m_empty = true;
    m_zoomed = false;

    m_xaxisValue->setRange(0, m_minR);
    m_xaxisTime->setRange(QDateTime::fromMSecsSinceEpoch(0),
                          QDateTime::fromMSecsSinceEpoch(m_minRTime));
}

void MyChart::visible(quint8 seriesN, bool visible)
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

void MyChart::showDots(bool show)
{
    for (MySeries *s : m_mySeries) {
        if (s->isMarkersOnly())
            s->setPointsVisible(true);
        else
            s->setPointsVisible(show);
    }

    for (MySeries *s : m_mySeriesDubl) {
        if (s->isMarkersOnly())
            s->setPointsVisible(true);
        else
            s->setPointsVisible(show);
    }
}

void MyChart::zoomIn(qreal min, qreal max)
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

void MyChart::zoomOut()
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

void MyChart::setName(QString name)
{
    m_name = name;
}

void MyChart::setMaxRange(qreal value)
{
    m_maxRange = value;
}

QString MyChart::getname() const
{
    return m_name;
}

QPair<QList<QPointF>, QList<QPointF>> MyChart::getPoints(quint8 seriesN) const
{
    return qMakePair(m_mySeries.at(seriesN)->points(), m_mySeriesDubl.at(seriesN)->points());
}

void MyChart::saveToStream(QDataStream &stream) const
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

void MyChart::loadFromStream(QDataStream &dataStream)
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

    for (const auto mySerial : m_mySeries) {
        QList<QPointF> points;
        dataStream >> points;
        mySerial->append(points);
    }

    for (const auto mySerial : m_mySeriesDubl) {
        QList<QPointF> points;
        dataStream >> points;
        mySerial->append(points);
    }

    autoUpdate(true);
}
