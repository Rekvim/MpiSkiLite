#include "Test.h"

Test::Test(QObject *parent)
    : QObject{parent}
{
    m_graphTimer = new QTimer(this);
    connect(m_graphTimer, &QTimer::timeout, this, [&] { emit UpdateGraph(); });
    m_terminate = false;
    m_eventLoop = new QEventLoop(this);
}

void Test::Sleep(quint16 msecs)
{
    QEventLoop loop(this);
    QTimer::singleShot(msecs, &loop, &QEventLoop::quit);
    loop.exec();
}

void Test::setDacBlocked(quint16 value, quint32 sleepMs, bool waitForStop, bool waitForStart)
{
    emit setDac(value, sleepMs, waitForStop, waitForStart);
    m_eventLoop->exec();
}

void Test::StoppingTheTest()
{
    m_terminate = true;
    m_eventLoop->quit();
    m_graphTimer->stop();

    emit EndTest();
}

void Test::ReleaseBlock()
{
    m_eventLoop->quit();
}
