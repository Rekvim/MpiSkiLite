#include "StrokeTestRunner.h"
#include "./Src/Tests/StrokeTest.h"
#include "./Program.h"

RunnerConfig StrokeTestRunner::buildConfig()
{
    auto* worker = new StrokeTest;

    auto owner = qobject_cast<Program*>(parent());
    const auto& valveInfo = owner->registry()->valveInfo();

    StrokeTest::Config cfg;
    cfg.normalClosed = (valveInfo.safePosition == 0);

    worker->setConfig(cfg);

    RunnerConfig rc;
    rc.worker = worker;
    rc.totalMs = 0;
    rc.chartToClear = static_cast<int>(Charts::Stroke);

    return rc;
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
