#ifndef MYCHART_H
#define MYCHART_H

#pragma once
#include <QObject>
#include <QtCharts>
#include "./Src/CustomChart/MySeries.h"

class MyChart : public QChartView
{
    Q_OBJECT
private:
    QTimer m_axisTimer;
    QElapsedTimer m_markerTimer;

    bool m_axesDirty = false;
    bool m_pendingPointAdded = false;

    void updateAxes();               // new
    bool allowMarkerUpdate();  // new

    QString m_name;
    const qreal m_minR = 0.1;
    const qreal m_minRTime = 1000;
    qreal m_minRange;
    qreal m_maxRange = 0;
    QList<QValueAxis *> m_yaxis;

    QAbstractAxis *m_xaxis;
    QValueAxis *m_xaxisValue;
    QDateTimeAxis *m_xaxisTime;

    QPoint m_markersPos;
    QLineSeries m_marker_X;
    QLineSeries m_marker_Y;

    QList<MySeries *> m_mySeries;
    QList<MySeries *> m_mySeriesDubl;

    qreal m_min_X;
    qreal m_max_X;

    qreal m_axisX_min;
    qreal m_axisX_max;

    bool m_empty;
    bool m_zoomed;
    bool m_update;

    qreal m_X1;
    qreal m_X2;

    QGraphicsSimpleTextItem *m_coordItem = NULL;

    bool m_fixedXEnabled = false;
    qreal m_fixedXMin = 0;
    qreal m_fixedXMax = 0;
    QList<quint8> m_fixedXSeriesIdx;

    void drawMarkers(QPoint pos);

    void zoomIn(qreal min, qreal max);
    void zoomOut();

    void autoScale(qreal min, qreal max);

public:
    MyChart(QWidget *parent = nullptr);

    ~MyChart();
    QString getname() const;
    void setName(QString name);
    void setMaxRange(qreal value);
    QPair<QList<QPointF>, QList<QPointF>> getPoints(quint8 seriesN) const;
    void saveToStream(QDataStream &stream) const;
    void loadFromStream(QDataStream &dataStream);
    void setPointsVisible(quint8 seriesN, bool visible);
    QVector<MySeries*>& series();
    void setSeriesMarkersOnly(quint8 seriesN, bool on);

public slots:
    void useTimeaxis(bool);
    void addAxis(QString);
    void addSeries(quint8 axisN, QString name, QColor color);
    void addPoint(quint8 seriesN, qreal X, qreal Y);
    void duplicateChartSeries(quint8 seriesN);
    void clear();
    void visible(quint8 seriesN, bool visible);
    void showDots(bool show);
    void setLabelXformat(QString);
    void autoUpdate(bool);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
};

#endif // MYCHART_H
