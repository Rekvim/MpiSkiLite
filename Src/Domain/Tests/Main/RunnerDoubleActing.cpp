#include "RunnerDoubleActing.h"

#include "Domain/Measurement/Sensor.h"
#include "Domain/Mpi/Device.h"

namespace Domain::Tests::Main {

RunnerConfig RunnerDoubleActing::buildConfig()
{
    auto p = m_daParams;

    if (p.delay == 0)
        return {};

    // Total time: initial stabilisation + forward jump + pause (unknown) + return
    // Use a generous estimate; the actual duration depends on mechanic's speed
    const quint64 totalMs = 10000ULL + p.delay + 10000ULL;

    p.dac_min =
        qMax(m_device.dac()->rawFromValue(p.signal_min),
             m_device.dacMin());

    p.dac_max =
        qMin(m_device.dac()->rawFromValue(p.signal_max),
             m_device.dacMax());

    auto worker = std::make_unique<AlgorithmDoubleActing>(p);

    return makeConfig(
        std::move(worker),
        totalMs,
        Widgets::Chart::ChartType::Task
    );
}

void RunnerDoubleActing::wireSpecificSignals(AbstractTestAlgorithm& base)
{
    Runner::wireSpecificSignals(base);

    auto& t = static_cast<AlgorithmDoubleActing&>(base);

    connect(&t, &AlgorithmDoubleActing::waitingForManualResume,
            this, &RunnerDoubleActing::waitingForManualResume,
            Qt::QueuedConnection);
}

}
