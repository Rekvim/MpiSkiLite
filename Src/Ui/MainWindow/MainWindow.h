#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#pragma once
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QPointF>
#include <QThread>
#include <QDateTime>
#include <QDebug>
#include <QPlainTextEdit>
#include <QElapsedTimer>

#include "./Src/ReportBuilders/ReportSaver.h"
#include "Program.h"
#include "Registry.h"
#include "./Src/Telemetry/TelemetryStore.h"
#include "./Src/Ui/TestSettings/AbstractTestSettings.h"

// QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
// QT_END_NAMESPACE

enum class TestState {
    Idle, // ничего не происходит
    Starting, // подготовка, диалог параметров
    Running, // тест выполняется
    Finished, // успешно завершён
    Canceled // отменён пользователем
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void setRegistry(Registry *registry);
    void setPatternType(SelectTests::PatternType pattern) {
        m_patternType = pattern;
        updateAvailableTabs();
    }
signals:
    void initialized();
    void patternChanged(SelectTests::PatternType pattern);

    // void doInitStatesSelected(const QVector<bool> &states);
    void dacValueRequested(qreal value);

    void runMainTest();
    void runStrokeTest();
    void runOptionalTest(quint8 testNum);

    void stopTest();

    void setDo(quint8 doIndex, bool state);

private slots:
    void appendLog(const QString& text);

    void onTelemetryUpdated(const TelemetryStore &telemetry);

    void updateInitUI(const InitState& init);
    void updateMainTestUI(const TelemetryStore& t);
    void updateStrokeTestUI(const StrokeTestRecord& r);
    void updateCrossingUI(const TelemetryStore& t);

    void addPoints(Charts chart, const QVector<Point> &points);
    void clearPoints(Charts chart);

    void promptSaveChartsAfterTest();

    void showDots(bool visible);
    void duplicateMainChartsSeries();

    void getDirectory(const QString &currentPath, QString &result);

    void updateCrossingIndicators();

    void setText(TextObjects object, const QString &text);
    void setTask(qreal task);

    void setStepTestResults(const QVector<StepTest::TestResult> &results, quint32 T_value);

    void setChartVisible(Charts chart, quint16 series, bool visible);
    void setSensorsNumber(quint8 sensorCount);

    void setButtonInitEnabled(bool enabled);
    void setRegressionEnabled(bool enabled);

    void setDoButtonsChecked(quint8 bitmask);
    void setDiCheckboxesChecked(quint8 bitmask);

    void setTaskControlsEnabled(bool enabled);

    void onCountdownTimeout();
    void onTotalTestTimeMs(quint64 totalMs);

    void askQuestion(const QString &title, const QString &text, bool &result);

    void startTest();
    void endTest();

    void onStrokeTestPointsRequested(QVector<QVector<QPointF>> &points, Charts chart);
    void onMainTestPointsRequested(QVector<QVector<QPointF>> &points, Charts chart);
    void onStepTestPointsRequested(QVector<QVector<QPointF>> &points, Charts chart);

    void onMainTestParametersRequested(MainTestSettings::TestParameters &parameters);
    void onStepTestParametersRequested(StepTestSettings::TestParameters &parameters);
    void onResolutionTestParametersRequested(OtherTestSettings::TestParameters &parameters);
    void onResponseTestParametersRequested(OtherTestSettings::TestParameters &parameters);

    void on_pushButton_init_clicked();

    void on_pushButton_report_generate_clicked();
    void on_pushButton_report_open_clicked();

    void on_pushButton_mainTest_start_clicked();
    void on_pushButton_mainTest_save_clicked();

    void on_pushButton_strokeTest_start_clicked();
    void on_pushButton_strokeTest_save_clicked();

    void on_pushButton_optionalTests_start_clicked();
    void on_pushButton_optionalTests_save_clicked();

    void on_pushButton_imageChartTask_clicked();
    void on_pushButton_imageChartPressure_clicked();
    void on_pushButton_imageChartFriction_clicked();

    void on_pushButton_signal_4mA_clicked();
    void on_pushButton_signal_8mA_clicked();
    void on_pushButton_signal_12mA_clicked();
    void on_pushButton_signal_16mA_clicked();
    void on_pushButton_signal_20mA_clicked();

private:
    Ui::MainWindow *ui;
    TelemetryStore m_telemetryStore;

    QPlainTextEdit* m_logOutput = nullptr;
    void setTestState(TestState state);

    bool m_isInitialized = false;

    TestState m_testState = TestState::Idle;

    void lockTabsForPreInit();
    void updateAvailableTabs();
    void applyCrossingLimitsFromRecommend(const ValveInfo& valveInfo);
    QVector<Charts> chartsForCurrentTest() const;

    Registry *m_registry = nullptr;

    ReportSaver *m_reportSaver = nullptr;

    struct SeriesVisibilityBackup {
        QVector<bool> visible;
    };

    SeriesVisibilityBackup hideTaskAuxSeries();
    SeriesVisibilityBackup hidePressureAuxSeries();
    void restoreSeries(Charts chart, const SeriesVisibilityBackup& b);


    Program *m_program;
    QThread *m_programThread;

    QTimer* m_durationTimer;
    QElapsedTimer m_elapsedTimer;

    quint64 m_totalTestMs;

    ReportSaver::Report m_report;

    SelectTests::PatternType m_patternType = SelectTests::Pattern_None;

    QHash<TextObjects, QLineEdit *> m_lineEdits;
    QHash<Charts, MyChart *> m_charts;

    QVector<AbstractTestSettings*> m_testSettings;

    MainTestSettings *m_mainTestSettings;
    StepTestSettings *m_stepTestSettings;
    OtherTestSettings *m_responseTestSettings;
    OtherTestSettings *m_resolutionTestSettings;

    QImage m_imageChartTask;
    QImage m_imageChartPressure;
    QImage m_imageChartFriction;
    QImage m_imageChartResponse;
    QImage m_imageChartResolution;
    QImage m_imageChartStep;

    void syncTaskChartSeriesVisibility(quint8 sensorCount);
    void displayDependingPattern();
    void triggerPrimaryAction();
    QTabWidget* currentInnerTabWidget() const;

    void initCharts();
    void saveChart(Charts chart);
    void getImage(QLabel* label, QImage* image);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
};
#endif // MAINWINDOW_H
