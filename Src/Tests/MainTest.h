#ifndef MAINTEST_H
#define MAINTEST_H

#pragma once
#include <QEventLoop>
#include <QObject>
#include <QPointF>
#include <QTimer>
#include <QDateTime>

#include "Test.h"
#include "./Src/Ui/TestSettings/MainTestSettings.h"

class MainTest : public Test
{
    Q_OBJECT
public:
    explicit MainTest(QObject *parent = nullptr, bool endTestAfterProcess = true);
    virtual void Process() override;
    void SetParameters(MainTestSettings::TestParameters &parameters);

    struct TestResults
    {
        double pressureDiff;
        double friction;
        double dynamicErrorMean;
        double dynamicErrorMax;
        double lowLimitPressure;
        double highLimitPressure;
        double springLow;
        double springHigh;

        double linearityError;
        double linearity;
    };

protected:
    MainTestSettings::TestParameters m_parameters;

private:
    const bool m_endTestAfterProcess;
    struct Regression
    {
        double k;
        double b;
    };

    struct Limits
    {
        double minX;
        double maxX;
        double minY;
        double maxY;
    };

    Regression CalculateRegression(const QVector<QPointF> &points, Limits limits);

    Limits GetLimits(const QVector<QPointF> &points1, const QVector<QPointF> &points2);

    QVector<QPointF> GetRegressionPoints(Regression regression, Limits limits);

    double GetLinearityError(const QVector<QPointF>& points,
                                      const Regression& regression,
                                      const Limits& limits);

    QVector<QPointF> GetFrictionPoints(const QVector<QPointF> &points1,
                                       const QVector<QPointF> &points2,
                                       Limits limits);

    QPair<double, double> GetMeanMax(const QVector<QPointF> &points_forward,
                                   const QVector<QPointF> &points_backward);

    QPair<double, double> GetRangeLimits(Regression regression1,
                                       Regression regression2,
                                       Limits limits);
    QPair<double, double> GetSpringLimits(Regression regression1,
                                        Regression regression2,
                                        Limits limits);
signals:
    void RequestSensorRawValue(quint16 &value);
    void DublSeries();
    void GetPoints(QVector<QVector<QPointF>> &points);
    void AddRegression(const QVector<QPointF> &points);
    void AddFriction(const QVector<QPointF> &points);
    void Results(TestResults results);
    void ShowDots(bool visible);
    void ClearGraph();
};

#endif // MAINTEST_H
