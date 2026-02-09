#include "StrokeTest.h"

#include <QDateTime>
#include <QVector>
#include <QPointF>
#include <QDebug>

static QString formatMs(quint64 ms)
{
    quint64 minutes = ms / 60000;
    quint64 seconds = (ms % 60000) / 1000;
    quint64 millis  = ms % 1000;

    return QString("%1:%2.%3")
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0'))
        .arg(millis,  3, 10, QChar('0'));
}
// ================== helper для читаемого вывода точки ==================
static QString pointStr(const QVector<QPointF>& pts, int idx)
{
    if (idx < 0 || idx >= pts.size()) {
        return QString("index=%1 (out of range)").arg(idx);
    }

    quint64 t = static_cast<quint64>(pts[idx].x());

    return QString("index=%1 | t=%2 | y=%3")
        .arg(idx)
        .arg(formatMs(t))
        .arg(pts[idx].y(), 0, 'f', 2);
}

// ======================================================================

StrokeTest::StrokeTest(QObject *parent)
    : Test(parent)
{}

void StrokeTest::Process()
{
    setDacBlocked(0, 10000, true);

    if (m_terminate) {
        emit EndTest();
        return;
    }

    emit SetStartTime();
    m_graphTimer->start(50);

    // Даём графику накопиться
    Sleep(5000);

    if (m_terminate) {
        emit EndTest();
        return;
    }

    // Прямой ход
    setDacBlocked(0xFFFF, 0, true, true);

    if (m_terminate) {
        emit EndTest();
        return;
    }

    setDacBlocked(0, 0, true, true);
    m_graphTimer->stop();

    // ==========================================================
    // Получаем точки графика
    // ==========================================================
    QVector<QVector<QPointF>> allPoints;
    emit GetPoints(allPoints);

    if (allPoints.isEmpty() || allPoints[0].isEmpty()) {
        qDebug() << "[StrokeTest] No graph points";
        emit Results(0, 0);
        emit EndTest();
        return;
    }

    const auto& points = allPoints[0];

    // ==========================================================
    // Поиск min / max
    // ==========================================================
    double minY = points[0].y();
    double maxY = points[0].y();

    for (const auto& p : points) {
        if (p.y() < minY) minY = p.y();
        if (p.y() > maxY) maxY = p.y();
    }

    const double zeroThreshold = minY + (maxY - minY) * 0.0050;
    const double maxThreshold  = maxY - (maxY - minY) * 0.0050;

    // ==========================================================
    // Поиск характерных точек
    // ==========================================================
    int forwardStart = 0;
    int forwardEnd    = 0;
    int backwardStart = 0;
    int backwardEnd   = 0;

    // -------- Прямой ход: последний ноль перед ростом --------
    bool wasAtZero = false;
    int lastZeroIndex = -1;

    for (int i = 0; i < points.size() - 1; ++i) {
        if (points[i].y() <= zeroThreshold) {
            lastZeroIndex = i;
            wasAtZero = true;
        } else if (wasAtZero && points[i].y() > zeroThreshold) {
            forwardStart = lastZeroIndex;
            break;
        }
    }

    if (forwardStart == 0 && lastZeroIndex != -1) {
        forwardStart = lastZeroIndex;
    }

    // -------- Достижение максимума --------
    for (int i = forwardStart; i < points.size(); ++i) {
        if (points[i].y() >= maxThreshold) {
            forwardEnd = i;
            break;
        }
    }

    // -------- Обратный ход: последний максимум перед спадом --------
    bool wasAtMax = false;
    int lastMaxIndex = -1;

    for (int i = forwardEnd; i < points.size() - 1; ++i) {
        if (points[i].y() >= maxThreshold) {
            lastMaxIndex = i;
            wasAtMax = true;
        } else if (wasAtMax && points[i].y() < maxThreshold) {
            backwardStart = lastMaxIndex;
            break;
        }
    }

    if (backwardStart == 0 && lastMaxIndex != -1) {
        backwardStart = lastMaxIndex;
    }

    // -------- Возврат к нулю --------
    for (int i = backwardStart; i < points.size(); ++i) {
        if (points[i].y() <= zeroThreshold) {
            backwardEnd = i;
            break;
        }
    }

    // ==========================================================
    // ЧЕЛОВЕЧЕСКИЙ ВЫВОД
    // ==========================================================
    qDebug().noquote()
        << "\n========== STROKE TEST ANALYSIS ==========\n"
        << "Total points:" << points.size()
        << "\nY range:"
        << "\n  minY =" << minY
        << "\n  maxY =" << maxY
        << "\nThresholds:"
        << "\n  Zero threshold =" << zeroThreshold
        << "\n  Max  threshold =" << maxThreshold

        << "\n\n--- FORWARD STROKE ---"
        << "\nStart (last zero before rise):"
        << "\n  " << pointStr(points, forwardStart)
        << "\nEnd (reach max threshold):"
        << "\n  " << pointStr(points, forwardEnd)

        << "\n\n--- BACKWARD STROKE ---"
        << "\nStart (last max before fall):"
        << "\n  " << pointStr(points, backwardStart)
        << "\nEnd (return to zero):"
        << "\n  " << pointStr(points, backwardEnd);

    // ==========================================================
    // Время движения
    // ==========================================================
    quint64 forwardTime  = 0;
    quint64 backwardTime = 0;

    if (forwardStart > 0 && forwardEnd > forwardStart) {
        forwardTime =
            static_cast<quint64>(points[forwardEnd].x() -
                                 points[forwardStart].x());
    }

    if (backwardStart > 0 && backwardEnd > backwardStart) {
        backwardTime =
            static_cast<quint64>(points[backwardEnd].x() -
                                 points[backwardStart].x());
    }

    qDebug().noquote()
        << "\n--- RESULT ---"
        << "\nForward time : " << formatMs(forwardTime)
        << "\nBackward time: " << formatMs(backwardTime)
        << "\n=========================================\n";

    emit Results(forwardTime, backwardTime);
    emit EndTest();
}
