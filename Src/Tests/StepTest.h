#ifndef STEPTEST_H
#define STEPTEST_H

#pragma once
#include <QEventLoop>
#include <QObject>
#include <QPointF>
#include <QTimer>
#include <QDateTime>

#include "OptionTest.h"

class StepTest : public OptionTest
{
    Q_OBJECT

public:
    explicit StepTest(QObject *parent = nullptr);
    void Process() override;

    struct TestResult
    {
        quint16 from;
        quint16 to;
        quint32 T_value;
        qreal overshoot;
    };

    void Set_T_value(quint32 T_value);

private:
    QVector<TestResult> CalculateResult(const QVector<QVector<QPointF>> &points) const;
    quint32 m_TValue;
signals:
    void GetPoints(QVector<QVector<QPointF>> &points);
    void Results(QVector<TestResult> result, quint32 T_value);
};

#endif // STEPTEST_H
