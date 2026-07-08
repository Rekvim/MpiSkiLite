#include "Runner.h"

#include "Domain/Measurement/Sensor.h"
#include "Domain/Mpi/Device.h"

namespace Domain::Tests::Main {

RunnerConfig Runner::buildConfig()
{
    auto params = m_params;

    if (params.delay == 0)
        return {};

    const quint64 N = static_cast<quint64>(params.pointNumbers);

    const quint64 upMs = params.delay + N * params.response;
    const quint64 downMs = params.delay + N * params.response;
    const quint64 totalMs = 10000ULL + upMs + downMs + 10000ULL;

    params.dac_min =
        qMax(m_device.dac()->rawFromValue(params.signal_min),
             m_device.dacMin());

    params.dac_max =
        qMin(m_device.dac()->rawFromValue(params.signal_max),
             m_device.dacMax());

    auto worker = std::make_unique<Algorithm>(params);

    return makeConfig(
        std::move(worker),
        totalMs,
        Widgets::Chart::ChartType::Task
        );
}

void Runner::wireSpecificSignals(AbstractTestAlgorithm& base)
{
    auto& t = static_cast<Algorithm&>(base);

    connect(&t, &Algorithm::backwardStrokeStarted,
            this, &Runner::startBackwardStroke,
            Qt::QueuedConnection);

    connect(&t, &Algorithm::dublSeries,
            this, &Runner::dublSeries,
            Qt::QueuedConnection);

    connect(&t, &Algorithm::processCompleted,
            this, &Runner::processCompleted,
            Qt::QueuedConnection);
}

}