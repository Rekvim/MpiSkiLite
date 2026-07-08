#include "Runner.h"
#include "Domain/Tests/Option/Algorithm.h"

#include "Domain/Measurement/Sensor.h"
#include "Domain/Mpi/Device.h"

namespace Domain::Tests::Option::Step {
static QVector<quint16> buildSequence(const Params& p, Mpi::Device& device, bool normalOpen)
{
    QVector<quint16> seq;
    auto rawFromPct = [&](quint16 pct){
        const qreal cur = 16.0 * (normalOpen ? (100 - pct) : pct) / 100.0 + 4.0;
        return device.dac()->rawFromValue(cur);
    };

    const qreal start = 4.0;
    const qreal end = 20.0;

    seq.push_back(device.dac()->rawFromValue(start));
    for (auto v : p.points) seq.push_back(rawFromPct(v));
    seq.push_back(device.dac()->rawFromValue(end));
    for (auto it = p.points.crbegin(); it != p.points.crend(); ++it)
        seq.push_back(rawFromPct(*it));

    seq.push_back(device.dac()->rawFromValue(start));
    return seq;
}

RunnerConfig Runner::buildConfig() {
    const auto& p = m_params;

    if (p.points.empty()) return {};

    const quint64 P = static_cast<quint64>(p.points.size());
    const quint64 delay = static_cast<quint64>(p.delay);
    const quint64 N_values = 3 + 2 * P;
    const quint64 totalMs = 10000ULL + N_values * delay;

    Algorithm::Task task;
    task.delay = p.delay;
    task.value = buildSequence(p, m_device, m_normalOpen);

    auto worker = std::make_unique<Algorithm>(task);

    return makeConfig(std::move(worker), totalMs, Widgets::Chart::ChartType::Step);
}

void Runner::wireSpecificSignals(AbstractTestAlgorithm& base)
{
}
}