#include "MainTest.h"

MainTest::MainTest(QObject *parent, bool endTestAfterProcess)
    : Test(parent)
    , m_endTestAfterProcess(endTestAfterProcess)
{}

void MainTest::Process()
{
    emit started();
    emit ClearGraph();
    emit ShowDots(!m_parameters.continuous);

    setDacBlocked(m_parameters.dac_min, 10000, true);

    m_graphTimer->start(m_parameters.delay);

    Sleep(m_parameters.delay);

    quint16 pointNumbers = m_parameters.pointNumbers * m_parameters.delay / m_parameters.response;

    quint64 time;
    time = QDateTime::currentMSecsSinceEpoch();

    for (qint16 i = 0; i <= pointNumbers; ++i) {
        quint16 dac = i * (m_parameters.dac_max - m_parameters.dac_min) / pointNumbers
                      + m_parameters.dac_min;

        time += m_parameters.response;
        quint64 currentTime = QDateTime::currentMSecsSinceEpoch();
        setDacBlocked(dac, time < currentTime ? 0 : (time - currentTime));

        if (m_terminate) {
            emit EndTest();
            return;
        }
    }

    setDacBlocked(m_parameters.dac_max, 0, true);

    emit DublSeries();

    Sleep(m_parameters.delay);

    time = QDateTime::currentMSecsSinceEpoch();

    for (qint16 i = pointNumbers; i >= 0; --i) {
        quint16 dac = i * (m_parameters.dac_max - m_parameters.dac_min) / pointNumbers
                      + m_parameters.dac_min;

        time += m_parameters.response;
        quint64 currentTime = QDateTime::currentMSecsSinceEpoch();
        setDacBlocked(dac, time < currentTime ? 0 : (time - currentTime));

        if (m_terminate) {
            emit EndTest();
            return;
        }
    }

    setDacBlocked(m_parameters.dac_min, 0, true);

    m_graphTimer->stop();

    if (m_terminate) {
        emit EndTest();
        return;
    }

    QVector<QVector<QPointF>> points;
    emit GetPoints(points);

    TestResults testResults;

    Limits regressionLimits = GetLimits(points[2], points[3]);
    Regression regressionForward = CalculateRegression(points[2], regressionLimits);
    Regression regressionBackward = CalculateRegression(points[3], regressionLimits);

    QVector<QPointF> pointsForward = GetRegressionPoints(regressionForward, regressionLimits);
    QVector<QPointF> pointsBackward = GetRegressionPoints(regressionBackward, regressionLimits);

    // Ошибка нелинейности характеристики "давление от перемещения", % диапазона
    qreal linErrForward  = GetLinearityError(points[2], regressionForward,  regressionLimits);
    qreal linErrBackward = GetLinearityError(points[3], regressionBackward, regressionLimits);


    // взять максимум из двух веток
    testResults.linearityError = qMax(linErrForward, linErrBackward);

    // если нужно "линейность", а не "ошибку":
    testResults.linearity = 100.0 - testResults.linearityError;

    pointsForward.append({pointsBackward.rbegin(), pointsBackward.rend()});
    pointsForward.push_back(pointsForward.first());

    emit AddRegression(pointsForward);

    QVector<QPointF> frictionPoints = GetFrictionPoints(points[2], points[3], regressionLimits);

    emit AddFriction(frictionPoints);


    double y_mean = (regressionLimits.maxY + regressionLimits.minY) / 2.0;

    testResults.pressureDiff = qAbs((y_mean - regressionForward.b) / regressionForward.k
                                      - (y_mean - regressionBackward.b) / regressionBackward.k);

    auto [mean, max] = GetMeanMax(points[0], points[1]);

    testResults.dynamicErrorMean = mean / 2;
    testResults.dynamicErrorMax = max;

    double range = qAbs((regressionLimits.minY - regressionForward.b) / regressionForward.k
                       - (regressionLimits.maxY - regressionForward.b) / regressionForward.k);

    testResults.friction = 50.0 * testResults.pressureDiff / range;

    auto [lowLimitPressure, highLimitPressure] = GetRangeLimits(regressionForward,
                                                  regressionBackward,
                                                  regressionLimits);

    testResults.lowLimitPressure = lowLimitPressure;
    testResults.highLimitPressure = highLimitPressure;

    auto [springLow, springHigh] = GetSpringLimits(regressionForward,
                                                     regressionBackward,
                                                     regressionLimits);

    testResults.springLow = springLow;
    testResults.springHigh = springHigh;

    emit Results(testResults);
    if (m_endTestAfterProcess) {
        emit EndTest();
    }
}

void MainTest::SetParameters(MainTestSettings::TestParameters &parameters)
{
    m_parameters = parameters;
}

MainTest::Regression MainTest::CalculateRegression(const QVector<QPointF> &points, Limits limits)
{
    const qreal range = 0.10;

    qreal minY = (limits.maxY - limits.minY) * range + limits.minY;
    qreal maxY = limits.maxY - (limits.maxY - limits.minY) * range;

    qreal xy = 0;
    qreal x = 0;
    qreal y = 0;
    qreal x2 = 0;
    quint16 n = 0;

    foreach (QPointF P, points) {
        if ((P.y() >= minY) && (P.y() <= maxY)) {
            n++;
            xy += P.x() * P.y();
            x += P.x();
            y += P.y();
            x2 += qPow(P.x(), 2);
        }
    }

    Regression result;

    result.k = (n * xy - x * y) / (n * x2 - x * x);
    result.b = (y - result.k * x) / n;

    return result;
}

qreal MainTest::GetLinearityError(const QVector<QPointF>& points,
                                  const Regression& regression,
                                  const Limits& limits)
{
    const qreal frac = 0.10;
    const qreal Pmin = limits.minY + (limits.maxY - limits.minY) * frac;
    const qreal Pmax = limits.maxY - (limits.maxY - limits.minY) * frac;
    const qreal Prange = Pmax - Pmin;
    if (Prange <= 0)
        return 0.0;

    qreal maxDiff = 0.0;

    for (const auto& P : points) {
        const qreal p = P.y();
        if (p < Pmin || p > Pmax)
            continue;

        const qreal p_lin = regression.k * P.x() + regression.b;
        const qreal diff = qAbs(p - p_lin);

        if (diff > maxDiff)
            maxDiff = diff;
    }

    return (maxDiff / Prange) * 100.0;
}

MainTest::Limits MainTest::GetLimits(const QVector<QPointF> &points1,
                                     const QVector<QPointF> &points2)
{
    Limits result;

    auto [minX1, maxX1] = std::minmax_element(points1.begin(),
                                              points1.end(),
                                              [](QPointF a, QPointF b) -> bool {
                                                  return a.x() < b.x();
                                              });

    auto [minX2, maxX2] = std::minmax_element(points2.begin(),
                                              points2.end(),
                                              [](QPointF a, QPointF b) -> bool {
                                                  return a.x() < b.x();
                                              });

    result.minX = qMin(minX1->x(), minX2->x());
    result.maxX = qMax(maxX1->x(), maxX2->x());

    auto [minY1, maxY1] = std::minmax_element(points1.begin(),
                                              points1.end(),
                                              [](QPointF a, QPointF b) -> bool {
                                                  return a.y() < b.y();
                                              });

    auto [minY2, maxY2] = std::minmax_element(points2.begin(),
                                              points2.end(),
                                              [](QPointF a, QPointF b) -> bool {
                                                  return a.y() < b.y();
                                              });

    result.minY = qMin(minY1->y(), minY2->y());
    result.maxY = qMax(maxY1->y(), maxY2->y());

    return result;
}

QVector<QPointF> MainTest::GetRegressionPoints(Regression regression, Limits limits)
{
    const QPointF point_minX(limits.minX, regression.k * limits.minX + regression.b);
    const QPointF point_maxX(limits.maxX, regression.k * limits.maxX + regression.b);
    const QPointF point_minY((limits.minY - regression.b) / regression.k, limits.minY);
    const QPointF point_maxY((limits.maxY - regression.b) / regression.k, limits.maxY);

    auto PointInLimits = [limits](const QPointF &point) {
        return (point.x() >= limits.minX) && (point.x() <= limits.maxX)
            && (point.y() >= limits.minY) && (point.y() <= limits.maxY);
    };

    bool minX_InLimits = PointInLimits(point_minX);
    bool maxX_InLimits = PointInLimits(point_maxX);
    bool minY_InLimits = PointInLimits(point_minY);
    bool maxY_InLimits = PointInLimits(point_maxY);

    QVector<QPointF> result;

    if (minX_InLimits && maxX_InLimits) {
        result.push_back(point_minX);
        result.push_back(point_maxX);
        return result;
    }

    if (minX_InLimits && minY_InLimits) {
        result.push_back(point_minX);
        result.push_back(point_minY);
        result.push_back({limits.maxX, limits.minY});
        return result;
    }

    if (minX_InLimits && maxY_InLimits) {
        result.push_back(point_minX);
        result.push_back(point_maxY);
        result.push_back({limits.maxX, limits.maxY});
        return result;
    }

    if (maxX_InLimits && minY_InLimits) {
        result.push_back({limits.minX, limits.minY});
        result.push_back(point_minY);
        result.push_back(point_maxX);
        return result;
    }

    if (maxX_InLimits && maxY_InLimits) {
        result.push_back({limits.minX, limits.maxY});
        result.push_back(point_maxY);
        result.push_back(point_maxX);
        return result;
    }

    if (minY_InLimits && maxY_InLimits) {
        if (point_minY.x() < point_maxY.x()) {
            result.push_back({limits.minX, limits.minY});
            result.push_back(point_minY);
            result.push_back(point_maxY);
            result.push_back({limits.maxX, limits.maxY});
        } else {
            result.push_back({limits.minX, limits.maxY});
            result.push_back(point_maxY);
            result.push_back(point_minY);
            result.push_back({limits.maxX, limits.minY});
        }
    }

    return result;
}

QVector<QPointF> MainTest::GetFrictionPoints(const QVector<QPointF> &pointsForward,
                                             const QVector<QPointF> &pointsBackward,
                                             Limits limits)
{
    const quint16 Sections = qMin(pointsForward.size(), pointsBackward.size()) / 3;

    qreal step = (limits.maxY - limits.minY) / Sections;

    QVector<quint16> pointsNumForward(Sections);
    QVector<quint16> pointsNumBackward(Sections);
    QVector<qreal> pointsValueForward(Sections);
    QVector<qreal> pointsValueBackward(Sections);

    for (auto point : pointsForward) {
        quint16 sectionNum = qFloor((point.y() - limits.minY) / step);
        sectionNum = qMin(sectionNum, quint16(Sections - 1));
        ++pointsNumForward[sectionNum];
        pointsValueForward[sectionNum] += point.x();
    }

    for (auto point : pointsBackward) {
        quint16 sectionNum = qFloor((point.y() - limits.minY) / step);
        sectionNum = qMin(sectionNum, quint16(Sections - 1));
        ++pointsNumBackward[sectionNum];
        pointsValueBackward[sectionNum] += point.x();
    }

    QVector<QPointF> result;

    for (quint16 i = Sections * 0.05; i < Sections * 0.95; ++i) {
        if (pointsNumForward[i] == 0 || pointsNumBackward[i] == 0)
            continue;
        result.push_back({step * i + limits.minY,
                          qAbs(pointsValueForward[i] / pointsNumForward[i]
                               - pointsValueBackward[i] / pointsNumBackward[i])});
    }

    return result;
}

QPair<qreal, qreal> MainTest::GetMeanMax(const QVector<QPointF> &pointsForward,
                                         const QVector<QPointF> &pointsBackward)
{
    Limits limits = GetLimits(pointsForward, pointsBackward);

    const quint16 Sections = qMin(pointsForward.size(), pointsBackward.size()) / 3;

    qreal step = (limits.maxY - limits.minY) / Sections;

    QVector<quint16> pointsNumForward(Sections);
    QVector<quint16> pointsNumBackward(Sections);
    QVector<qreal> pointsValueForward(Sections);
    QVector<qreal> pointsValueBackward(Sections);

    for (auto point : pointsForward) {
        quint16 sectionNum = qFloor((point.y() - limits.minY) / step);
        sectionNum = qMin(sectionNum, quint16(Sections - 1));
        ++pointsNumForward[sectionNum];
        pointsValueForward[sectionNum] += point.x();
    }

    for (auto point : pointsBackward) {
        quint16 sectionNum = qFloor((point.y() - limits.minY) / step);
        sectionNum = qMin(sectionNum, quint16(Sections - 1));
        ++pointsNumBackward[sectionNum];
        pointsValueBackward[sectionNum] += point.x();
    }

    qreal sum = 0;
    quint16 num = 0;
    qreal max = 0;

    for (quint16 i = Sections * 0.05; i < Sections * 0.95; ++i) {
        if (pointsNumForward[i] == 0 || pointsNumBackward[i] == 0)
            continue;

        qreal diff = qAbs(pointsValueForward[i] / pointsNumForward[i]
                          - pointsValueBackward[i] / pointsNumBackward[i]);

        sum += diff;
        ++num;
        max = qMax(max, diff);
    }

    return qMakePair(sum / num, max);
}

QPair<qreal, qreal> MainTest::GetRangeLimits(Regression regression1,
                                             Regression regression2,
                                             Limits limits)
{
    auto x_val = [](Regression regression, qreal y) { return (y - regression.b) / regression.k; };

    qreal x1 = x_val(regression1, limits.minY);
    qreal x2 = x_val(regression1, limits.maxY);
    qreal x3 = x_val(regression2, limits.minY);
    qreal x4 = x_val(regression2, limits.maxY);

    qreal min = qMin(qMin(x1, x2), qMin(x3, x4));
    qreal max = qMax(qMax(x1, x2), qMax(x3, x4));

    return qMakePair(min, max);
}

QPair<qreal, qreal> MainTest::GetSpringLimits(Regression regression1,
                                              Regression regression2,
                                              Limits limits)
{
    auto x_val = [](Regression regression, qreal y) { return (y - regression.b) / regression.k; };

    qreal x1 = x_val(regression1, limits.minY);
    qreal x2 = x_val(regression1, limits.maxY);
    qreal x3 = x_val(regression2, limits.minY);
    qreal x4 = x_val(regression2, limits.maxY);

    qreal min = (qMin(x1, x2) + qMin(x3, x4)) / 2;
    qreal max = (qMax(x1, x2) + qMax(x3, x4)) / 2;

    return qMakePair(min, max);
}
