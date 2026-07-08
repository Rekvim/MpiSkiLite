#include "AlgorithmDoubleActing.h"

namespace Domain::Tests::Main {

void AlgorithmDoubleActing::run()
{
    // Hold at minimum, wait for valve to stabilise
    setDacBlocked(m_params.dac_min, 0, true);
    if (m_terminate) { emit finished(); return; }

    Sleep(m_params.delay);

    // Jump directly to maximum (forward stroke — no gradual steps)
    setDacBlocked(m_params.dac_max, 0, true);
    if (m_terminate) { emit finished(); return; }

    // Notify analyzer and chart that forward stroke is complete
    emit backwardStrokeStarted();
    emit dublSeries();

    // Pause: mechanic must manually reset mA to 3 mA and return the valve
    emit waitingForManualResume();
    waitBlocked();
    if (m_terminate) { emit finished(); return; }

    // Sync DAC to minimum (mechanic already returned valve to start position)
    setDacBlocked(m_params.dac_min, 0, false, false);

    emit processCompleted();
    emit finished();
}

}
