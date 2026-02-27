#include "OptionTest.h"

OptionTest::OptionTest(QObject *parent, bool endTestAfterProcess)
    : Test(parent)
    , m_endTestAfterProcess(endTestAfterProcess)
{}

void OptionTest::Process()
{
    emit started();

    if (m_task.value.empty()) {
        emit EndTest();
        return;
    }
    emit SetStartTime();

    setDacBlocked(m_task.value.first(), 10000, true);

    if (m_terminate) { emit EndTest(); return; }

    m_graphTimer->start(50);

    for (const auto &value : m_task.value) {
        setDacBlocked(value, m_task.delay);

        if (m_terminate) {
            emit EndTest();
            return;
        }
    }
    if (m_endTestAfterProcess) {
        emit EndTest();
    }
}

void OptionTest::SetTask(Task task)
{
    m_task = task;
}
