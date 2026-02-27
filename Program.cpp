#include "Program.h"

#include "./Src/Tests/StepTest.h"
#include "./Src/Tests/StrokeTest.h"
#include "./Src/Tests/MainTest.h"
#include "./Src/Runners/MainTestRunner.h"
#include "./Src/Runners/StepTestRunner.h"
#include "./Src/Runners/StrokeTestRunner.h"
#include "./Src/Runners/OptionResponseRunner.h"
#include "./Src/Runners/OptionResolutionRunner.h"
#include <QRegularExpression>
#include <QLocale>
#include <utility>

namespace {

double toDouble(QString s, bool* okOut = nullptr)
{
    s = s.trimmed();
    s.replace(',', '.');
    bool ok = false;
    const double v = QLocale::c().toDouble(s, &ok);
    if (okOut) *okOut = ok;
    return v;
}
} // namespace


Program::Program(QObject *parent)
    : QObject{parent}
{
    qRegisterMetaType<RealtimeState>("RealtimeState");

    m_timerSensors = new QTimer(this);
    m_timerSensors->setInterval(200);

    m_dacEventLoop = new QEventLoop(this);

    connect(m_timerSensors, &QTimer::timeout,
            this, &Program::updateSensors);

    // m_timerDI = new QTimer(this);
    // m_timerDI->setInterval(1000);
    // connect(m_timerDI, &QTimer::timeout, this, [&]() {
    //     quint8 DI = m_mpi.digitalInputs();
    //     emit setDiCheckboxesChecked(DI);
    // });

    // connect(&m_mpi, &MPI::errorOccured,
    //         this, &Program::errorOccured,
    //         Qt::QueuedConnection);
}

void Program::setRegistry(Registry *registry)
{
    m_registry = registry;
}

void Program::setDacRaw(quint16 dac, quint32 sleepMs, bool waitForStop, bool waitForStart)
{
    m_isDacStopRequested = false;

    if (m_mpi.sensorCount() == 0) {
        emit releaseBlock();
        return;
    }

    m_mpi.setDacRaw(dac);

    if (waitForStart) {
        QTimer timer;
        timer.setInterval(50);

        QList<quint16> lineSensor;

        connect(&timer, &QTimer::timeout, this, [&]() {
            lineSensor.push_back(m_mpi[0]->rawValue());
            if (qAbs(lineSensor.first() - lineSensor.last()) > 10) {
                timer.stop();
                m_dacEventLoop->quit();
            }
            if (lineSensor.size() > 50) {
                lineSensor.pop_front();
            }
        });

        timer.start();
        m_dacEventLoop->exec();
        timer.stop();
    }

    if (m_isDacStopRequested) {
        emit releaseBlock();
        return;
    }

    if (sleepMs > 20) {
        QTimer timer;
        connect(&timer, &QTimer::timeout, m_dacEventLoop, &QEventLoop::quit);
        timer.start(sleepMs);
        m_dacEventLoop->exec();
        timer.stop();
    }

    if (m_isDacStopRequested) {
        emit releaseBlock();
        return;
    }

    if (waitForStop) {
        QTimer timer;
        timer.setInterval(50);

        QList<quint16> lineSensor;

        connect(&timer, &QTimer::timeout, this, [&]() {
            lineSensor.push_back(m_mpi[0]->rawValue());
            if (lineSensor.size() == 50) {
                if (qAbs(lineSensor.first() - lineSensor.last()) < 10) {
                    timer.stop();
                    m_dacEventLoop->quit();
                }
                lineSensor.pop_front();
            }
        });

        timer.start();
        m_dacEventLoop->exec();
        timer.stop();
    }

    emit releaseBlock();
}

void Program::setTimeStart()
{
    m_startTime = QDateTime::currentMSecsSinceEpoch();
}

void Program::updateSensors()
{
    for (quint8 i = 0; i < m_mpi.sensorCount(); ++i) {
        switch (i) {
        case 0:
            emit setText(TextObjects::LineEdit_linearSensor, m_mpi[i]->formattedValue());
            emit setText(TextObjects::LineEdit_linearSensorPercent, m_mpi[i]->percentFormatted());
            break;
        case 1:
            emit setText(TextObjects::LineEdit_pressureSensor_1, m_mpi[i]->formattedValue());
            break;
        case 2:
            emit setText(TextObjects::LineEdit_pressureSensor_2, m_mpi[i]->formattedValue());
            break;
        case 3:
            emit setText(TextObjects::LineEdit_pressureSensor_3, m_mpi[i]->formattedValue());
            break;
        }
    }
    if (m_activeRunner)
        emit setTask(m_mpi.dac()->value());

    Sensor *feedbackSensor = m_mpi.dac();

    QString fbValue = feedbackSensor->formattedValue();
    emit setText(TextObjects::LineEdit_feedback_4_20mA, fbValue);

    QVector<Point> points;

    const auto& v = m_registry->valveInfo();
    qreal percent = calcPercent(
        m_mpi.dac()->value(),
        v.safePosition != 0
        );

    quint64 time = QDateTime::currentMSecsSinceEpoch() - m_initTime;

    points.push_back({0, qreal(time), percent});
    points.push_back({1, qreal(time), m_mpi[0]->percent()});

    emit addPoints(Charts::Trend, points);
}

void Program::endTest()
{
    emit setTaskControlsEnabled(true);
    emit setButtonInitEnabled(true);

    emit setTask(m_mpi.dac()->value());

    const bool hadRunner = (m_activeRunner != nullptr);

    m_activeRunner.reset();

    if (hadRunner)
        emit testReallyFinished();

    emit testFinished();
}

void Program::disposeActiveRunnerAsync() {
    m_activeRunner.reset();
}

void Program::setDAC_real(qreal value)
{
    m_mpi.setDacValue(value);
}

// void Program::setInitDOStates(const QVector<bool> &states)
// {
//     m_initialDoStates = states;
//     m_savedInitialDoStates = states;
// }

void Program::initialization()
{
    auto &ts = m_telemetryStore;

    ts.init.initStatusText = "";
    ts.init.connectedSensorsText = "";
    ts.init.deviceStatusText = "";
    ts.init.startingPositionText = "";
    ts.init.finalPositionText = "";

    m_timerSensors->stop();
    // m_timerDI->stop();
    emit setButtonInitEnabled(false);

    if (!connectAndInitDevice()) {
        emit setButtonInitEnabled(true);
        return;
    }

    if (!detectAndReportSensors()) {
        emit setButtonInitEnabled(true);
        return;
    }

    const auto& valveInfo = m_registry->valveInfo();
    bool normalClosed = (valveInfo.safePosition == 0);


    if (m_patternType == SelectTests::Pattern_C_CVT) {
        measureStartPosition(normalClosed);
        measureEndPosition(normalClosed);
    }

    calculateAndApplyCoefficients();

    if (m_patternType == SelectTests::Pattern_C_CVT) {
        recordStrokeRange(normalClosed);
    }

    finalizeInitialization();
}

// Подключение и инициализация устройства
bool Program::connectAndInitDevice()
{
    auto &ts = m_telemetryStore;
    bool ok = m_mpi.isConnect();
    ts.init.deviceStatusText  = ok
                                   ? QString("Успешное подключение к порту %1").arg(m_mpi.portName())
                                   : "Ошибка подключения";
    ts.init.deviceStatusColor = ok ? Qt::darkGreen : Qt::red;
    emit telemetryUpdated(ts);

    if (!ok) {
        return false;
    }

    ok = m_mpi.initialize();
    ts.init.initStatusText  = ok ? "Успешная инициализация" : "Ошибка инициализации";
    ts.init.initStatusColor = ok ? Qt::darkGreen : Qt::red;
    emit telemetryUpdated(ts);

    m_isInitialized = ok;
    return ok;
}

// Обнаружение и отчёт по датчикам
bool Program::detectAndReportSensors()
{
    auto &ts = m_telemetryStore;
    int cnt = m_mpi.sensorCount();

    if (cnt == 0) {
        ts.init.connectedSensorsText = "Датчики не обнаружены";
        ts.init.connectedSensorsColor = Qt::red;
    }
    else if (cnt == 1) {
        ts.init.connectedSensorsText  = "Обнаружен 1 датчик";
        ts.init.connectedSensorsColor = Qt::darkYellow;
    }
    else {
        ts.init.connectedSensorsText = QString("Обнаружено %1 датчика").arg(cnt);
        ts.init.connectedSensorsColor = Qt::darkGreen;
    }

    emit telemetryUpdated(ts);

    return cnt > 0;
}

void Program::waitForDacCycle()
{
    QTimer timer(this);
    connect(&timer, &QTimer::timeout, this, [&] {
        if (!m_shouldWaitForButton || m_isDacStopRequested)
            m_dacEventLoop->quit();
    });
    timer.start(50);
    m_dacEventLoop->exec();
    timer.stop();
}

void Program::measureStartPosition(bool normalClosed)
{
    auto &ts = m_telemetryStore;
    ts.init.startingPositionText = "Измерение";
    ts.init.startingPositionColor = Qt::darkYellow;
    emit telemetryUpdated(ts);

    setDacRaw(0, 10000, true);
    waitForDacCycle();

    if (normalClosed) m_mpi[0]->captureMin();
    else m_mpi[0]->captureMax();

    ts.init.startingPositionText  = m_mpi[0]->formattedValue();
    ts.init.startingPositionColor = Qt::darkGreen;
    emit telemetryUpdated(ts);
}

void Program::measureEndPosition(bool normalClosed)
{
    auto &ts = m_telemetryStore;
    ts.init.finalPositionText = "Измерение";
    ts.init.finalPositionColor = Qt::darkYellow;
    emit telemetryUpdated(ts);

    setDacRaw(65535, 10000, true);

    waitForDacCycle();

    if (normalClosed) m_mpi[0]->captureMax();
    else m_mpi[0]->captureMin();

    ts.init.finalPositionText  = m_mpi[0]->formattedValue();
    ts.init.finalPositionColor = Qt::darkGreen;
    emit telemetryUpdated(ts);
}

void Program::measureStartPositionShutoff(bool normalClosed)
{
    // auto &ts = m_telemetryStore;

    // ts.init.startingPositionText  = "Измерение";
    // ts.init.startingPositionColor = Qt::darkYellow;
    // emit telemetryUpdated(ts);

    // for (int i = 0; i < m_savedInitialDoStates.size(); ++i) {
    //     if (m_savedInitialDoStates[i]) {
    //         m_initialDoStates[i] = false;
    //         m_mpi.setDiscreteOutput(i, false);
    //     }
    // }
    // emit setDoButtonsChecked(m_mpi.digitalOutputs());

    // setDacRaw(0, 1000, true, true);

    // waitForDacCycle();

    // if (normalClosed) m_mpi[0]->captureMin();
    // else m_mpi[0]->captureMax();

    // ts.init.startingPositionText = m_mpi[0]->formattedValue();
    // ts.init.startingPositionColor = Qt::darkGreen;
    // emit telemetryUpdated(ts);
}

void Program::measureEndPositionShutoff(bool normalClosed)
{
    // auto &s = m_telemetryStore;

    // s.init.finalPositionText = "Измерение";
    // s.init.finalPositionColor = Qt::darkYellow;
    // emit telemetryUpdated(s);

    // for (int i = 0; i < m_savedInitialDoStates.size(); ++i) {
    //     if (m_savedInitialDoStates[i]) {
    //         m_initialDoStates[i] = true;
    //         m_mpi.setDiscreteOutput(i, true);
    //     }
    // }
    // emit setDoButtonsChecked(m_mpi.digitalOutputs());

    // setDacRaw(65535, 1000, true);

    // waitForDacCycle();

    // if (normalClosed) m_mpi[0]->captureMax();
    // else m_mpi[0]->captureMin();

    // s.init.finalPositionText = m_mpi[0]->formattedValue();
    // s.init.finalPositionColor = Qt::darkGreen;
    // emit telemetryUpdated(s);
}

void Program::calculateAndApplyCoefficients()
{
    const auto& valveInfo = m_registry->valveInfo();

    qreal coeff = 1.0;

    if (valveInfo.strokeMovement != 0) {
        coeff = qRadiansToDegrees(2.0 / valveInfo.diameterPulley);
        m_mpi[0]->setUnit("°");
    }

    m_mpi[0]->correctCoefficients(coeff);
}

void Program::recordStrokeRange(bool normalClosed)
{
    auto &s = m_telemetryStore;

    if (normalClosed) {
        s.valveStrokeRecord.range = m_mpi[0]->formattedValue();
        s.valveStrokeRecord.real = m_mpi[0]->value();
        setDacRaw(0);
    } else {
        setDacRaw(0, 10000, true);
        s.valveStrokeRecord.range = m_mpi[0]->formattedValue();
        s.valveStrokeRecord.real = m_mpi[0]->value();
    }

    emit telemetryUpdated(s);
    emit setTask(m_mpi.dac()->value());
}

void Program::finalizeInitialization()
{
    emit clearPoints(Charts::Trend);
    m_initTime = QDateTime::currentMSecsSinceEpoch();

    emit setSensorNumber(m_mpi.sensorCount());
    emit setTaskControlsEnabled(true);
    emit setButtonInitEnabled(true);

    m_timerSensors->start();
}

bool Program::isInitialized() const {
    return m_isInitialized;
}

void Program::startMainTest()
{
    auto r = std::make_unique<MainTestRunner>(m_mpi, *m_registry, this);

    connect(r.get(), &MainTestRunner::getParameters_mainTest,
            this, &Program::forwardGetParameters_mainTest);

    startRunner(std::move(r));
}

void Program::receivedPoints_mainTest(QVector<QVector<QPointF>> &points)
{
    emit getPoints_mainTest(points, Charts::Task);
}

static bool inRange(double value, double lower, double upper)
{
    if (lower > upper)
        std::swap(lower, upper);
    return value >= lower && value <= upper;
}

void Program::updateCrossingStatus()
{
    auto &ts = m_telemetryStore;
    const auto& valveInfo = m_registry->valveInfo();
    const auto& limits = valveInfo.crossingLimits;
    using State = CrossingStatus::State;

    // --- friction ---
    if (limits.frictionEnabled) {
        ts.crossingStatus.frictionPercent =
            inRange(ts.mainTestRecord.frictionPercent,
                    limits.frictionCoefLowerLimit,
                    limits.frictionCoefUpperLimit)
                ? State::Ok : State::Fail;
    } else {
        ts.crossingStatus.frictionPercent = State::Unknown;
    }

    // --- range / stroke : сравниваем реальный ход с (recomend ± %) ---
    if (limits.rangeEnabled) {
        bool ok = false;
        const double recStroke = toDouble(valveInfo.strokValve, &ok);
        if (ok) {
            const double d = std::abs(recStroke) * (limits.rangeUpperLimit / 100.0); // rangeUpperLimit как %
            const double lo = recStroke - d;
            const double hi = recStroke + d;

            ts.crossingStatus.range =
                inRange(ts.valveStrokeRecord.real, lo, hi) ? State::Ok : State::Fail;
        } else {
            ts.crossingStatus.range = State::Unknown;
        }
    } else {
        ts.crossingStatus.range = State::Unknown;
    }

    // --- dynamicError (оставил как абсолютный лимит: 0..recomend) ---
    if (limits.dynamicErrorEnabled) {
        ts.crossingStatus.dynamicError =
            inRange(ts.mainTestRecord.dynamicErrorReal,
                    0.0,
                    valveInfo.dinamicErrorRecomend)
                ? State::Ok : State::Fail;
    } else {
        ts.crossingStatus.dynamicError = State::Unknown;
    }

    // --- spring : два диапазона, проверяем каждое число отдельно ---
    if (limits.springEnabled) {
        double recLow  = valveInfo.driveRangeLow;
        double recHigh = valveInfo.driveRangeHigh;

        if (recLow > recHigh)
            std::swap(recLow, recHigh);

        const double lowD  = std::abs(recLow) * (limits.springLowerLimit / 100.0);
        const double highD = std::abs(recHigh) * (limits.springUpperLimit / 100.0);

        const double lowLo = recLow - lowD;
        const double lowHi = recLow + lowD;

        const double highLo = recHigh - highD;
        const double highHi = recHigh + highD;

        const bool okLow  = inRange(ts.mainTestRecord.springLow,  lowLo,  lowHi);
        const bool okHigh = inRange(ts.mainTestRecord.springHigh, highLo, highHi);

        ts.crossingStatus.spring = (okLow && okHigh)
                                       ? State::Ok
                                       : State::Fail;
    } else {
        ts.crossingStatus.spring = State::Unknown;
    }

    // --- linearCharacteristic ---
    // Сейчас у тебя всегда Ok, это подозрительно.
    // Если у тебя есть лимит (например 0..limits.linearCharacteristicLowerLimit),
    // то сравнивай реальную ошибку:
    if (limits.linearCharacteristicEnabled) {
        ts.crossingStatus.linearCharacteristic =
            inRange(ts.mainTestRecord.linearityError, 0.0, limits.linearCharacteristicLowerLimit)
                ? State::Ok : State::Fail;
    } else {
        ts.crossingStatus.linearCharacteristic = State::Unknown;
    }
}

void Program::results_mainTest(const MainTest::TestResults &results)
{
    const auto& valveInfo = m_registry->valveInfo();
    qreal k = 5 * M_PI * valveInfo.driveDiameter * valveInfo.driveDiameter / 4;

    auto &s = m_telemetryStore.mainTestRecord;

    s.pressureDifference = results.pressureDiff;

    s.frictionForce = results.pressureDiff * k;
    s.frictionPercent = results.friction;

    s.dynamicError_mean = results.dynamicErrorMean;
    s.dynamicError_meanPercent = results.dynamicErrorMean / 0.16;
    s.dynamicError_max = results.dynamicErrorMax;
    s.dynamicError_maxPercent = results.dynamicErrorMean / 0.16;
    s.dynamicErrorReal = results.dynamicErrorMean / 0.16;

    s.lowLimitPressure = results.lowLimitPressure;
    s.highLimitPressure = results.highLimitPressure;

    s.springLow = results.springLow;
    s.springHigh = results.springHigh;

    s.linearityError = results.linearityError;
    s.linearity = results.linearity;

    updateCrossingStatus();
    emit telemetryUpdated(m_telemetryStore);
}

void Program::updateCharts_mainTest()
{
    QVector<Point> points;

    const auto& v = m_registry->valveInfo();
    qreal percent = calcPercent(
        m_mpi.dac()->value(),
        v.safePosition != 0
    );

    qreal task = m_mpi[0]->valueFromPercent(percent);
    qreal X = m_mpi.dac()->value();
    points.push_back({0, X, task});

    for (quint8 i = 0; i < m_mpi.sensorCount(); ++i) {
        points.push_back({static_cast<quint8>(i + 1), X, m_mpi[i]->value()});
    }

    emit addPoints(Charts::Task, points);

    points.clear();
    points.push_back({0, m_mpi[1]->value(), m_mpi[0]->value()});

    emit addPoints(Charts::Pressure, points);
}

void Program::addFriction(const QVector<QPointF> &points)
{
    QVector<Point> chartPoints;

    auto& valveInfo = m_registry->valveInfo();

    qreal k = 5 * M_PI * valveInfo.driveDiameter * valveInfo.driveDiameter / 4;

    for (QPointF point : points) {
        chartPoints.push_back({0, point.x(), point.y() * k});
    }
    emit addPoints(Charts::Friction, chartPoints);
}

void Program::addRegression(const QVector<QPointF> &points)
{
    QVector<Point> chartPoints;
    for (QPointF point : points) {
        chartPoints.push_back({1, point.x(), point.y()});
    }
    emit addPoints(Charts::Pressure, chartPoints);

    emit setRegressionEnable(true);
}

void Program::startStrokeTest() {
    auto r = std::make_unique<StrokeTestRunner>(m_mpi, *m_registry, this);
    startRunner(std::move(r));
}

void Program::receivedPoints_strokeTest(QVector<QVector<QPointF>> &points)
{
    emit getPoints_strokeTest(points, Charts::Stroke);
}

void Program::results_strokeTest(const quint64 forwardTime, const quint64 backwardTime)
{
    QString forwardText = QTime(0, 0).addMSecs(forwardTime).toString("mm:ss.zzz");
    QString backwardText = QTime(0, 0).addMSecs(backwardTime).toString("mm:ss.zzz");

    m_telemetryStore.strokeTestRecord.timeForwardMs = forwardText;
    m_telemetryStore.strokeTestRecord.timeBackwardMs = backwardText;

    emit telemetryUpdated(m_telemetryStore);
}

void Program::updateCharts_strokeTest()
{
    QVector<Point> points;

    const auto& v = m_registry->valveInfo();
    qreal percent = calcPercent(
        m_mpi.dac()->value(),
        v.safePosition != 0
    );

    quint64 time = QDateTime::currentMSecsSinceEpoch() - m_startTime;

    points.push_back({0, qreal(time), percent});
    points.push_back({1, qreal(time), m_mpi[0]->percent()});

    emit addPoints(Charts::Stroke, points);
}

// void Program::setMultipleDO(const QVector<bool>& states)
// {
//     quint8 mask = 0;
//     for (int d = 0; d < states.size(); ++d) {
//         m_mpi.setDiscreteOutput(d, states[d]);
//         if (states[d]) mask |= (1 << d);
//     }
//     //emit SetButtonsDOChecked(mask);
// }

void Program::startOptionalTest(quint8 testNum)
{
    switch (testNum) {
    case 0: {
        auto r = std::make_unique<OptionResponseRunner>(m_mpi, *m_registry, this);
        connect(r.get(), &OptionResponseRunner::getParameters_responseTest,
                this, [&](OtherTestSettings::TestParameters& p){
                    emit getParameters_responseTest(p);
                });
        startRunner(std::move(r));
        break;
    }
    case 1: {
        auto r = std::make_unique<OptionResolutionRunner>(m_mpi, *m_registry, this);
        connect(r.get(), &OptionResolutionRunner::getParameters_resolutionTest,
                this, [&](OtherTestSettings::TestParameters& p){
                    emit getParameters_resolutionTest(p);
                });
        startRunner(std::move(r));
        break;
    }
    case 2: {
        auto r = std::make_unique<StepTestRunner>(m_mpi, *m_registry, this);
        startRunner(std::move(r));
        break;
    }
    default:
        emit stopTheTest();
        break;
    }
}

void Program::receivedPoints_stepTest(QVector<QVector<QPointF>> &points)
{
    emit getPoints_stepTest(points, Charts::Step);
}

void Program::results_stepTest(const QVector<StepTest::TestResult> &results, quint32 T_value)
{
    m_telemetryStore.stepResults.clear();
    for (auto &r : results) {
        StepTestRecord rec;
        rec.from = r.from;
        rec.to = r.to;
        rec.T_value = r.T_value;
        rec.overshoot = r.overshoot;
        m_telemetryStore.stepResults.push_back(rec);
    }

    emit telemetryUpdated(m_telemetryStore);

    emit setStepResults(results, T_value);
}

void Program::updateCharts_optionTest(Charts chart)
{
    QVector<Point> points;

    const auto& v = m_registry->valveInfo();
    qreal percent = calcPercent(
        m_mpi.dac()->value(),
        v.safePosition != 0
    );

    quint64 time = QDateTime::currentMSecsSinceEpoch() - m_startTime;

    points.push_back({0, qreal(time), percent});
    points.push_back({1, qreal(time), m_mpi[0]->percent()});

    emit addPoints(chart, points);
}

void Program::button_set_position()
{
    m_isDacStopRequested = true;
    m_dacEventLoop->quit();
}

void Program::button_DO(quint8 DO_num, bool state)
{
    // if (!m_isInitialized) {
    //     if ((int)m_initialDoStates.size() < 4)
    //         m_initialDoStates.resize(4);

    //     m_initialDoStates[DO_num] = state;

    //     quint8 mask = 0;
    //     for (int i = 0; i < m_initialDoStates.size(); ++i)
    //         if (m_initialDoStates[i]) mask |= (1 << i);

    //     emit setDoButtonsChecked(mask);
    //     return;
    // }

    // m_mpi.setDiscreteOutput(DO_num, state);
    // emit setDoButtonsChecked(m_mpi.digitalOutputs());
}

void Program::checkbox_autoInit(int state)
{
    m_shouldWaitForButton = (state == 0);
}

void Program::terminateTest()
{
    m_isDacStopRequested = true;
    m_dacEventLoop->quit();
    emit stopTheTest();
}
