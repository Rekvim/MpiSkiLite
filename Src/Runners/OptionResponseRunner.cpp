#include "OptionResponseRunner.h"
#include "./Program.h"
#include "./Registry.h"
#include "./Src/Tests/OptionTest.h"

RunnerConfig OptionResponseRunner::buildConfig()
{
    OtherTestSettings::TestParameters p{};
    emit getParameters_responseTest(p);
    if (p.points.empty())
        return {};

    auto* worker = new OptionTest;
    OptionTest::Task task;
    task.delay = p.delay;

    const bool normalOpen = (m_reg.valveInfo().safePosition != 0);

    task.value.push_back(m_mpi.dac()->rawFromValue(4.0));

    for (auto it = p.points.begin(); it != p.points.end(); ++it)
    {
        // Базовая точка (инверсия только здесь!)
        const qreal basePercent = normalOpen ? (100.0 - *it) : *it;
        const qreal baseCurrent = 16.0 * basePercent / 100.0 + 4.0;

        for (quint8 phase = 0; phase < 2; ++phase)
        {
            qreal current = baseCurrent;

            task.value.push_back(m_mpi.dac()->rawFromValue(current));

            // В терминах тока:
            // 1-я фаза: вверх, 2-я фаза: вниз (НЕ зависит от normalOpen)
            const qreal dir = (phase == 0 ? +1.0 : -1.0);

            for (auto it_s = p.steps.begin(); it_s < p.steps.end(); ++it_s)
            {
                const qreal stepValue = 16.0 * (*it_s) / 100.0;
                current += dir * stepValue;
                task.value.push_back(m_mpi.dac()->rawFromValue(current));
            }
        }
    }

    task.value.push_back(m_mpi.dac()->rawFromValue(4.0));

    worker->SetTask(task);

    const quint64 P = static_cast<quint64>(p.points.size());
    const quint64 S = static_cast<quint64>(p.steps.size());
    const quint64 delay = static_cast<quint64>(p.delay);
    const quint64 N_values = 1ULL + 2ULL * P * (1ULL + S);
    const quint64 totalMs = 10000ULL + N_values * delay + 10000ULL;

    RunnerConfig cfg;
    cfg.worker = worker;
    cfg.totalMs = totalMs;
    cfg.chartToClear = static_cast<int>(Charts::Response);
    return cfg;
}

void OptionResponseRunner::wireSpecificSignals(Test& base) {
    auto& t = static_cast<OptionTest&>(base);
    auto* owner = qobject_cast<Program*>(parent()); Q_ASSERT(owner);

    connect(&t, &OptionTest::UpdateGraph,
            owner, [owner]{ owner->updateCharts_optionTest(Charts::Response); },
            Qt::QueuedConnection);

    connect(&t, &OptionTest::SetStartTime,
            owner, &Program::setTimeStart,
            Qt::QueuedConnection);
}
