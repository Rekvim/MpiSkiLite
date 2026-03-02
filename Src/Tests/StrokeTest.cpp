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

static int findLastAtLevelBeforeLeaving(const QVector<QPointF>& pts,
                                        int startIdx,
                                        double levelThr,
                                        bool atOrBelowLevel,
                                        int minStable = 3)
{
    // Ищем "последнюю точку на уровне" перед тем, как сигнал уйдёт с уровня.
    // minStable: чтобы не ловить одиночный шум.

    auto isAtLevel = [&](double y) {
        return atOrBelowLevel ? (y <= levelThr) : (y >= levelThr);
    };
    auto isLeaving = [&](double y) {
        return atOrBelowLevel ? (y > levelThr) : (y < levelThr);
    };

    int lastAt = -1;
    int stable = 0;

    for (int i = qMax(0, startIdx); i < pts.size(); ++i) {
        const double y = pts[i].y();

        if (isAtLevel(y)) {
            lastAt = i;
            stable = qMin(stable + 1, minStable);
            continue;
        }

        if (lastAt != -1 && stable >= minStable && isLeaving(y)) {
            return lastAt;
        }

        // шум/дрожание: сбрасываем устойчивость, но lastAt оставляем
        stable = 0;
    }

    return lastAt; // если так и не "вышли", берём последний найденный
}

static int findFirstReachLevel(const QVector<QPointF>& pts,
                               int startIdx,
                               double levelThr,
                               bool reachAtOrBelow)
{
    // Ищем первую точку достижения целевого уровня.
    auto reached = [&](double y) {
        return reachAtOrBelow ? (y <= levelThr) : (y >= levelThr);
    };

    for (int i = qMax(0, startIdx); i < pts.size(); ++i) {
        if (reached(pts[i].y()))
            return i;
    }
    return -1;
}

static quint64 dtMs(const QVector<QPointF>& pts, int a, int b)
{
    if (a < 0 || b < 0 || b <= a) return 0;
    return static_cast<quint64>(pts[b].x() - pts[a].x());
}

// ======================================================================

StrokeTest::StrokeTest(QObject *parent)
    : Test(parent)
{}

void StrokeTest::Process()
{
    emit started();
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

    QVector<QVector<QPointF>> allPoints;
    emit GetPoints(allPoints);

    if (allPoints.isEmpty() || allPoints[0].isEmpty()) {
        qDebug() << "[StrokeTest] No graph points";
        emit Results(0, 0);
        emit EndTest();
        return;
    }

    const auto& points = allPoints[0];

    double minY = points[0].y();
    double maxY = points[0].y();

    for (const auto& p : points) {
        if (p.y() < minY) minY = p.y();
        if (p.y() > maxY) maxY = p.y();
    }

    const double lowThr  = minY + (maxY - minY) * 0.0050;
    const double highThr = maxY - (maxY - minY) * 0.0050;

    const bool normalClosed = m_cfg.normalClosed;

    const double forwardStartLevel = normalClosed ? lowThr  : highThr;
    const bool forwardStartIsLow = normalClosed ? true : false;
    const double forwardEndLevel   = normalClosed ? highThr : lowThr;
    const bool forwardEndIsLow = normalClosed ? false : true;

    const double backwardStartLevel = forwardEndLevel;
    const bool backwardStartIsLow = forwardEndIsLow;

    const double backwardEndLevel   = forwardStartLevel;
    const bool backwardEndIsLow   = forwardStartIsLow;

    int forwardStart  = findLastAtLevelBeforeLeaving(points, 0, forwardStartLevel, /*atOrBelowLevel=*/forwardStartIsLow);
    int forwardEnd = findFirstReachLevel(points, qMax(0, forwardStart), forwardEndLevel, /*reachAtOrBelow=*/forwardEndIsLow);

    int backwardStart = findLastAtLevelBeforeLeaving(points, qMax(0, forwardEnd), backwardStartLevel, /*atOrBelowLevel=*/backwardStartIsLow);
    int backwardEnd = findFirstReachLevel(points, qMax(0, backwardStart), backwardEndLevel, /*reachAtOrBelow=*/backwardEndIsLow);

    if (forwardStart < 0 || forwardEnd < 0 ||
        backwardStart < 0 || backwardEnd < 0)
    {
        qDebug() << "[StrokeTest] Could not detect full stroke movement";
        emit Results(0, 0);
        emit EndTest();
        return;
    }

    quint64 forwardTime  = dtMs(points, forwardStart, forwardEnd);
    quint64 backwardTime = dtMs(points, backwardStart, backwardEnd);

    qDebug().noquote()
        << "\n========== STROKE TEST ANALYSIS ==========\n"
        << "Mode:" << (normalClosed ? "NormalClosed (low->high->low)" : "NormalOpen (high->low->high)")
        << "\nTotal points:" << points.size()
        << "\nY range:"
        << "\n  minY =" << minY
        << "\n  maxY =" << maxY
        << "\nThresholds:"
        << "\n  lowThr  =" << lowThr
        << "\n  highThr =" << highThr

        << "\n\n--- FORWARD STROKE ---"
        << "\nStart:"
        << "\n  " << pointStr(points, forwardStart)
        << "\nEnd:"
        << "\n  " << pointStr(points, forwardEnd)

        << "\n\n--- BACKWARD STROKE ---"
        << "\nStart:"
        << "\n  " << pointStr(points, backwardStart)
        << "\nEnd:"
        << "\n  " << pointStr(points, backwardEnd);

    emit Results(forwardTime, backwardTime);
    emit EndTest();
}
