#ifndef STROKETEST_H
#define STROKETEST_H

#pragma once
#include <QPointF>
#include <QTimer>
#include <QDateTime>
#include "Test.h"

class StrokeTest : public Test
{
    Q_OBJECT

public:
    explicit StrokeTest(QObject *parent = nullptr);
    void Process() override;
signals:
    void GetPoints(QVector<QVector<QPointF>> &points);

    void SetStartTime();
    void Results(quint64 forwardTime, quint64 backwardTime);
};

#endif // STROKETEST_H
