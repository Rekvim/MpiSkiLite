#pragma once

#include <QtCharts>

#include "Widgets/Chart/Series.h"

namespace Widgets::Chart {
class ChartView : public QChartView
{
    Q_OBJECT

public:
    ChartView(QWidget *parent = nullptr);
    ~ChartView()  = default;

    QString getname() const;
    void setName(QString name);
    void setMaxRange(qreal value);
    QPair<QList<QPointF>, QList<QPointF>> getPoints(quint8 seriesN) const;
    void saveToStream(QDataStream &stream) const;
    void loadFromStream(QDataStream &dataStream);
    void setPointsVisible(quint8 seriesN, bool visible);
    QVector<Series*>& series();
    void setSeriesMarkersOnly(quint8 seriesN, bool on);
    int seriesCount() const;
    bool isSeriesVisible(int series) const;
    void setSeriesDraggable(quint8 seriesN, bool draggable);
    void snapshotDraggableSeries();
    void restoreDraggableSeries();

signals:
    void seriesDragged(quint8 seriesN, QList<QPointF> points);

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

private:
    struct DragState {
        bool active = false;
        quint8 seriesN = 0;
        int pointIdx = -1;
        QPointF prevValue;
    };

    QSet<quint8> m_draggableSeries;
    DragState m_drag;
    QHash<quint8, QList<QPointF>> m_draggableSnapshot;

    int findNearestPointIndex(QPoint mousePos, int* outSeriesN) const;
    int findNearestDraggableSeries(QPoint mousePos) const;
    void highlightBendPoints(quint8 seriesN);


    QTimer m_axisTimer;
    QElapsedTimer m_markerTimer;
    QString m_xMarkerFormat = QStringLiteral("%1");
    QList<QString> m_yMarkerFormats;


    bool m_axesDirty = false;
    bool m_pendingPointAdded = false;

    void updateAxes();
    bool allowMarkerUpdate();

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

    QList<Series*> m_mySeries;
    QList<Series*> m_mySeriesDubl;

    qreal m_min_X;
    qreal m_max_X;

    qreal m_axisX_min;
    qreal m_axisX_max;

    bool m_empty;
    bool m_zoomed;
    bool m_update;

    qreal m_X1;
    qreal m_X2;

    QGraphicsSimpleTextItem* m_coordItem = NULL;

    bool m_fixedXEnabled = false;
    qreal m_fixedXMin = 0;
    qreal m_fixedXMax = 0;
    QList<quint8> m_fixedXSeriesIdx;

    void drawMarkers(QPoint pos);

    void zoomIn(qreal min, qreal max);
    void zoomOut();

    void autoScale(qreal min, qreal max);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
};
}
