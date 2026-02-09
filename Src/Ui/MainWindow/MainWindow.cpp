#include "MainWindow.h"
#include "./Src/CustomChart/MyChart.h"
#include "ui_MainWindow.h"

#include "Src/ReportBuilders/ReportBuilder.h"
#include "./Src/Ui/TestSettings/AbstractTestSettings.h"

namespace {
static QString formatRange(double lo, double hi, int prec = 2)
{
    if (lo > hi) std::swap(lo, hi);
    return QString("%1–%2")
        .arg(lo, 0, 'f', prec)
        .arg(hi, 0, 'f', prec);
}

double toDouble(QString s, bool* okOut = nullptr)
{
    s = s.trimmed();
    s.replace(',', '.');
    bool ok = false;
    const double v = QLocale::c().toDouble(s, &ok);
    if (okOut) *okOut = ok;
    return v;
}

void setNum(QLineEdit* le, double v, int prec = 2)
{
    le->setText(QString::number(v, 'f', prec));
}

void setPlusMinusPercent(QLineEdit* loLe, QLineEdit* hiLe,
                         double base, double pct, int prec = 2)
{
    const double d = std::abs(base) * (pct / 100.0);
    setNum(loLe, base - d, prec);
    setNum(hiLe, base + d, prec);
}
} // namespace

static void switchTab(QTabWidget* tw, int dir)
{
    if (!tw) return;
    int i = tw->currentIndex();
    int n = tw->count();
    tw->setCurrentIndex((i + dir + n) % n);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    auto bindTab = [&](int key, QWidget* tab) {
        auto* sc = new QShortcut(QKeySequence(QString::number(key)), this);
        sc->setContext(Qt::ApplicationShortcut);
        connect(sc, &QShortcut::activated, this, [=] {
            const int idx = ui->tabWidget->indexOf(tab);
            if (idx >= 0 && ui->tabWidget->isTabEnabled(idx)) {
                ui->tabWidget->setCurrentIndex(idx);
            }
        });
    };
    bindTab(1, ui->tab_manual);
    bindTab(2, ui->tab_strokeTest);
    bindTab(3, ui->tab_mainTests);
    bindTab(4, ui->tab_optionalTests);
    bindTab(5, ui->tab_reportGeneration);

    auto* left = new QShortcut(QKeySequence(Qt::Key_Left), this);
    left->setContext(Qt::ApplicationShortcut);
    connect(left, &QShortcut::activated, this, [&] {
        if (auto* tw = currentInnerTabWidget())
            switchTab(tw, -1);
    });

    auto* right = new QShortcut(QKeySequence(Qt::Key_Right), this);
    right->setContext(Qt::ApplicationShortcut);
    connect(right, &QShortcut::activated, this, [&] {
        if (auto* tw = currentInnerTabWidget())
            switchTab(tw, +1);
    });

    new QShortcut(QKeySequence(Qt::Key_Home), this, [&] {
        if (auto* tw = currentInnerTabWidget())
            tw->setCurrentIndex(0);
    });

    new QShortcut(QKeySequence(Qt::Key_End), this, [&] {
        if (auto* tw = currentInnerTabWidget())
            tw->setCurrentIndex(tw->count() - 1);
    });

    auto* enter = new QShortcut(QKeySequence(Qt::Key_Return), this);
    enter->setContext(Qt::ApplicationShortcut);

    connect(enter, &QShortcut::activated, this,
            &MainWindow::triggerPrimaryAction);

    QMap<int, QString> labels;

    labels[3000] = "3 мА";
    labels[4000] = "0% – 4 мА";
    labels[5000] = "5 мА";
    labels[6000] = "6 мА";
    labels[7000] = "7 мА";
    labels[8000] = "25% – 8 мА";
    labels[9000] = "9 мА";
    labels[10000] = "10 мА";
    labels[11000] = "11 мА";
    labels[12000] = "50% – 12 мА";
    labels[13000] = "13 мА";
    labels[14000] = "14 мА";
    labels[15000] = "15 мА";
    labels[16000] = "75% – 16 мА";
    labels[17000] = "17 мА";
    labels[18000] = "18 мА";
    labels[19000] = "19 мА";
    labels[20000] = "100% – 20 мА";
    labels[21000] = "21 мА";

    auto* s = qobject_cast<LabeledSlider*>(ui->verticalSlider_task);

    if (s) {
        s->setTickLabels(labels);
        s->setTickGap(6);
        s->setTickLength(10);
        s->setLabelOffset(8);

        QTimer::singleShot(0, s, [s]{
            s->setFixedWidth(s->sizeHint().width());
        });
    }

    ui->tabWidget->setCurrentIndex(0);

    lockTabsForPreInit();

    m_mainTestSettings = new MainTestSettings(this);
    m_stepTestSettings = new StepTestSettings(this);
    m_responseTestSettings = new OtherTestSettings(this);
    m_resolutionTestSettings = new OtherTestSettings(this);

    m_testSettings = {
        m_stepTestSettings,
        m_responseTestSettings,
        m_resolutionTestSettings
    };

    m_reportSaver = new ReportSaver(this);


    ui->checkBox_switch_3_0->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->checkBox_switch_0_3->setAttribute(Qt::WA_TransparentForMouseEvents);

    m_lineEdits[TextObjects::LineEdit_linearSensor] = ui->lineEdit_linearSensor;
    m_lineEdits[TextObjects::LineEdit_linearSensorPercent] = ui->lineEdit_linearSensorPercent;
    m_lineEdits[TextObjects::LineEdit_pressureSensor_1] = ui->lineEdit_pressureSensor_1;
    m_lineEdits[TextObjects::LineEdit_pressureSensor_2] = ui->lineEdit_pressureSensor_2;
    m_lineEdits[TextObjects::LineEdit_pressureSensor_3] = ui->lineEdit_pressureSensor_3;
    m_lineEdits[TextObjects::LineEdit_feedback_4_20mA] = ui->lineEdit_feedback_4_20mA;

    m_program = new Program;
    m_programThread = new QThread(this);
    m_program->moveToThread(m_programThread);

    // kоговое окно
    // logOutput = new QPlainTextEdit(this);
    // logOutput->setReadOnly(true);
    // logOutput->setStyleSheet("font-size: 8pt;");

    // auto *dock = new QDockWidget(tr("Лог"), this);
    // dock->setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);
    // dock->setWidget(logOutput);

    // dock->setMinimumWidth(300);
    // // dock->resize(300, dock->height());

    // addDockWidget(Qt::RightDockWidgetArea, dock);

    // connect(m_program, &Program::errorOccured,
    //         this, &MainWindow::appendLog,
    //         Qt::QueuedConnection);

    // appendLog("Логовое окно инициализировано");

    m_durationTimer = new QTimer(this);
    m_durationTimer->setInterval(1000);

    connect(m_durationTimer, &QTimer::timeout,
            this, &MainWindow::onCountdownTimeout);

    connect(m_program, &Program::totalTestTimeMs,
            this, &MainWindow::onTotalTestTimeMs);

    connect(this, &MainWindow::initialized,
            m_program, &Program::initialization);

    // connect(this, &MainWindow::doInitStatesSelected,
    //         m_program, &Program::setInitDOStates);

    connect(ui->pushButton_set, &QPushButton::clicked,
            m_program, &Program::button_set_position);

    connect(ui->checkBox_autoinit, &QCheckBox::checkStateChanged,
            m_program, &Program::checkbox_autoInit);

    connect(this, &MainWindow::setDo,
            m_program, &Program::button_DO);

    for (int i = 0; i < 4; ++i) {
        auto btn = findChild<QPushButton*>(QString("pushButton_DO%1").arg(i));
        if (!btn) continue;

        connect(btn, &QPushButton::clicked,
                this, [this, i](bool checked)
                {
                    emit setDo(i, checked);
                });
    }

    connect(this, &MainWindow::runMainTest,
            m_program, &Program::startMainTest);

    // connect(m_program, &Program::mainTestFinished,
    //         this, &MainWindow::promptSaveCharts);

    connect(this, &MainWindow::runStrokeTest,
            m_program, &Program::startStrokeTest);

    connect(this, &MainWindow::runOptionalTest,
            m_program, &Program::startOptionalTest);

    connect(this, &MainWindow::stopTest,
            m_program, &Program::terminateTest);

    connect(m_program, &Program::stopTheTest,
            this, &MainWindow::endTest);

    connect(m_program, &Program::setText,
            this, &MainWindow::setText);

    connect(m_program, &Program::setDoButtonsChecked,
            this, &MainWindow::setDoButtonsChecked);

    connect(m_program, &Program::setDiCheckboxesChecked,
            this, &MainWindow::setDiCheckboxesChecked);

    connect(this, &MainWindow::dacValueRequested,
            m_program, &Program::setDAC_real);

    connect(ui->doubleSpinBox_task,
            qOverload<double>(&QDoubleSpinBox::valueChanged),
            this,[&](double value) {
                if (qRound(value * 1000) != ui->verticalSlider_task->value()) {
                    if (ui->verticalSlider_task->isEnabled())
                        emit dacValueRequested(value);
                    ui->verticalSlider_task->setValue(qRound(value * 1000));
                }
            });

    connect(ui->verticalSlider_task, &QSlider::valueChanged,
            this, [&](int value) {
                if (qRound(ui->doubleSpinBox_task->value() * 1000) != value) {
                    if (ui->doubleSpinBox_task->isEnabled())
                        emit dacValueRequested(value / 1000.0);
                    ui->doubleSpinBox_task->setValue(value / 1000.0);
                }
            });

    connect(this, &MainWindow::patternChanged,
            m_program, &Program::setPattern);

    connect(m_program, &Program::setTask,
            this, &MainWindow::setTask);

    connect(m_program, &Program::setSensorNumber,
            this, &MainWindow::setSensorsNumber);

    connect(m_program, &Program::setButtonInitEnabled,
            this, &MainWindow::setButtonInitEnabled);

    connect(m_program, &Program::setTaskControlsEnabled,
            this, &MainWindow::setTaskControlsEnabled);

    connect(m_program, &Program::setStepResults,
            this, &MainWindow::setStepTestResults);

    connect(m_program, &Program::getParameters_mainTest,
            this, &MainWindow::onMainTestParametersRequested,
            Qt::BlockingQueuedConnection);

    connect(m_program, &Program::getParameters_stepTest,
            this, &MainWindow::onStepTestParametersRequested,
            Qt::BlockingQueuedConnection);

    connect(m_program, &Program::getParameters_resolutionTest,
            this, &MainWindow::onResolutionTestParametersRequested,
            Qt::BlockingQueuedConnection);

    connect(m_program, &Program::getParameters_responseTest,
            this, &MainWindow::onResponseTestParametersRequested,
            Qt::BlockingQueuedConnection);

    connect(m_program, &Program::question,
            this, &MainWindow::askQuestion,
            Qt::BlockingQueuedConnection);

    connect(m_reportSaver, &ReportSaver::question,
            this, &MainWindow::askQuestion,
            Qt::DirectConnection);

    connect(m_reportSaver, &ReportSaver::getDirectory,
            this, &MainWindow::getDirectory,
            Qt::DirectConnection);

    connect(ui->checkBox_autoinit, &QCheckBox::checkStateChanged,
            this, [&](int state) {
                ui->pushButton_set->setEnabled(!state);
            });

    ui->tableWidget_stepResults->setColumnCount(2);
    ui->tableWidget_stepResults->setHorizontalHeaderLabels({QLatin1String("T86"), tr("Перерегулирование")});
    ui->tableWidget_stepResults->resizeColumnsToContents();

    ui->toolButton_arrowUp->setIcon(QIcon(":/Src/Img/arrowUp.png"));
    ui->toolButton_arrowUp->setIconSize(ui->toolButton_arrowUp->size());
    ui->toolButton_arrowUp->setFixedSize(100, 60);
    ui->toolButton_arrowUp->setIconSize(QSize(90, 50));
    ui->toolButton_arrowUp->setText(QString());
    ui->toolButton_arrowUp->setAutoRepeat(true);
    ui->toolButton_arrowUp->setAutoRepeatDelay(300);
    ui->toolButton_arrowUp->setAutoRepeatInterval(100);

    ui->toolButton_arrowUp->setStyleSheet(
        "QToolButton {"
        "   background-color: transparent;"
        "   border: none;"
        "   padding: 0px;"
        "   margin: 0px;"
        "}"
        "QToolButton:hover {"
        "   background-color: transparent;"
        "}"
        "QToolButton:pressed {"
        "   background-color: transparent;"
        "}"
    );

    connect(ui->toolButton_arrowUp, &QToolButton::clicked,
            this, [this]() {
                double cur = ui->doubleSpinBox_task->value();
                double nxt = cur + 0.05;
                if (nxt > ui->doubleSpinBox_task->maximum())
                    nxt = ui->doubleSpinBox_task->maximum();
                ui->doubleSpinBox_task->setValue(nxt);
                if (ui->doubleSpinBox_task->isEnabled())
                    emit dacValueRequested(nxt);
            });
    ui->toolButton_arrowUp->installEventFilter(this);

    ui->toolButton_arrowDown->setIcon(QIcon(":/Src/Img/arrowDown.png"));
    ui->toolButton_arrowDown->setIconSize(ui->toolButton_arrowDown->size());
    ui->toolButton_arrowDown->setFixedSize(100, 60);
    ui->toolButton_arrowDown->setIconSize(QSize(90, 50));
    ui->toolButton_arrowDown->setText(QString());
    ui->toolButton_arrowDown->setAutoRepeat(true);
    ui->toolButton_arrowDown->setAutoRepeatDelay(300);
    ui->toolButton_arrowDown->setAutoRepeatInterval(100);

    ui->toolButton_arrowDown->setStyleSheet(
        "QToolButton {"
        "   background-color: transparent;"
        "   border: none;"
        "   padding: 0px;"
        "   margin: 0px;"
        "}"
        "QToolButton:hover {"
        "   background-color: transparent;"
        "}"
        "QToolButton:pressed {"
        "   background-color: transparent;"
        "}"
        );

    ui->toolButton_arrowDown->installEventFilter(this);

    connect(ui->toolButton_arrowDown, &QToolButton::clicked,
            this, [this]() {
                double cur = ui->doubleSpinBox_task->value();
                double nxt = cur - 0.05;
                if (nxt < ui->doubleSpinBox_task->minimum())
                    nxt = ui->doubleSpinBox_task->minimum();
                ui->doubleSpinBox_task->setValue(nxt);
                if (ui->doubleSpinBox_task->isEnabled())
                    emit dacValueRequested(nxt);
            });

    connect(m_program, &Program::telemetryUpdated,
            this, &MainWindow::onTelemetryUpdated,
            Qt::QueuedConnection);

    connect(m_program, &Program::testFinished,
            this, &MainWindow::endTest);

    ui->tabWidget_mainTests->setCurrentIndex(0);
    ui->tabWidget_optionalTests->setCurrentIndex(0);
    ui->tabWidget_reportGeneration->setCurrentIndex(0);

    connect(ui->tabWidget, &QTabWidget::currentChanged,
            this, [this](int) {
                ui->tabWidget_mainTests->setCurrentIndex(0);
                ui->tabWidget_optionalTests->setCurrentIndex(0);
                ui->tabWidget_reportGeneration->setCurrentIndex(0);
            });
}

MainWindow::~MainWindow()
{
    QMetaObject::invokeMethod(
        m_program, "terminateTest",
        Qt::BlockingQueuedConnection);

    m_programThread->quit();
    m_programThread->wait();

    delete ui;
}

void MainWindow::lockTabsForPreInit()
{
    ui->tabWidget->setTabEnabled(ui->tabWidget->indexOf(ui->tab_mainTests), false);
    ui->tabWidget->setTabEnabled(1, false);
    ui->tabWidget->setTabEnabled(2, false);
    ui->tabWidget->setTabEnabled(3, false);
}

QTabWidget* MainWindow::currentInnerTabWidget() const
{
    QWidget* top = ui->tabWidget->currentWidget();

    if (top == ui->tab_mainTests)
        return ui->tabWidget_mainTests;

    if (top == ui->tab_optionalTests)
        return ui->tabWidget_optionalTests;

    if (top == ui->tab_reportGeneration)
        return ui->tabWidget_reportGeneration;

    return nullptr;
}

void MainWindow::updateAvailableTabs()
{
    displayDependingPattern();
    if (!m_isInitialized) {
        lockTabsForPreInit();
        return;
    }
}

static QString formatHMS(quint64 ms)
{
    const quint64 totalSec = ms / 1000ULL;
    const quint64 h = totalSec / 3600ULL;
    const quint64 m = (totalSec % 3600ULL) / 60ULL;
    const quint64 s = totalSec % 60ULL;

    return QString("%1:%2:%3")
        .arg(h, 2, 10, QChar('0'))
        .arg(m, 2, 10, QChar('0'))
        .arg(s, 2, 10, QChar('0'));
}

void MainWindow::onCountdownTimeout()
{
    const qint64 elapsed = m_elapsedTimer.elapsed();
    qint64 remaining = static_cast<qint64>(m_totalTestMs) - elapsed;
    if (remaining < 0) remaining = 0;

    ui->statusbar->showMessage(
        tr("Тест в процессе. До завершения теста осталось: %1 (прошло %2 из %3)")
            .arg(formatHMS(static_cast<quint64>(remaining)),
                 formatHMS(static_cast<quint64>(elapsed)),
                 formatHMS(m_totalTestMs))
        );

    if (remaining == 0)
        m_durationTimer->stop();
}

void MainWindow::onTotalTestTimeMs(quint64 totalMs)
{
    m_totalTestMs = totalMs;
    m_elapsedTimer.restart();

    ui->statusbar->showMessage(
        tr("Плановая длительность теста: %1").arg(formatHMS(m_totalTestMs))
        );

    m_durationTimer->setInterval(1000);
    m_durationTimer->start();
    onCountdownTimeout();
}

static void setIndicatorColor(QWidget* widget, const QString& color, const QString& border) {
    widget->setStyleSheet(QString(
                              "background: %1;"
                              "border: 4px solid %2;"
                              "border-radius: 10px;"
                              ).arg(color, border));
}

static void setIndicatorByState(QWidget* widget, CrossingStatus::State state)
{
    using State = CrossingStatus::State;

    switch (state) {
    case State::Unknown:
        setIndicatorColor(widget,
                          QLatin1String("#A0A0A0"),
                          QLatin1String("#505050"));
        break;
    case State::Ok:
        setIndicatorColor(widget,
                          QLatin1String("#4E8448"),
                          QLatin1String("#16362B"));
        break;
    case State::Fail:
        setIndicatorColor(widget,
                          QLatin1String("#B80F0F"),
                          QLatin1String("#510000"));
        break;
    }
}

void MainWindow::applyCrossingLimitsFromRecommend(const ValveInfo& valveInfo)
{
    const CrossingLimits& limits = valveInfo.crossingLimits;

    if (limits.rangeEnabled) {
        bool ok = false;
        const double stroke = toDouble(valveInfo.strokValve, &ok);
        if (ok) {
            setPlusMinusPercent(ui->lineEdit_crossingLimits_range_lowerLimit,
                                ui->lineEdit_crossingLimits_range_upperLimit,
                                stroke, limits.rangeUpperLimit);
        }
    }

    if (limits.springEnabled) {

        // Берём значения из чисел (как ты и хотел)
        double low = valveInfo.driveRangeLow;
        double high = valveInfo.driveRangeHigh;

        // Если по смыслу эти величины не могут быть отрицательными —
        // нормализуем (иначе получишь -1.28 и т.п.)
        low = std::abs(low);
        high = std::abs(high);

        if (low > high)
            std::swap(low, high);

        // Допуски ВОКРУГ каждого числа отдельно (как было у тебя)
        const double lowDelta = low * (limits.springLowerLimit / 100.0);
        const double highDelta = high * (limits.springUpperLimit / 100.0);

        double lowLo = low - lowDelta;
        double lowHi = low + lowDelta;

        double highLo = high - highDelta;
        double highHi = high + highDelta;

        // Запрещаем отрицательные границы (если физически не бывает < 0)
        lowLo = std::max(0.0, lowLo);
        highLo = std::max(0.0, highLo);

        // На всякий случай порядок
        if (lowLo > lowHi) std::swap(lowLo, lowHi);
        if (highLo > highHi) std::swap(highLo, highHi);

        ui->lineEdit_crossingLimits_spring_lowerLimit->setText(formatRange(lowLo, lowHi));
        ui->lineEdit_crossingLimits_spring_upperLimit->setText(formatRange(highLo, highHi));
    }


    if (limits.dynamicErrorEnabled) {
        ui->lineEdit_crossingLimits_dynamicError_lowerLimit->setText(QStringLiteral("0"));
        ui->lineEdit_crossingLimits_dynamicError_upperLimit->setText(
            QString::number(valveInfo.dinamicErrorRecomend, 'f', 2));
    }
}

void MainWindow::updateCrossingIndicators()
{
    const auto &cs = m_telemetryStore.crossingStatus;

    setIndicatorByState(ui->widget_crossingLimits_coefficientFriction_limitStatusIndicator,
                        cs.frictionPercent);
    setIndicatorByState(ui->widget_crossingLimits_linearCharacteristic_limitStatusIndicator,
                        cs.linearCharacteristic);
    setIndicatorByState(ui->widget_crossingLimits_range_limitStatusIndicator,
                        cs.range);
    setIndicatorByState(ui->widget_crossingLimits_spring_limitStatusIndicator,
                        cs.spring);
    setIndicatorByState(ui->widget_crossingLimits_dynamicError_limitStatusIndicator,
                        cs.dynamicError);
}

void MainWindow::onTelemetryUpdated(const TelemetryStore &t) {

    m_telemetryStore = t;

    updateInitUI(t.init);
    updateMainTestUI(t);
    updateStrokeTestUI(t.strokeTestRecord);
    updateCrossingUI(t);
}

void MainWindow::updateInitUI(const InitState& init)
{
    ui->label_deviceStatusValue->setText(init.deviceStatusText);
    ui->label_deviceStatusValue->setStyleSheet(
        "color:" + init.deviceStatusColor.name(QColor::HexRgb));

    ui->label_deviceInitValue->setText(init.initStatusText);
    ui->label_deviceInitValue->setStyleSheet(
        "color:" + init.initStatusColor.name(QColor::HexRgb));

    ui->label_connectedSensorsNumber->setText(init.connectedSensorsText);
    ui->label_connectedSensorsNumber->setStyleSheet(
        "color:" + init.connectedSensorsColor.name(QColor::HexRgb));

    ui->label_startingPositionValue->setText(init.startingPositionText);
    ui->label_startingPositionValue->setStyleSheet(
        "color:" + init.startingPositionColor.name(QColor::HexRgb));

    ui->label_finalPositionValue->setText(init.finalPositionText);
    ui->label_finalPositionValue->setStyleSheet(
        "color:" + init.finalPositionColor.name(QColor::HexRgb));
}

void MainWindow::updateMainTestUI(const TelemetryStore& t)
{
    ui->label_pressureDifferenceValue->setText(
        QString("%1")
            .arg(t.mainTestRecord.pressureDifference, 0, 'f', 3)
        );
    ui->label_frictionForceValue->setText(
        QString("%1")
            .arg(t.mainTestRecord.frictionForce, 0, 'f', 3)
        );
    ui->label_frictionPercentValue->setText(
        QString("%1")
            .arg(t.mainTestRecord.frictionPercent, 0, 'f', 2)
        );
    ui->lineEdit_resultsTable_frictionForceValue->setText(
        QString("%1")
            .arg(t.mainTestRecord.frictionForce, 0, 'f', 3)
        );
    ui->lineEdit_resultsTable_frictionPercentValue->setText(
        QString("%1")
            .arg(t.mainTestRecord.frictionPercent, 0, 'f', 2)
        );

    ui->label_dynamicErrorMeanPercent->setText(
        QString("%1 %")
            .arg(t.mainTestRecord.dynamicError_meanPercent, 0, 'f', 2)
        );
    ui->label_dynamicErrorMean->setText(
        QString("%1 mA")
            .arg(t.mainTestRecord.dynamicError_mean, 0, 'f', 3)
        );
    ui->label_dynamicErrorMaxPercent->setText(
        QString("%1 %")
            .arg(t.mainTestRecord.dynamicError_maxPercent, 0, 'f', 2)
        );

    ui->label_dynamicErrorMax->setText(
        QString("%1 mA")
            .arg(t.mainTestRecord.dynamicError_max, 0, 'f', 3)
        );
    ui->lineEdit_resultsTable_dynamicErrorReal->setText(
        QString("%1")
            .arg(t.mainTestRecord.dynamicErrorReal, 0, 'f', 2)
        );

    ui->label_dynamicErrorMax->setText(
        QString("%1 bar")
            .arg(t.mainTestRecord.lowLimitPressure, 0, 'f', 2)
        );
    ui->label_dynamicErrorMax->setText(
        QString("%1 bar")
            .arg(t.mainTestRecord.highLimitPressure, 0, 'f', 2)
        );

    ui->label_valveStroke_range->setText(
        QString("%1")
            .arg(t.valveStrokeRecord.range)
        );

    // StrokeRecord
    ui->lineEdit_resultsTable_strokeReal->setText(
        QString("%1").arg(t.valveStrokeRecord.real, 0, 'f', 2));

    ui->label_lowLimitValue->setText(
        QString("%1")
            .arg(t.mainTestRecord.lowLimitPressure)
        );
    ui->label_highLimitValue->setText(
        QString("%1")
            .arg(t.mainTestRecord.highLimitPressure)
        );

    ui->lineEdit_resultsTable_rangePressure->setText(
        QString("%1–%2")
            .arg(t.mainTestRecord.lowLimitPressure, 0, 'f', 2)
            .arg(t.mainTestRecord.highLimitPressure, 0, 'f', 2)
        );

    ui->lineEdit_resultsTable_driveRangeReal->setText(
        QString("%1–%2")
            .arg(t.mainTestRecord.springLow, 0, 'f', 2)
            .arg(t.mainTestRecord.springHigh, 0, 'f', 2)
        );
}

void MainWindow::updateStrokeTestUI(const StrokeTestRecord& r)
{
    ui->lineEdit_strokeTest_forwardTime->setText(r.timeForwardMs);
    ui->lineEdit_resultsTable_strokeTest_forwardTime->setText(r.timeForwardMs);

    ui->lineEdit_strokeTest_backwardTime->setText(r.timeBackwardMs);
    ui->lineEdit_resultsTable_strokeTest_backwardTime->setText(r.timeBackwardMs);
}

void MainWindow::updateCrossingUI(const TelemetryStore& t)
{
    ui->lineEdit_crossingLimits_dynamicError_value->setText(
        QString::number(t.mainTestRecord.dynamicErrorReal, 'f', 2));

    ui->lineEdit_crossingLimits_linearCharacteristic_value->setText(
        QString::number(t.mainTestRecord.linearityError, 'f', 2));

    ui->lineEdit_crossingLimits_range_value->setText(
        QString::number(t.valveStrokeRecord.real, 'f', 2));

    ui->lineEdit_crossingLimits_spring_value->setText(
        QString("%1–%2")
            .arg(t.mainTestRecord.springLow, 0, 'f', 2)
            .arg(t.mainTestRecord.springHigh, 0, 'f', 2));

    ui->lineEdit_crossingLimits_coefficientFriction_value->setText(
        QString::number(t.mainTestRecord.frictionPercent, 'f', 2));

    updateCrossingIndicators();
}


void MainWindow::appendLog(const QString& text) {
    const QString stamp = QDateTime::currentDateTime()
    .toString("[hh:mm:ss.zzz] ");
    m_logOutput->appendPlainText(stamp + text);
}

void MainWindow::on_pushButton_signal_4mA_clicked()
{
    ui->doubleSpinBox_task->setValue(4.0);
}
void MainWindow::on_pushButton_signal_8mA_clicked()
{
    ui->doubleSpinBox_task->setValue(8.0);
}
void MainWindow::on_pushButton_signal_12mA_clicked()
{
    ui->doubleSpinBox_task->setValue(12.0);
}
void MainWindow::on_pushButton_signal_16mA_clicked()
{
    ui->doubleSpinBox_task->setValue(16.0);
}
void MainWindow::on_pushButton_signal_20mA_clicked()
{
    ui->doubleSpinBox_task->setValue(20.0);
}

void MainWindow::setTaskControlsEnabled(bool enabled)
{
    ui->verticalSlider_task->setEnabled(enabled);
    ui->doubleSpinBox_task->setEnabled(enabled);
    ui->groupBox_DO->setEnabled(enabled);
    ui->groupBox_SettingCurrentSignal->setEnabled(enabled);
}

void MainWindow::triggerPrimaryAction()
{
    QWidget* top = ui->tabWidget->currentWidget();

    if (top == ui->tab_manual)
        ui->pushButton_init->click();

    if (top == ui->tab_strokeTest)
        ui->pushButton_strokeTest_start->click();

    else if (top == ui->tab_mainTests)
        ui->pushButton_mainTest_start->click();

    else if (top == ui->tab_optionalTests)
        ui->pushButton_optionalTests_start->click();

    else if (top == ui->tab_reportGeneration)
        ui->pushButton_report_generate->click();
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->toolButton_arrowUp) {
        if (event->type() == QEvent::Enter) {
            ui->toolButton_arrowUp->setIcon(QIcon(":/Src/Img/arrowUpHover.png"));
            return true;
        }
        if (event->type() == QEvent::Leave) {
            ui->toolButton_arrowUp->setIcon(QIcon(":/Src/Img/arrowUp.png"));
            return true;
        }
    }

    if (watched == ui->toolButton_arrowDown) {
        if (event->type() == QEvent::Enter) {
            ui->toolButton_arrowDown->setIcon(QIcon(":/Src/Img/arrowDownHover.png"));
            return true;
        }
        if (event->type() == QEvent::Leave) {
            ui->toolButton_arrowDown->setIcon(QIcon(":/Src/Img/arrowDown.png"));
            return true;
        }
    }

    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::setRegistry(Registry *registry)
{
    m_registry = registry;

    const auto& objectInfo = m_registry->objectInfo();
    const auto& valveInfo = m_registry->valveInfo();
    const auto& materialInfo = m_registry->materialsOfComponentParts();
    const auto& otherParameters = m_registry->otherParameters();

    const CrossingLimits &limits = valveInfo.crossingLimits;

    ui->lineEdit_date->setText(otherParameters.date);

    ui->lineEdit_object->setText(objectInfo.object);
    ui->lineEdit_manufacture->setText(objectInfo.manufactory);
    ui->lineEdit_department->setText(objectInfo.department);
    ui->lineEdit_FIO->setText(objectInfo.FIO);

    ui->lineEdit_positionNumber->setText(valveInfo.positionNumber);
    ui->lineEdit_manufacturer->setText(valveInfo.manufacturer);
    ui->lineEdit_valveModel->setText(valveInfo.valveModel);
    ui->lineEdit_serialNumber->setText(valveInfo.serialNumber);
    ui->lineEdit_CV->setText(materialInfo.CV);

    ui->lineEdit_DNPN->setText(QString("%1 / %2").arg(valveInfo.DN, valveInfo.PN));
    ui->lineEdit_driveModel->setText(valveInfo.driveModel);
    ui->lineEdit_positionerModel->setText(valveInfo.positionerModel);
    ui->lineEdit_strokeMovement->setText(otherParameters.strokeMovement);
    ui->lineEdit_safePosition->setText(otherParameters.safePosition);
    ui->lineEdit_resultsTable_dynamicErrorRecomend->setText(QString::number(valveInfo.dinamicErrorRecomend, 'f', 2));
    ui->lineEdit_materialStuffingBoxSeal->setText(valveInfo.materialStuffingBoxSeal);

    ui->lineEdit_materialStuffingBoxSeal->setText(materialInfo.stuffingBoxSeal);
    ui->lineEdit_materialCap->setText(materialInfo.cap);
    ui->lineEdit_materialCorpus->setText(materialInfo.corpus);
    ui->lineEdit_materialSaddle->setText(materialInfo.saddle);
    ui->lineEdit_materialBall->setText(materialInfo.ball);
    ui->lineEdit_materialDisk->setText(materialInfo.disk);
    ui->lineEdit_materialPlunger->setText(materialInfo.plunger);
    ui->lineEdit_materialShaft->setText(materialInfo.shaft);
    ui->lineEdit_materialStock->setText(materialInfo.stock);
    ui->lineEdit_materialGuideSleeve->setText(materialInfo.guideSleeve);


    const bool anyCrossingEnabled =
        limits.frictionEnabled
        || limits.linearCharacteristicEnabled
        || limits.rangeEnabled
        || limits.springEnabled
        || limits.dynamicErrorEnabled;

    ui->groupBox_crossingLimits->setVisible(anyCrossingEnabled);

    ui->lineEdit_resultsTable_strokeRecomend->setText(valveInfo.strokValve);

    ui->lineEdit_resultsTable_driveRangeRecomend->setText(
        QString("%1–%2")
            .arg(valveInfo.driveRangeLow, 0, 'f', 2)
            .arg(valveInfo.driveRangeHigh, 0, 'f', 2)
        );
    ui->widget_crossingLimits_frictionForce->setVisible(limits.frictionEnabled);
    ui->widget_crossingLimits_linearCharacteristic->setVisible(limits.linearCharacteristicEnabled);
    ui->widget_crossingLimits_range->setVisible(limits.rangeEnabled);
    ui->widget_crossingLimits_spring->setVisible(limits.springEnabled);
    ui->widget_crossingLimits_dynamicError->setVisible(limits.dynamicErrorEnabled);

    if (limits.frictionEnabled) {
        ui->lineEdit_crossingLimits_coefficientFriction_lowerLimit->setText(
            QString::number(limits.frictionCoefLowerLimit, 'f', 2));
        ui->lineEdit_crossingLimits_coefficientFriction_upperLimit->setText(
            QString::number(limits.frictionCoefUpperLimit, 'f', 2));
    }

    if (limits.linearCharacteristicEnabled) {
        ui->lineEdit_crossingLimits_linearCharacteristic_lowerLimit->setText(QStringLiteral("0"));
        ui->lineEdit_crossingLimits_linearCharacteristic_upperLimit->setText(
            QString::number(limits.linearCharacteristicLowerLimit, 'f', 2));
    }

    applyCrossingLimitsFromRecommend(valveInfo);

    if (limits.dynamicErrorEnabled) {
        ui->lineEdit_crossingLimits_dynamicError_lowerLimit->setText(QStringLiteral("0"));
        ui->lineEdit_crossingLimits_dynamicError_upperLimit->setText(QString::number(valveInfo.dinamicErrorRecomend, 'f', 2));
    }

    for (AbstractTestSettings* s : m_testSettings)
        s->applyValveInfo(valveInfo);

    initCharts();

    m_program->setRegistry(registry);
    m_programThread->start();

    m_reportSaver->setRegistry(registry);
}

void MainWindow::setText(TextObjects object, const QString &text)
{
    if (m_lineEdits.contains(object)) {
        m_lineEdits[object]->setText(text);
    }
}

void MainWindow::setTask(qreal task)
{
    quint16 i_task = qRound(task * 1000);

    if (ui->doubleSpinBox_task->value() != i_task / 1000.0) {
        ui->doubleSpinBox_task->setValue(i_task / 1000.0);
    }

    if (ui->verticalSlider_task->value() != i_task) {
        ui->verticalSlider_task->setSliderPosition(i_task);
    }
}

void MainWindow::setStepTestResults(const QVector<StepTest::TestResult> &results, quint32 T_value)
{
    ui->tableWidget_stepResults->setHorizontalHeaderLabels(
        {tr(("T%1")).arg(T_value), tr("Перерегулирование")});

    ui->tableWidget_stepResults->setRowCount(results.size());
    QStringList rowNames;
    for (int i = 0; i < results.size(); ++i) {

        QString time = results.at(i).T_value == 0
                           ? tr("Ошибка")
                           : QTime(0, 0).addMSecs(results.at(i).T_value).toString("m:ss.zzz");

        ui->tableWidget_stepResults->setItem(i, 0, new QTableWidgetItem(time));

        QString overshoot = QString("%1%").arg(results.at(i).overshoot, 4, 'f', 2);
        ui->tableWidget_stepResults->setItem(i, 1, new QTableWidgetItem(overshoot));

        QString rowName = QString("%1-%2").arg(results.at(i).from)
                              .arg(results.at(i).to);
        rowNames << rowName;
    }
    ui->tableWidget_stepResults->setVerticalHeaderLabels(rowNames);
    ui->tableWidget_stepResults->resizeColumnsToContents();
}

void MainWindow::displayDependingPattern() {
    switch (m_patternType) {
    case SelectTests::Pattern_B_CVT:
        ui->groupBox_DO->setVisible(false);
        ui->tabWidget->setTabEnabled(1, true);
        ui->tabWidget->setTabEnabled(2, false);
        ui->tabWidget->setTabEnabled(3, false);
        ui->tabWidget->setTabEnabled(4, true);
        break;
    case SelectTests::Pattern_B_SACVT:
        ui->groupBox_DO->setEnabled(true);
        ui->tabWidget->setTabEnabled(1, true);
        ui->tabWidget->setTabEnabled(2, false);
        ui->tabWidget->setTabEnabled(3, false);
        ui->tabWidget->setTabEnabled(4, true);
        break;
    case SelectTests::Pattern_C_CVT:
        ui->groupBox_DO->setVisible(false);
        ui->tabWidget->setTabEnabled(1, true);
        ui->tabWidget->setTabEnabled(2, true);
        ui->tabWidget->setTabEnabled(3, true);
        ui->tabWidget->setTabEnabled(4, true);
        break;
    case SelectTests::Pattern_C_SACVT:
        ui->groupBox_DO->setEnabled(true);
        ui->tabWidget->setTabEnabled(1, true);
        ui->tabWidget->setTabEnabled(2, true);
        ui->tabWidget->setTabEnabled(3, true);
        ui->tabWidget->setTabEnabled(4, true);
        break;
    case SelectTests::Pattern_C_SOVT:
        ui->groupBox_SettingCurrentSignal->setVisible(false);
        ui->groupBox_DO->setEnabled(true);
        ui->tabWidget->setTabEnabled(1, true);
        ui->tabWidget->setTabEnabled(2, false);
        ui->tabWidget->setTabEnabled(3, false);
        ui->tabWidget->setTabEnabled(4, true);
        break;
    default:
        break;
    }
}

void MainWindow::setSensorsNumber(quint8 sensorCount)
{
    bool hasSensors = (sensorCount > 0);

    if (hasSensors) {
        m_isInitialized = true;
    }

    updateAvailableTabs();

    ui->groupBox_SettingCurrentSignal->setEnabled(hasSensors);

    ui->pushButton_mainTest_start->setEnabled(sensorCount > 1);
    ui->pushButton_strokeTest_start->setEnabled(hasSensors);
    ui->pushButton_optionalTests_start->setEnabled(hasSensors);

    ui->doubleSpinBox_task->setEnabled(hasSensors);
    ui->verticalSlider_task->setEnabled(hasSensors);

    displayDependingPattern();

    if (hasSensors) {
        ui->checkBox_showCurve_task->setVisible(sensorCount > 1);
        ui->checkBox_showCurve_moving->setVisible(sensorCount > 1);
        ui->checkBox_showCurve_pressure_1->setVisible(sensorCount > 1);
        ui->checkBox_showCurve_pressure_2->setVisible(sensorCount > 2);
        ui->checkBox_showCurve_pressure_3->setVisible(sensorCount > 3);

        ui->checkBox_showCurve_task->setCheckState(sensorCount > 1 ? Qt::Checked : Qt::Unchecked);
        ui->checkBox_showCurve_moving->setCheckState(sensorCount > 1 ? Qt::Checked : Qt::Unchecked);
        ui->checkBox_showCurve_pressure_1->setCheckState(sensorCount > 1 ? Qt::Checked : Qt::Unchecked);
        ui->checkBox_showCurve_pressure_2->setCheckState(sensorCount > 2 ? Qt::Checked : Qt::Unchecked);
        ui->checkBox_showCurve_pressure_3->setCheckState(sensorCount > 3 ? Qt::Checked : Qt::Unchecked);

        syncTaskChartSeriesVisibility(sensorCount);
    }
}
void MainWindow::setButtonInitEnabled(bool enable)
{
    ui->pushButton_init->setEnabled(enable);
}

void MainWindow::addPoints(Charts chart, const QVector<Point> &points)
{
    for (const auto& point : points)
        m_charts[chart]->addPoint(point.seriesNum, point.X, point.Y);
}

void MainWindow::clearPoints(Charts chart)
{
    m_charts[chart]->clear();
}

void MainWindow::setChartVisible(Charts chart, quint16 series, bool visible)
{
    m_charts[chart]->visible(series, visible);
}

void MainWindow::showDots(bool visible)
{
    m_charts[Charts::Task]->showDots(visible);
    m_charts[Charts::Pressure]->showDots(visible);
}

void MainWindow::duplicateMainChartsSeries()
{
    m_charts[Charts::Task]->duplicateChartSeries(1);
    m_charts[Charts::Task]->duplicateChartSeries(2);
    m_charts[Charts::Task]->duplicateChartSeries(3);
    m_charts[Charts::Task]->duplicateChartSeries(4);
    m_charts[Charts::Pressure]->duplicateChartSeries(0);
}

void MainWindow::onStrokeTestPointsRequested(QVector<QVector<QPointF>> &points, Charts chart)
{
    points.clear();

    QPair<QList<QPointF>, QList<QPointF>> pointsLinear = m_charts[chart]->getPoints(1);
    QPair<QList<QPointF>, QList<QPointF>> pointsTask = m_charts[chart]->getPoints(0);

    points.push_back({pointsLinear.first.begin(), pointsLinear.first.end()});
    points.push_back({pointsTask.first.begin(), pointsTask.first.end()});
}

void MainWindow::onMainTestPointsRequested(QVector<QVector<QPointF>> &points, Charts chart)
{
    points.clear();

    QPair<QList<QPointF>, QList<QPointF>> pointsLinear = m_charts[chart]->getPoints(1);
    QPair<QList<QPointF>, QList<QPointF>> pointsPressure = m_charts[Charts::Pressure]->getPoints(0);

    points.push_back({pointsLinear.first.begin(), pointsLinear.first.end()});
    points.push_back({pointsLinear.second.begin(), pointsLinear.second.end()});
    points.push_back({pointsPressure.first.begin(), pointsPressure.first.end()});
    points.push_back({pointsPressure.second.begin(), pointsPressure.second.end()});
}

void MainWindow::onStepTestPointsRequested(QVector<QVector<QPointF>> &points, Charts chart)
{
    points.clear();

    if (chart == Charts::Step) {
        QPair<QList<QPointF>, QList<QPointF>> pointsLinear = m_charts[chart]->getPoints(1);
        QPair<QList<QPointF>, QList<QPointF>> pointsTask = m_charts[chart]->getPoints(0);

        points.clear();
        points.push_back({pointsLinear.first.begin(), pointsLinear.first.end()});
        points.push_back({pointsTask.first.begin(), pointsTask.first.end()});
    }
}

void MainWindow::setRegressionEnabled(bool enabled)
{
    ui->checkBox_regression->setEnabled(enabled);
    ui->checkBox_regression->setCheckState(enabled ? Qt::Checked : Qt::Unchecked);
}

void MainWindow::onMainTestParametersRequested(MainTestSettings::TestParameters &parameters)
{
    if (m_mainTestSettings->exec() == QDialog::Accepted) {
        parameters = m_mainTestSettings->getParameters();
    } else {
        parameters.delay = 0;
    }
}

void MainWindow::onStepTestParametersRequested(StepTestSettings::TestParameters &parameters)
{
    parameters.points.clear();

    if (m_stepTestSettings->exec() == QDialog::Accepted) {
        parameters = m_stepTestSettings->getParameters();
        return;
    } else {
        parameters = {};
        return;
    }
}

void MainWindow::onResolutionTestParametersRequested(OtherTestSettings::TestParameters &parameters)
{
    parameters.points.clear();

    if (m_resolutionTestSettings->exec() == QDialog::Accepted) {
        parameters = m_resolutionTestSettings->getParameters();
        return;
    } else {
        parameters = {};
        return;
    }
}

void MainWindow::onResponseTestParametersRequested(OtherTestSettings::TestParameters &parameters)
{
    parameters.points.clear();

    if (m_responseTestSettings->exec() == QDialog::Accepted) {
        parameters = m_responseTestSettings->getParameters();
        return;

    } else {
        parameters = {};
        return;
    }
}


static QString seqToString(const QVector<qreal>& seq)
{
    QStringList parts;
    parts.reserve(seq.size());
    for (quint16 v : seq) parts << QString::number(v);
    return parts.join('-');
}

void MainWindow::askQuestion(const QString &title, const QString &text, bool &result)
{
    result = (QMessageBox::question(this, title, text) == QMessageBox::Yes);
}

void MainWindow::getDirectory(const QString &currentPath, QString &result)
{
    result = QFileDialog::getExistingDirectory(this,
                                               tr("Выберите папку для сохранения изображений"),
                                               currentPath);
}

void MainWindow::startTest()
{
    m_isTestRunning = true;
    ui->statusbar->showMessage(tr("Тест в процессе"));
}

void MainWindow::endTest()
{
    m_isTestRunning = false;

    ui->statusbar->showMessage(tr("Тест завершён"));

    if (m_durationTimer)
        m_durationTimer->stop();

    promptSaveChartsAfterTest();
}

void MainWindow::on_pushButton_mainTest_start_clicked()
{
    if (m_isTestRunning ) {
        if (QMessageBox::question(this, tr("Внимание!"), tr("Вы действительно хотите завершить тест?"))
        == QMessageBox::Yes) {
            m_isUserCanceled = true;
            emit stopTest();
        }
    } else {
        m_isUserCanceled = false;
        emit runMainTest();
        startTest();
    }
}
void MainWindow::on_pushButton_mainTest_save_clicked()
{
    if (ui->tabWidget_mainTests->currentWidget() == ui->tab_mainTests_task) {
        saveChart(Charts::Task);
    } else if (ui->tabWidget_mainTests->currentWidget() == ui->tab_mainTests_pressure) {
        saveChart(Charts::Pressure);
    } else if (ui->tabWidget_mainTests->currentWidget() == ui->tab_mainTests_friction) {
        saveChart(Charts::Friction);
    }
}

void MainWindow::on_pushButton_strokeTest_start_clicked()
{
    if (m_isTestRunning ) {
        if (QMessageBox::question(this, tr("Внимание!"), tr("Вы действительно хотите завершить тест?"))
            == QMessageBox::Yes) {
            emit stopTest();
        }
    } else {
        emit runStrokeTest();
        startTest();
    }
}
void MainWindow::on_pushButton_strokeTest_save_clicked()
{
    saveChart(Charts::Stroke);
}

void MainWindow::on_pushButton_optionalTests_start_clicked()
{
    if (m_isTestRunning ) {
        if (QMessageBox::question(this, tr("Внимание!"), tr("Вы действительно хотите завершить тест?"))
            == QMessageBox::Yes) {
            emit stopTest();
        }
    } else {
        emit runOptionalTest(ui->tabWidget_optionalTests->currentIndex());
        startTest();
    }
}
void MainWindow::on_pushButton_optionalTests_save_clicked()
{
    if (ui->tabWidget_optionalTests->currentWidget() == ui->tab_optionalTests_response) {
        saveChart(Charts::Response);
    } else if (ui->tabWidget_optionalTests->currentWidget() == ui->tab_optionalTests_resolution) {
        saveChart(Charts::Resolution);
    } else if (ui->tabWidget_optionalTests->currentWidget() == ui->tab_optionalTests_step) {
        saveChart(Charts::Step);
    }
}

void MainWindow::setDoButtonsChecked(quint8 bitmask)
{
    ui->pushButton_DO0->blockSignals(true);
    ui->pushButton_DO1->blockSignals(true);
    ui->pushButton_DO2->blockSignals(true);
    ui->pushButton_DO3->blockSignals(true);

    ui->pushButton_DO0->setChecked((bitmask & (1 << 0)) != 0);
    ui->pushButton_DO1->setChecked((bitmask & (1 << 1)) != 0);
    ui->pushButton_DO2->setChecked((bitmask & (1 << 2)) != 0);
    ui->pushButton_DO3->setChecked((bitmask & (1 << 3)) != 0);

    ui->pushButton_DO0->blockSignals(false);
    ui->pushButton_DO1->blockSignals(false);
    ui->pushButton_DO2->blockSignals(false);
    ui->pushButton_DO3->blockSignals(false);

    ui->groupBox_DO->setEnabled(true);
}

void MainWindow::setDiCheckboxesChecked(quint8 bitmask)
{
    ui->checkBox_switch_3_0->setChecked((bitmask & (1 << 0)) != 0);
    ui->checkBox_switch_0_3->setChecked((bitmask & (1 << 1)) != 0);
}

void MainWindow::syncTaskChartSeriesVisibility(quint8 sensorCount)
{
    auto *ch = m_charts.value(Charts::Task, nullptr);
    if (!ch) return;

    // 0 - Задание, 1 - линейный датчик, 2..4 - давления 1..3
    ch->visible(0, sensorCount > 1 && ui->checkBox_showCurve_task->isChecked());
    ch->visible(1, sensorCount > 1 && ui->checkBox_showCurve_moving->isChecked());

    ch->visible(2, sensorCount > 1 && ui->checkBox_showCurve_pressure_1->isChecked());
    ch->visible(3, sensorCount > 2 && ui->checkBox_showCurve_pressure_2->isChecked());
    ch->visible(4, sensorCount > 3 && ui->checkBox_showCurve_pressure_3->isChecked());
}

void MainWindow::initCharts()
{
    auto& valveInfo = m_registry->valveInfo();
    bool isRotaryStroke = (valveInfo.strokeMovement != 0);

    const QString strokeAxisFormat =
        isRotaryStroke ? QStringLiteral("%.2f deg")
                       : QStringLiteral("%.2f mm");

    m_charts[Charts::Task] = ui->Chart_task;
    m_charts[Charts::Task]->setName(QStringLiteral("Task"));
    m_charts[Charts::Task]->useTimeaxis(false);
    m_charts[Charts::Task]->addAxis(QStringLiteral("%.2f bar"));
    m_charts[Charts::Task]->addAxis(strokeAxisFormat);
    m_charts[Charts::Task]->addSeries(1, tr("Задание"), QColor::fromRgb(0, 0, 0));
    m_charts[Charts::Task]->addSeries(1, tr("Датчик линейных перемещений"), QColor::fromRgb(255, 0, 0));
    m_charts[Charts::Task]->addSeries(0, tr("Датчик давления 1"), QColor::fromRgb(42, 104, 159));
    m_charts[Charts::Task]->addSeries(0, tr("Датчик давления 2"), QColor::fromRgb(69, 116, 72));
    m_charts[Charts::Task]->addSeries(0, tr("Датчик давления 3"), QColor::fromRgb(211, 187, 42));

    m_charts[Charts::Friction] = ui->Chart_friction;
    m_charts[Charts::Friction]->setName(QStringLiteral("Friction"));
    m_charts[Charts::Friction]->addAxis(QStringLiteral("%.2f H"));
    m_charts[Charts::Friction]->addSeries(0, tr("Трение от перемещения"), QColor::fromRgb(255, 0, 0));
    m_charts[Charts::Friction]->setLabelXformat(strokeAxisFormat);

    m_charts[Charts::Pressure] = ui->Chart_pressure;
    m_charts[Charts::Pressure]->setName(QStringLiteral("Pressure"));
    m_charts[Charts::Pressure]->useTimeaxis(false);
    m_charts[Charts::Pressure]->setLabelXformat(QStringLiteral("%.2f bar"));
    m_charts[Charts::Pressure]->addAxis(strokeAxisFormat);
    m_charts[Charts::Pressure]->addSeries(0, tr("Перемещение от давления"), QColor::fromRgb(255, 0, 0));
    m_charts[Charts::Pressure]->addSeries(0, tr("Линейная регрессия"), QColor::fromRgb(0, 0, 0));
    m_charts[Charts::Pressure]->visible(1, false);


    m_charts[Charts::Resolution] = ui->Chart_resolution;
    m_charts[Charts::Resolution]->setName(QStringLiteral("Resolution"));
    m_charts[Charts::Resolution]->useTimeaxis(true);
    m_charts[Charts::Resolution]->addAxis(QStringLiteral("%.2f%%"));
    m_charts[Charts::Resolution]->addSeries(0, tr("Задание"), QColor::fromRgb(0, 0, 0));
    m_charts[Charts::Resolution]->addSeries(0, tr("Датчик линейных перемещений"), QColor::fromRgb(255, 0, 0));


    m_charts[Charts::Response] = ui->Chart_response;
    m_charts[Charts::Response]->setName(QStringLiteral("Response"));
    m_charts[Charts::Response]->useTimeaxis(true);
    m_charts[Charts::Response]->addAxis(QStringLiteral("%.2f%%"));
    m_charts[Charts::Response]->addSeries(0, tr("Задание"), QColor::fromRgb(0, 0, 0));
    m_charts[Charts::Response]->addSeries(0, tr("Датчик линейных перемещений"), QColor::fromRgb(255, 0, 0));


    m_charts[Charts::Stroke] = ui->Chart_stroke;
    m_charts[Charts::Stroke]->setName(QStringLiteral("Stroke"));
    m_charts[Charts::Stroke]->useTimeaxis(true);
    m_charts[Charts::Stroke]->addAxis(QStringLiteral("%.2f%%"));
    m_charts[Charts::Stroke]->addSeries(0, tr("Задание"), QColor::fromRgb(0, 0, 0));
    m_charts[Charts::Stroke]->addSeries(0, tr("Датчик линейных перемещений"), QColor::fromRgb(255, 0, 0));


    m_charts[Charts::Step] = ui->Chart_step;
    m_charts[Charts::Step]->setName(QStringLiteral("Step"));
    m_charts[Charts::Step]->useTimeaxis(true);
    m_charts[Charts::Step]->addAxis(QStringLiteral("%.2f%%"));
    m_charts[Charts::Step]->addSeries(0, tr("Задание"), QColor::fromRgb(0, 0, 0));
    m_charts[Charts::Step]->addSeries(0, tr("Датчик линейных перемещений"), QColor::fromRgb(255, 0, 0));


    m_charts[Charts::Trend] = ui->Chart_trend;
    m_charts[Charts::Trend]->useTimeaxis(true);
    m_charts[Charts::Trend]->addAxis(QStringLiteral("%.2f%%"));

    m_charts[Charts::Trend]->addSeries(0, tr("Задание"), QColor::fromRgb(0, 0, 0));
    m_charts[Charts::Trend]->addSeries(0, tr("Датчик линейных перемещений"), QColor::fromRgb(255, 0, 0));
    m_charts[Charts::Trend]->setMaxRange(60000);

    connect(m_program, &Program::addPoints,
            this, &MainWindow::addPoints);

    connect(m_program, &Program::clearPoints,
            this, &MainWindow::clearPoints);

    connect(m_program, &Program::duplicateMainChartsSeries,
            this, &MainWindow::duplicateMainChartsSeries);

    connect(m_program, &Program::setVisible,
            this, &MainWindow::setChartVisible);

    connect(m_program, &Program::setRegressionEnable,
            this, &MainWindow::setRegressionEnabled);

    connect(m_program, &Program::showDots,
            this, &MainWindow::showDots);

    connect(ui->checkBox_showCurve_task, &QCheckBox::checkStateChanged,
            this, [&](int state) {
        m_charts[Charts::Task]->visible(0, state != 0);
    });

    connect(ui->checkBox_showCurve_moving, &QCheckBox::checkStateChanged,
            this, [&](int state) {
        m_charts[Charts::Task]->visible(1, state != 0);
    });

    connect(ui->checkBox_showCurve_pressure_1, &QCheckBox::checkStateChanged,
            this, [&](int state) {
        m_charts[Charts::Task]->visible(2, state != 0);
    });

    connect(ui->checkBox_showCurve_pressure_2, &QCheckBox::checkStateChanged,
            this, [&](int state) {
        m_charts[Charts::Task]->visible(3, state != 0);
    });

    connect(ui->checkBox_showCurve_pressure_3, &QCheckBox::checkStateChanged,
            this, [&](int state) {
        m_charts[Charts::Task]->visible(4, state != 0);
    });

    connect(ui->checkBox_regression, &QCheckBox::checkStateChanged,
            this, [&](int state) {
        m_charts[Charts::Pressure]->visible(1, state != 0);
    });

    connect(m_program, &Program::getPoints_strokeTest,
            this, &MainWindow::onStrokeTestPointsRequested,
            Qt::BlockingQueuedConnection);

    connect(m_program, &Program::getPoints_mainTest,
            this, &MainWindow::onMainTestPointsRequested,
            Qt::BlockingQueuedConnection);

    connect(m_program, &Program::getPoints_stepTest,
            this, &MainWindow::onStepTestPointsRequested,
            Qt::BlockingQueuedConnection);
}

void MainWindow::saveChart(Charts chart)
{
    std::optional<SeriesVisibilityBackup> backup;

    if (chart == Charts::Task) {
        backup = hideTaskAuxSeries();
    } else if (chart == Charts::Pressure) {
        backup = hidePressureAuxSeries();
    }

    if (m_reportSaver && m_charts.contains(chart) && m_charts[chart]) {
        m_reportSaver->saveImage(m_charts[chart]);
    }

    QPixmap pix = m_charts[chart]->grab();

    if (backup.has_value())
        restoreSeries(chart, *backup);

    QImage img = pix.toImage();


    switch (chart) {
    case Charts::Task:
        ui->label_imageChartTask->setPixmap(pix);
        m_imageChartTask = img;
        break;
    case Charts::Pressure:
        ui->label_imageChartPressure->setPixmap(pix);
        m_imageChartPressure = img;
        break;
    case Charts::Friction:
        ui->label_imageChartFriction->setPixmap(pix);
        m_imageChartFriction = img;
        break;
    case Charts::Response:
        m_imageChartResponse = img;
        break;
    case Charts::Resolution:
        m_imageChartResolution = img;
        break;
    case Charts::Step:
        m_imageChartStep = img;
        break;
    case Charts::Trend:
        break;
    default:
        break;
    }

    ui->label_imageChartTask->setScaledContents(true);
    ui->label_imageChartPressure->setScaledContents(true);
    ui->label_imageChartFriction->setScaledContents(true);
}

void MainWindow::getImage(QLabel *label, QImage *image)
{
    QString imgPath = QFileDialog::getOpenFileName(this,
                                                   tr("Выберите файл"),
                                                   m_reportSaver->directory().absolutePath(),
                                                   tr("Изображения (*.jpg *.png *.bmp)"));

    if (!imgPath.isEmpty()) {
        QImage img(imgPath);
        *image = img.scaled(1000, 430, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        label->setPixmap(QPixmap::fromImage(img));
    }
}

void MainWindow::on_pushButton_init_clicked()
{
    // QVector<bool> states = {
    //     ui->pushButton_DO0->isChecked(),
    //     ui->pushButton_DO1->isChecked(),
    //     ui->pushButton_DO2->isChecked(),
    //     ui->pushButton_DO3->isChecked()
    // };

    // emit doInitStatesSelected(states);
    emit initialized();
    emit patternChanged(m_patternType);
}

void MainWindow::on_pushButton_imageChartTask_clicked()
{
    getImage(ui->label_imageChartTask, &m_imageChartTask);
}
void MainWindow::on_pushButton_imageChartPressure_clicked()
{
    getImage(ui->label_imageChartPressure, &m_imageChartPressure);
}
void MainWindow::on_pushButton_imageChartFriction_clicked()
{
    getImage(ui->label_imageChartFriction, &m_imageChartFriction);
}

MainWindow::SeriesVisibilityBackup MainWindow::hidePressureAuxSeries()
{
    SeriesVisibilityBackup b;

    b.visible = { ui->checkBox_regression->isChecked() };

    auto* ch = m_charts.value(Charts::Pressure, nullptr);
    if (!ch) return b;

    ch->visible(1, false);
    return b;
}

void MainWindow::promptSaveChartsAfterTest()
{
    if (m_isUserCanceled)
        return;

    const auto charts = chartsForCurrentTest();
    if (charts.isEmpty())
        return;

    auto answer = QMessageBox::question(
        this,
        tr("Сохранение результатов"),
        tr("Тест завершён.\nСохранить графики?"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes
        );

    if (answer != QMessageBox::Yes)
        return;

    for (Charts c : charts)
        saveChart(c);
}

QVector<Charts> MainWindow::chartsForCurrentTest() const
{
    QWidget* top = ui->tabWidget->currentWidget();

    if (top == ui->tab_strokeTest) {
        return { Charts::Stroke };
    }

    if (top == ui->tab_mainTests) {
        return { Charts::Task, Charts::Pressure, Charts::Friction };
    }

    if (top == ui->tab_optionalTests) {
        QWidget* w = ui->tabWidget_optionalTests->currentWidget();

        if (w == ui->tab_optionalTests_response)
            return { Charts::Response };

        if (w == ui->tab_optionalTests_resolution)
            return { Charts::Resolution };

        if (w == ui->tab_optionalTests_step)
            return { Charts::Step };
    }

    return {};
}

MainWindow::SeriesVisibilityBackup MainWindow::hideTaskAuxSeries()
{
    SeriesVisibilityBackup b;

    b.visible = {
        ui->checkBox_showCurve_pressure_1->isChecked(),
        ui->checkBox_showCurve_pressure_2->isChecked(),
        ui->checkBox_showCurve_pressure_3->isChecked()
    };

    auto* ch = m_charts.value(Charts::Task, nullptr);
    if (!ch) return b;

    ch->visible(2, false);
    ch->visible(3, false);
    ch->visible(4, false);

    return b;
}

void MainWindow::restoreSeries(Charts chart, const SeriesVisibilityBackup& b)
{
    auto* ch = m_charts.value(chart, nullptr);
    if (!ch) return;

    if (chart == Charts::Task && b.visible.size() == 3) {
        ch->visible(2, b.visible[0]);
        ch->visible(3, b.visible[1]);
        ch->visible(4, b.visible[2]);
    }

    if (chart == Charts::Pressure && b.visible.size() == 1) {
        ch->visible(1, b.visible[0]);
    }
}

void MainWindow::on_pushButton_report_generate_clicked()
{
    std::unique_ptr<ReportBuilder> reportBuilder;

    reportBuilder = std::make_unique<ReportBuilder>();

    ReportSaver::Report report;
    reportBuilder->buildReport(
        report,
        m_telemetryStore,
        m_registry->objectInfo(),
        m_registry->valveInfo(),
        m_registry->otherParameters(),
        m_registry->materialsOfComponentParts(),
        m_imageChartTask,
        m_imageChartPressure,
        m_imageChartFriction,
        m_imageChartResponse,
        m_imageChartResolution,
        m_imageChartStep
        );

    bool saved = m_reportSaver->saveReport(report, reportBuilder->templatePath());
    ui->pushButton_report_open->setEnabled(saved);
}
void MainWindow::on_pushButton_report_open_clicked()
{
    QDesktopServices::openUrl(
        QUrl::fromLocalFile(m_reportSaver->directory().filePath(QStringLiteral("report.xlsx"))));
}
