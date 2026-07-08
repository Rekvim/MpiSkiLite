#pragma once

#include <QVector>
#include <QPair>

#include <QtMath>
#include <cmath>

#include "Result.h"
#include "Domain/Tests/IAnalyzer.h"

namespace Domain::Tests::Main {

class Analyzer : public IAnalyzer
{
public:
    struct Config
    {
        qreal driveDiameter = 0.0;
    };

private:
    enum class StrokeDirection
    {
        Forward,
        Backward
    };

    struct LinearPoint
    {
        double dac = 0.0;
        double position = 0.0;
    };

    struct PressurePoint
    {
        double pressure = 0.0;  // X
        double position = 0.0;  // Y
    };

    struct Limits
    {
        double minX = 0.0;
        double maxX = 0.0;
        double minY = 0.0;
        double maxY = 0.0;
    };

    struct Regression
    {
        double k = 0.0;
        double b = 0.0;
        bool valid = false;
    };

public:
    void setConfig(const Config& cfg);

    void start() override;
    void onSample(const Domain::Measurement::Sample& s) override;
    void finish() override;

    const Result& result() const;

    void startBackwardStroke();

    const QVector<QPointF>& regressionChartPointsForward() const;
    const QVector<QPointF>& regressionChartPointsBackward() const;
    const QVector<QPointF>& frictionChartPoints() const;

private:
    Config m_cfg;
    Result m_result;

    StrokeDirection m_direction = StrokeDirection::Forward;

    QVector<LinearPoint> m_linearSeriesFirst;
    QVector<LinearPoint> m_linearSeriesSecond;

    QVector<PressurePoint> m_pressureSeriesFirst;
    QVector<PressurePoint> m_pressureSeriesSecond;

    QVector<QPointF> m_regressionChartPointsForward;
    QVector<QPointF> m_regressionChartPointsBackward;
    QVector<QPointF> m_frictionChartPoints;

private:
    static bool isValidSample(const Domain::Measurement::Sample& s);

    Limits computePressureLimits(
        const QVector<PressurePoint>& forward,
        const QVector<PressurePoint>& backward) const;

    Limits computeTaskLimits(
        const QVector<LinearPoint>& forward,
        const QVector<LinearPoint>& backward) const;

    Regression calculateRegression(
        const QVector<PressurePoint>& points,
        const Limits& limits) const;

    double computeLinearityError(
        const QVector<PressurePoint>& points,
        const Regression& regression,
        const Limits& limits) const;

    double computePressureDiff(
        const Regression& regressionForward,
        const Regression& regressionBackward,
        const Limits& limits) const;

    double computeFrictionPercent(
        const Regression& regressionForward,
        const Limits& limits,
        double pressureDiff) const;

    QPair<double, double> computeDynamicErrorMeanMax(
        const QVector<LinearPoint>& forward,
        const QVector<LinearPoint>& backward) const;

    QPair<double, double> computeRangeLimits(
        const Regression& regressionForward,
        const Regression& regressionBackward,
        const Limits& limits) const;

    QPair<double, double> computeSpringLimits(
        const Regression& regressionForward,
        const Regression& regressionBackward,
        const Limits& limits) const;

    QVector<QPointF> buildRegressionLinePoints(
        const Regression& regression,
        const Limits& limits) const;

    QVector<QPointF> buildFrictionChartPoints(
        const QVector<PressurePoint>& first,
        const QVector<PressurePoint>& second,
        const Limits& limits) const;

    QVector<QPointF> toPressureQPoints(
        const QVector<PressurePoint>& points) const;
};

}