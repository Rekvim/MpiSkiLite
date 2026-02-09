#include "StepTest.h"

StepTest::StepTest(QObject *parent)
    : OptionTest(parent, false)
{}

void StepTest::Set_T_value(quint32 T_value)
{
    m_TValue = T_value;
}

QVector<StepTest::TestResult> StepTest::CalculateResult(const QVector<QVector<QPointF>> &points) const
{
    QVector<TestResult> result;
    const QVector<QPointF> &line = points.at(0);
    const QVector<QPointF> &task = points.at(1);

    qreal from = 0;
    qreal prevTask = task.first().y();
    qreal timeStart;
    quint32 t86Time = 0;
    qreal threshold;
    qreal overshoot = 0;
    bool t86Have = false;
    bool first = true;
    bool up;
    for (int i = 0; i < line.size(); ++i) {
        qreal currTask = task.at(i).y();
        if (!qFuzzyCompare(currTask, prevTask)) {
            if (first) {
                first = false;
            } else {
                result.push_back({static_cast<quint16>(qRound(from)),
                                  static_cast<quint16>(qRound(prevTask)),
                                  t86Have ? t86Time : 0,
                                  overshoot});
            }
            from = prevTask;
            timeStart = task.at(i).x();
            threshold = (currTask - prevTask) * m_TValue / 100 + prevTask;
            up = (currTask > prevTask);
            overshoot = -1000;
            t86Have = false;
            prevTask = currTask;
        } else {
            if (first) {
                continue;
            }
            overshoot = qMax(overshoot, (line.at(i).y() - currTask) / (currTask - from) * 100);
            if (!t86Have) {
                if ((line.at(i).y() - threshold) * (up ? 1 : -1) > 0) {
                    t86Time = qRound(line.at(i).x() - timeStart);
                    t86Have = true;
                }
            }
        }
    }
    result.push_back({static_cast<quint16>(qRound(from)),
                      static_cast<quint16>(qRound(prevTask)),
                      t86Have ? t86Time : 0,
                      overshoot});
    return result;
}

void StepTest::Process()
{
    emit SetStartTime();

    OptionTest::Process();
    if (m_terminate) {
        emit EndTest();
        return;
    }
    QVector<QVector<QPointF>> points;
    emit GetPoints(points);
    emit Results(CalculateResult(points), m_TValue);
    emit EndTest();
}
