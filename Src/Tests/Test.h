#ifndef TEST_H
#define TEST_H

#pragma once
#include <QEventLoop>
#include <QObject>
#include <QPointF>
#include <QTimer>

class Test : public QObject
{
    Q_OBJECT
public:
    explicit Test(QObject *parent = nullptr);
    virtual ~Test() = default;

protected:
    QTimer *m_graphTimer;
    QEventLoop *m_eventLoop;
    bool m_terminate;

    void Sleep(quint16 msecs);
    void setDacBlocked(quint16 value,
                       quint32 sleepMs = 0,
                       bool waitForStop = false,
                       bool waitForStart = false);

public slots:
    virtual void Process() = 0;
    void StoppingTheTest();
    void ReleaseBlock();

signals:
    void setDac(quint16 value,
                quint32 sleepMs = 0,
                bool waitForStop = false,
                bool waitForStart = false);
    void UpdateGraph();
    void EndTest();
};
#endif // TEST_H
