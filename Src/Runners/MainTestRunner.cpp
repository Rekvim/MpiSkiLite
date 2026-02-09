#include "MainTestRunner.h"
#include "./Src/Tests/MainTest.h"
#include "./Src/Ui/TestSettings/MainTestSettings.h"
#include "./Program.h"

RunnerConfig MainTestRunner::buildConfig() {
    MainTestSettings::TestParameters p{};
    emit getParameters_mainTest(p);
    if (p.delay == 0) return {};

    const quint64 N = static_cast<quint64>(p.pointNumbers);
    const quint64 upMs = p.delay + N * p.response;
    const quint64 downMs = p.delay + N * p.response;
    const quint64 totalMs = 10000ULL + upMs + downMs + 10000ULL;

    p.dac_min = qMax(m_mpi.dac()->rawFromValue(p.signal_min), m_mpi.dacMin());
    p.dac_max = qMin(m_mpi.dac()->rawFromValue(p.signal_max), m_mpi.dacMax());

    auto* worker = new MainTest;
    worker->SetParameters(p);

    RunnerConfig cfg;
    cfg.worker = worker;
    cfg.totalMs = totalMs;
    cfg.chartToClear = static_cast<int>(Charts::Task);

    emit totalTestTimeMs(totalMs);

    return cfg;
}

void MainTestRunner::wireSpecificSignals(Test& base) {
    auto& t = static_cast<MainTest&>(base);
    auto owner = qobject_cast<Program*>(parent());

    connect(&t, &MainTest::UpdateGraph,
            owner, &Program::updateCharts_mainTest,
            Qt::QueuedConnection);

    connect(&t, &MainTest::GetPoints,
            owner, &Program::receivedPoints_mainTest,
            Qt::BlockingQueuedConnection);

    connect(&t, &MainTest::DublSeries,
            owner, [owner]{ emit owner->duplicateMainChartsSeries(); });

    connect(&t, &MainTest::AddRegression,
            owner, &Program::addRegression,
            Qt::QueuedConnection);

    connect(&t, &MainTest::AddFriction,
            owner, &Program::addFriction,
            Qt::QueuedConnection);

    connect(&t, &MainTest::Results,
            owner, &Program::results_mainTest,
            Qt::QueuedConnection);

    connect(&t, &MainTest::ShowDots,
            owner, [owner](bool v){ emit owner->showDots(v); });

    connect(&t, &MainTest::ClearGraph, owner, [owner]{
        emit owner->clearPoints(Charts::Task);
        emit owner->clearPoints(Charts::Pressure);
        emit owner->clearPoints(Charts::Friction);
        emit owner->setRegressionEnable(false);
    });

    connect(&t, &MainTest::EndTest,
            owner, &Program::mainTestFinished,
            Qt::QueuedConnection);
}
