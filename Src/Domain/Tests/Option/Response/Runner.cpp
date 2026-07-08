#include "Runner.h"
#include "Domain/Tests/Option/Algorithm.h"

#include "Domain/Measurement/Sensor.h"
#include "Domain/Mpi/Device.h"

namespace Domain::Tests::Option::Response {
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

        for (quint8 phase = 0; phase < 2; ++phase)
        {
            qreal current = baseCurrent;

            task.value.push_back(m_device.dac()->rawFromValue(current));

            const qreal dir = (phase == 0 ? +1.0 : -1.0);

            for (auto it_s = p.steps.begin(); it_s < p.steps.end(); ++it_s)
            {
                const qreal stepValue = 16.0 * (*it_s) / 100.0;
                current += dir * stepValue;
                task.value.push_back(m_device.dac()->rawFromValue(current));
            }
        }
    }

    task.value.push_back(m_device.dac()->rawFromValue(4.0));

    auto worker = std::make_unique<Algorithm>(task);

    const quint64 P = static_cast<quint64>(p.points.size());
    const quint64 S = static_cast<quint64>(p.steps.size());
    const quint64 delay = static_cast<quint64>(p.delay);
    const quint64 N_values = 1ULL + 2ULL * P * (1ULL + S);
    const quint64 totalMs = 10000ULL + N_values * delay + 10000ULL;

    return makeConfig(std::move(worker), totalMs, Widgets::Chart::ChartType::Response);
}

void Runner::wireSpecificSignals(AbstractTestAlgorithm& base) {
    Q_UNUSED(base)
}
}