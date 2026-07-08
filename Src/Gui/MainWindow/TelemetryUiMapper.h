#pragma once

#include "Storage/Telemetry.h"

namespace Ui {
class MainWindow;
}

class TelemetryUiMapper
{
public:
    TelemetryUiMapper(Ui::MainWindow* ui) : m_ui(ui) { }

    void updateInit(const InitState& init,
                    const ValveStrokeRecord& strokeResult);

    void updateMainTest(const Domain::Tests::Main::Result& mainResult,
                        const ValveStrokeRecord& strokeResult);

    void updateStrokeTest(const Domain::Tests::Stroke::Result& result);

    void updateStepTest(const Domain::Tests::Option::Step::Result& result);

    void updateCrossingValues(const Domain::Tests::Main::Result& mainResult,
                              const ValveStrokeRecord& strokeResult);

private:
    Ui::MainWindow* m_ui;
};