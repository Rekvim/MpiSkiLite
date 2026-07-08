#include "Runner.h"
#include "Domain/Tests/Option/Algorithm.h"
#include "Domain/Mpi/Device.h"
#include "Domain/Measurement/Sensor.h"

namespace Domain::Tests::Option::Resolution {
RunnerConfig Runner::buildConfig()
{
    const auto& p = m_params;

    if (p.points.empty())
        return {};

    Algorithm::Task task;
    task.delay = p.delay;

    task.value.push_back(m_device.dac()->rawFromValue(4.0));

    for (auto it = p.points.begin(); it != p.points.end(); ++it)
    {
        const qreal basePercent = m_normalOpen ? (100.0 - *it) : *it;
        const qreal baseCurrent = 16.0 * basePercent / 100.0 + 4.0;
        const qreal baseRaw = m_device.dac()->rawFromValue(baseCurrent);
        task.value.push_back(baseRaw);

        for (auto it_s = p.steps.begin(); it_s < p.steps.end(); ++it_s)
        {
            const qreal stepValue = 16.0 * (*it_s) / 100.0;

            // вверх
            const qreal stepUpCurrent = baseCurrent + stepValue;
            task.value.push_back(m_device.dac()->rawFromValue(stepUpCurrent));
            task.value.push_back(baseRaw);

            // вниз
            const qreal stepDownCurrent = baseCurrent - stepValue;
            task.value.push_back(m_device.dac()->rawFromValue(stepDownCurrent));
            task.value.push_back(baseRaw);
        }
    }

    task.value.push_back(m_device.dac()->rawFromValue(4.0));

    auto worker = std::make_unique<Algorithm>(task);

    const quint64 P = static_cast<quint64>(p.points.size());
    const quint64 S = static_cast<quint64>(p.steps.size());
    const quint64 delay = static_cast<quint64>(p.delay);

    const quint64 N_values =
        2ULL + P * (1ULL + 2ULL * S);

    const quint64 totalMs =
        10000ULL + N_values * delay + 10000ULL;

    return makeConfig(std::move(worker), totalMs, Widgets::Chart::ChartType::Resolution);
}

void Runner::wireSpecificSignals(AbstractTestAlgorithm& base) {
    Q_UNUSED(base);
}
}