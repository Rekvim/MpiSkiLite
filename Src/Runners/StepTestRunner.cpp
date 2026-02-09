#include "StepTestRunner.h"
#include "./Program.h"
#include "./Registry.h"

static QVector<quint16> buildSequence(const StepTestSettings::TestParameters& p,
                                      Mpi& mpi, bool normalOpen)
{
    QVector<quint16> seq;
    auto rawFromPct = [&](quint16 pct){
        const qreal cur = 16.0 * (normalOpen ? (100 - pct) : pct) / 100.0 + 4.0;
        return mpi.dac()->rawFromValue(cur);
    };

    const qreal start = 4.0, end = 20.0;
    seq.push_back(mpi.dac()->rawFromValue(start));
    for (auto v : p.points)  seq.push_back(rawFromPct(v));
    seq.push_back(mpi.dac()->rawFromValue(end));
    for (auto it = p.points.crbegin(); it != p.points.crend(); ++it)
        seq.push_back(rawFromPct(*it));
    seq.push_back(mpi.dac()->rawFromValue(start));
    return seq;
}

RunnerConfig StepTestRunner::buildConfig() {
    StepTestSettings::TestParameters p{};
    auto owner = qobject_cast<Program*>(parent()); Q_ASSERT(owner);
    emit owner->getParameters_stepTest(p);
    if (p.points.empty()) return {};

    const quint64 P = static_cast<quint64>(p.points.size());
    const quint64 delay = static_cast<quint64>(p.delay);
    const quint64 N_values = 3 + 2 * P;
    const quint64 totalMs = 10000ULL + N_values * delay;

    auto* worker = new StepTest;
    StepTest::Task task;
    task.delay = p.delay;

    const bool normalOpen = (m_reg.valveInfo().safePosition != 0);
    task.value = buildSequence(p, m_mpi, normalOpen);
    worker->SetTask(task);
    worker->Set_T_value(p.testValue);

    RunnerConfig cfg;
    cfg.worker = worker;
    cfg.totalMs = totalMs;
    cfg.chartToClear = static_cast<int>(Charts::Step);
    return cfg;
}

void StepTestRunner::wireSpecificSignals(Test& base) {
    auto& t = static_cast<StepTest&>(base);
    auto owner = qobject_cast<Program*>(parent());

    connect(&t, &StepTest::UpdateGraph,
            owner, [owner]{ owner->updateCharts_optionTest(Charts::Step); },
            Qt::QueuedConnection);

    connect(&t, &StepTest::GetPoints,
            owner, &Program::receivedPoints_stepTest,
            Qt::BlockingQueuedConnection);

    connect(&t, &StepTest::Results,
            owner, &Program::results_stepTest,
            Qt::QueuedConnection);

    connect(&t, &OptionTest::SetStartTime,
            owner, &Program::setTimeStart,
            Qt::QueuedConnection);
}
