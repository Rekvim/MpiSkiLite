#include "StrokeTestRunner.h"
#include "./Src/Tests/StrokeTest.h"
#include "./Program.h"

RunnerConfig StrokeTestRunner::buildConfig() {
    auto* worker = new StrokeTest;

    RunnerConfig cfg;
    cfg.worker = worker;
    cfg.totalMs = 0;
    cfg.chartToClear = static_cast<int>(Charts::Stroke);
    return cfg;
}

void StrokeTestRunner::wireSpecificSignals(Test& base) {
    auto& t = static_cast<StrokeTest&>(base);
    auto owner = qobject_cast<Program*>(parent());

    connect(&t, &StrokeTest::UpdateGraph,
            owner, &Program::updateCharts_strokeTest,
            Qt::QueuedConnection);

    connect(&t, &StrokeTest::SetStartTime,
            owner, &Program::setTimeStart,
            Qt::QueuedConnection);

    connect(&t, &StrokeTest::Results,
            owner, &Program::results_strokeTest,
            Qt::QueuedConnection);

    connect(&t, &StrokeTest::GetPoints,
            owner, &Program::receivedPoints_strokeTest,
            Qt::BlockingQueuedConnection);
}
