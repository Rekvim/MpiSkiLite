#ifndef OPTIONTEST_H
#define OPTIONTEST_H

#pragma once
#include <QEventLoop>
#include <QObject>
#include <QPointF>
#include <QTimer>
#include <QDateTime>

#include "Test.h"

class OptionTest : public Test
{
    Q_OBJECT

public:
    explicit OptionTest(QObject *parent = nullptr, bool endTestAfterProcess = true);
    virtual void Process() override;

    struct Task
    {
        QVector<quint16> value;
        quint32 delay;
    };

    void SetTask(Task task);

private:
    Task m_task;
    const bool m_endTestAfterProcess;
signals:
    void SetStartTime();
};

#endif // OPTIONTEST_H
