#pragma once

#include <QColor>
#include <QObject>
#include <QPointF>
#include <QEventLoop>
#include <QMessageBox>
#include <QTimer>

#include "Domain/Mpi/Device.h"

#include "Widgets/Chart/ChartType.h"
#include "Widgets/Chart/Point.h"


#include "Storage/Telemetry.h"

#include "Domain/Measurement/Sample.h"
#include "Domain/Measurement/TestDataBuffer.h"
#include "Domain/Tests/IAnalyzer.h"

#include "Domain/Tests/BaseRunner.h"

#include "Gui/Setup/SelectTests.h"
#include "DeviceConfig.h"

namespace Domain::Tests {
class AbstractScenario;
}

namespace Domain::Tests {
class Context;
}

namespace Domain::Tests::Main {
struct Params;
}

namespace Domain::Tests::Option {
struct Params;
}

namespace Domain::Tests::Option::Step {
struct Params;
struct Result;
}

namespace Widgets::Chart {
struct Point;
}

enum class TextObjects
{
    LineEdit_linearSensor,
    LineEdit_linearSensorPercent,
    LineEdit_pressureSensor_1,
    LineEdit_pressureSensor_2,
    LineEdit_pressureSensor_3,
    LineEdit_feedback_4_20mA,
};

namespace Domain {
class Program : public QObject
{
    Q_OBJECT
public:

    explicit Program(QObject *parent = nullptr);

    void setConfig(const Domain::DeviceConfig& deviceConfig) { m_deviceConfig = deviceConfig; }

    quint8 getDIStatus() { return m_device.digitalInputs(); }
    quint8 getDOStatus() { return m_device.digitalOutputs(); }

    enum class TestWorker
    {
        None,
        Stroke,
        Main,
        Response,
        Resolution,
        Step
    };


signals:
    void sampleReady(const Domain::Measurement::Sample& sample);
    void testStartRejected(const QString& reason);
    void telemetryUpdated(const Telemetry& telemetry);

    void errorOccured(const QString& error);

    void testActuallyStarted();

    void setSensorsMask(quint8 adcMask);

    void setText(const TextObjects object, const QString& text);
    void setTextColor(const TextObjects object, const QColor color);
    void setTask(qreal task);
    void setSensorNumber(quint8 num);
    void setGroupDOVisible(bool visible);
    void setVisible(Widgets::Chart::ChartType chart, quint16 series, bool visible);
    void setRegressionEnable(bool enable);

    void mainResultUpdated(const Domain::Tests::Main::Result& result);
    void strokeResultUpdated(const Domain::Tests::Stroke::Result& result);
    void stepResultUpdated(const Domain::Tests::Option::Step::Result& result);
    void crossingStatusUpdated(const CrossingStatus& status);

    void setDoButtonsChecked(quint8 status);

    void setDiCheckboxesChecked(quint8 status);

    void points(QVector<QVector<QPointF>>& points, Widgets::Chart::ChartType chartType);

    void addPoints(Widgets::Chart::ChartType chartType, const QVector<Widgets::Chart::Point>& points);
    void clearPoints(Widgets::Chart::ChartType chartType);

    void stopTheTest();
    void duplicateMainChartsSeries();
    void releaseBlock();

    bool question(QString& title, QString& text);

    void testFinished();

    void totalTestTimeMs(quint64 totalMs);
    void runnerFinished();

    void manualResumeRequired();

private:
    bool isDeviceReadyForTest() const;
    void failToStartTest(const QString& reason);
    DeviceConfig m_deviceConfig;
    SelectTests::PatternType m_patternType;

    // Sample
    Domain::Measurement::Sample makeSample() const;
    void updateRealtimeTexts(const Domain::Measurement::Sample& s);
    Domain::Measurement::TestDataBuffer m_testDataBuffer;

    std::unique_ptr<IAnalyzer> m_analyzer;
    TestWorker m_testWorker = TestWorker::None;
    std::unique_ptr<Domain::Tests::AbstractScenario> m_currentScenario;
    //
    void onRunnerActuallyStarted();

    // init
    void waitForDacCycle();
    void finalizeInitialization();

    bool m_suppressPublicTestFinished = false;

    void startScenario(std::unique_ptr<Domain::Tests::AbstractScenario> scenario);
    void connectScenarioRuntime(Domain::Tests::AbstractScenario* scenario);

    Tests::Context makeContext();

    QVector<quint16> makeRawValues(const QVector<quint16>& seq, bool normalOpen);
    QString seqToString(const QVector<quint16>& seq);

    Registry* m_registry;

    Domain::Mpi::Device m_device;

    Telemetry m_telemetry;
    QTimer* m_diPollTimer = nullptr;
    quint8 m_lastDiStatus = 0;
    QTimer* m_timerSensors;
    QTimer* m_timerDI;

    quint64 m_startTime;
    quint64 m_initTime;
    QEventLoop *m_dacEventLoop;

    bool m_isInitialized = false;
    bool m_isTestRunning = false;
    bool m_isDacStopRequested;
    bool m_shouldWaitForButton = false;
    QVector<bool> m_initialDoStates;
    QVector<bool> m_savedInitialDoStates;

private slots:
    void updateSensors();

public slots:
    void setMultipleDO(const QVector<bool>& states);

    void setDacRaw(quint16 dac,
                quint32 sleep_ms = 0,
                bool waitForStop = false,
                bool waitForStart = false);

    void initialization();

    // updateCharts
    void updateChartsFromSample(const Domain::Measurement::Sample& s);
    void updateMainCharts(const Domain::Measurement::Sample& s);
    void updateTimeChart(const Domain::Measurement::Sample& s, Widgets::Chart::ChartType chartType, qint64 time);

    void setInitDoStates(const QVector<bool>& states);
    void setPattern(SelectTests::PatternType pattern) { m_patternType = pattern; }

    void addRegression(const QVector<QPointF>& forwardPoints,
                       const QVector<QPointF>& backwardPoints);
    void addFriction(const QVector<QPointF>& points);

    void setDacReal(qreal value);

    void startStrokeTest();
    void startMainTest(const Domain::Tests::Main::Params& params);
    void startMainTestDoubleActing(const Domain::Tests::Main::Params& params);
    void continueTest();
    void startResponseTest(const Domain::Tests::Option::Params& params);
    void startResolutionTest(const Domain::Tests::Option::Params& params);
    void startStepTest(const Domain::Tests::Option::Step::Params& params);

    void endTest();
    void terminateTest();

    void button_set_position();
    void button_DO(quint8 DO_num, bool state);
    void checkbox_autoInit(int state);
};
}