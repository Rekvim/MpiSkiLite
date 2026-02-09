#include "OptionResolutionRunner.h"
#include "./Program.h"
#include "./Registry.h"
#include "./Src/Tests/OptionTest.h"

RunnerConfig OptionResolutionRunner::buildConfig() {
    OtherTestSettings::TestParameters p{};
    emit getParameters_resolutionTest(p);
    if (p.points.empty()) return {};

    auto* worker = new OptionTest;
    OptionTest::Task task;
    task.delay = p.delay;

    const bool normalOpen = (m_reg.valveInfo().safePosition != 0);

    task.value.push_back(m_mpi.dac()->rawFromValue(4.0));

    for (auto it = p.points.begin(); it != p.points.end(); ++it) {
        qreal current = 16.0 * (normalOpen ? 100 - *it : *it) / 100 + 4.0;
        qreal dacValue = m_mpi.dac()->rawFromValue(current);

        task.value.push_back(dacValue);

        for (auto it_s = p.steps.begin(); it_s < p.steps.end(); ++it_s) {
            current = (16 * (normalOpen ? 100 - *it - *it_s : *it + *it_s) / 100 + 4.0);
            qreal dacValueStep = m_mpi.dac()->rawFromValue(current);
            task.value.push_back(dacValueStep);

            task.value.push_back(dacValue);
        }
    }

    task.value.push_back(m_mpi.dac()->rawFromValue(4.0));
    worker->SetTask(task);

    // Тайминги
    const quint64 P = static_cast<quint64>(p.points.size());
    const quint64 S = static_cast<quint64>(p.steps.size());
    const quint64 delay = static_cast<quint64>(p.delay);
    const quint64 N_values = 1ULL + P * (2ULL * S + 1ULL);
    const quint64 totalMs  = 10000ULL + N_values * delay + 10000ULL;

    RunnerConfig cfg;
    cfg.worker = worker;
    cfg.totalMs = totalMs;
    cfg.chartToClear = static_cast<int>(Charts::Resolution);
    return cfg;
}

void OptionResolutionRunner::wireSpecificSignals(Test& base) {
    auto& t = static_cast<OptionTest&>(base);
    auto* owner = qobject_cast<Program*>(parent()); Q_ASSERT(owner);

    connect(&t, &OptionTest::UpdateGraph,
            owner, [owner]{ owner->updateCharts_optionTest(Charts::Resolution); },
            Qt::QueuedConnection);

    connect(&t, &OptionTest::SetStartTime,
            owner, &Program::setTimeStart,
            Qt::QueuedConnection);
}
