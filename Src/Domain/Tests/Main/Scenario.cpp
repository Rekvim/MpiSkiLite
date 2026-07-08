#include "Scenario.h"

#include "Domain/Tests/BaseRunner.h"
#include "Runner.h"
#include "Analyzer.h"

#include "Utils/NumberUtils.h"

#include <QDebug>

namespace Domain::Tests::Main {

namespace {

static bool inRange(double value, double lower, double upper)
{
    if (lower > upper)
        std::swap(lower, upper);

    return value >= lower && value <= upper;
}
}

Scenario::Scenario(Tests::Context context,
                   const Params& params,
                   QObject* parent)
    : Tests::AbstractScenario(parent)
    , m_context(context)
    , m_params(params)
{
}

Scenario::~Scenario() = default;

void Scenario::beforeStart()
{
    m_analyzer = std::make_unique<Analyzer>();

    Analyzer::Config cfg;
    cfg.driveDiameter = m_context.config.driveDiameter;

    m_analyzer->setConfig(cfg);
    m_analyzer->start();
}

std::unique_ptr<BaseRunner> Scenario::createRunner()
{
    const bool normalOpen = m_context.config.safePosition == SafePosition::NormallyOpen;

    return std::make_unique<Runner>(
        m_context.device,
        normalOpen,
        m_params,
        this
    );
}

void Scenario::afterRunnerCreated(BaseRunner& baseRunner)
{
    auto& runner = static_cast<Runner&>(baseRunner);

    connect(&runner, &Runner::dublSeries,
            this, &Scenario::duplicateMainChartsSeriesRequested,
            Qt::QueuedConnection);

    connect(&runner, &Runner::startBackwardStroke,
            this, [this]() {
                if (m_analyzer)
                    m_analyzer->startBackwardStroke();
            },
            Qt::DirectConnection);

    connect(&runner, &Runner::processCompleted,
            this, &Scenario::onProcessCompleted,
            Qt::DirectConnection);
}

void Scenario::onSample(const Measurement::Sample& sample)
{
    if (m_analyzer)
        m_analyzer->onSample(sample);
}

void Scenario::onProcessCompleted()
{
    if (!m_analyzer) {
        qWarning() << "[Main::Scenario] Analyzer is null";
        return;
    }

    m_analyzer->finish();

    const Result& analyzerResult = m_analyzer->result();

    auto& main = m_context.telemetry.testMain.emplace();

    main.pressureDiff = analyzerResult.pressureDiff;

    main.frictionForce = analyzerResult.frictionForce;
    main.frictionPercent = analyzerResult.frictionPercent;

    main.dynamicErrorMean = analyzerResult.dynamicErrorMean;
    main.dynamicErrorMeanPercent = analyzerResult.dynamicErrorMeanPercent;

    main.dynamicErrorMax = analyzerResult.dynamicErrorMax;
    main.dynamicErrorMaxPercent = analyzerResult.dynamicErrorMaxPercent;

    main.dynamicErrorReal = analyzerResult.dynamicErrorReal;

    main.lowLimitPressure = analyzerResult.lowLimitPressure;
    main.highLimitPressure = analyzerResult.highLimitPressure;

    main.springLow = analyzerResult.springLow;
    main.springHigh = analyzerResult.springHigh;

    main.linearityError = analyzerResult.linearityError;
    main.linearity = analyzerResult.linearity;

    updateCrossingStatus();

    emit addRegressionRequested(m_analyzer->regressionChartPointsForward(),
                                m_analyzer->regressionChartPointsBackward());
    emit addFrictionRequested(m_analyzer->frictionChartPoints());

    emit mainResultUpdated(main);
    emit crossingStatusUpdated(m_context.telemetry.crossingStatus);
    emit telemetryUpdated(m_context.telemetry);
}

void Scenario::updateCrossingStatus()
{
    auto& ts = m_context.telemetry;
    const auto& cfg = m_context.config;
    const QString& valveStroke = cfg.valveStroke;
    const auto& limits = cfg.crossingLimits;

    using State = CrossingStatus::State;

    if (!ts.testMain)
        return;

    if (limits.frictionEnabled) {
        ts.crossingStatus.frictionPercent =
            inRange(ts.testMain->frictionPercent,
                    limits.frictionCoefLower,
                    limits.frictionCoefUpper)
                ? State::Ok
                : State::Fail;
    } else {
        ts.crossingStatus.frictionPercent = State::Unknown;
    }

    if (limits.valveStrokeEnabled) {
        bool ok = false;
        const double recStroke = NumberUtils::toDouble(valveStroke, &ok);

        if (ok) {
            const double d =
                std::abs(recStroke) * (limits.valveStroke / 100.0);

            const double lo = recStroke - d;
            const double hi = recStroke + d;

            ts.crossingStatus.valveStroke =
                inRange(ts.valveStrokeRecord.real, lo, hi)
                    ? State::Ok
                    : State::Fail;
        } else {
            ts.crossingStatus.valveStroke = State::Unknown;
        }
    } else {
        ts.crossingStatus.valveStroke = State::Unknown;
    }

    if (limits.dynamicErrorEnabled) {
        ts.crossingStatus.dynamicError =
            inRange(ts.testMain->dynamicErrorReal,
                    0.0,
                    cfg.dinamicErrorRecomend)
                ? State::Ok
                : State::Fail;
    } else {
        ts.crossingStatus.dynamicError = State::Unknown;
    }

    if (limits.springEnabled) {
        double recLow = cfg.driveRangeLow;
        double recHigh = cfg.driveRangeHigh;

        if (recLow > recHigh)
            std::swap(recLow, recHigh);

        const double lowD = std::abs(recLow) * (limits.springLower / 100.0);
        const double highD = std::abs(recHigh) * (limits.springUpper / 100.0);

        const double lowLo = recLow - lowD;
        const double lowHi = recLow + lowD;

        const double highLo = recHigh - highD;
        const double highHi = recHigh + highD;

        const bool okLow = inRange(ts.testMain->springLow, lowLo, lowHi);

        const bool okHigh = inRange(ts.testMain->springHigh, highLo, highHi);

        ts.crossingStatus.spring = (okLow && okHigh) ? State::Ok : State::Fail;
    } else {
        ts.crossingStatus.spring = State::Unknown;
    }

    if (limits.linearCharacteristicEnabled) {
        ts.crossingStatus.linearCharacteristic =
            inRange(ts.testMain->linearityError,
                    0.0,
                    limits.linearCharacteristic)
                ? State::Ok
                : State::Fail;
    } else {
        ts.crossingStatus.linearCharacteristic = State::Unknown;
    }
}

}