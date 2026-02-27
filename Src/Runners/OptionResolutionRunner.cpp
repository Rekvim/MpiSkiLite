#include "OptionResolutionRunner.h"
#include "./Program.h"
#include "./Registry.h"
#include "./Src/Tests/OptionTest.h"

RunnerConfig OptionResolutionRunner::buildConfig()
{
    OtherTestSettings::TestParameters p{};
    emit getParameters_resolutionTest(p);
    if (p.points.empty())
        return {};

    auto* worker = new OptionTest;
    OptionTest::Task task;
    task.delay = p.delay;

    const bool normalOpen = (m_reg.valveInfo().safePosition != 0);

    // Старт — 4 мА
    task.value.push_back(m_mpi.dac()->rawFromValue(4.0));

    for (auto it = p.points.begin(); it != p.points.end(); ++it)
    {
        // Базовая точка (инверсия только здесь)
        const qreal basePercent =
            normalOpen ? (100.0 - *it) : *it;

        const qreal baseCurrent =
            16.0 * basePercent / 100.0 + 4.0;

        const qreal baseRaw =
            m_mpi.dac()->rawFromValue(baseCurrent);

        // Добавляем базовую точку
        task.value.push_back(baseRaw);

        for (auto it_s = p.steps.begin();
             it_s < p.steps.end();
             ++it_s)
        {
            const qreal stepValue =
                16.0 * (*it_s) / 100.0;

            // Физически:
            // NC: сначала вверх (ток +), потом база
            // NO: сначала вниз (ток +, потому что инверсия уже в базе)
            const qreal stepCurrent =
                baseCurrent + stepValue;

            task.value.push_back(
                m_mpi.dac()->rawFromValue(stepCurrent));

            // возврат к базе
            task.value.push_back(baseRaw);
        }
    }

    // Финиш — 4 мА
    task.value.push_back(m_mpi.dac()->rawFromValue(4.0));

    worker->SetTask(task);

    const quint64 P = static_cast<quint64>(p.points.size());
    const quint64 S = static_cast<quint64>(p.steps.size());
    const quint64 delay = static_cast<quint64>(p.delay);

    // старт + для каждой точки: база + (шаг+база)*S + финиш
    const quint64 N_values =
        2ULL + P * (1ULL + 2ULL * S);

    const quint64 totalMs =
        10000ULL + N_values * delay + 10000ULL;

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
