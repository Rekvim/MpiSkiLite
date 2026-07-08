#include "TelemetryUiMapper.h"
#include "ui_MainWindow.h"

#include <QTableWidgetItem>
#include <QTime>

void TelemetryUiMapper::updateInit(const InitState& init,
                                   const ValveStrokeRecord& strokeResult)
{
    m_ui->label_deviceStatusValue->setText(init.deviceStatusText);
    m_ui->label_deviceStatusValue->setStyleSheet(
        "color:" + init.deviceStatusColor.name(QColor::HexRgb));

    m_ui->label_deviceInitValue->setText(init.initStatusText);
    m_ui->label_deviceInitValue->setStyleSheet(
        "color:" + init.initStatusColor.name(QColor::HexRgb));

    m_ui->label_connectedSensorsNumber->setText(init.connectedSensorsText);
    m_ui->label_connectedSensorsNumber->setStyleSheet(
        "color:" + init.connectedSensorsColor.name(QColor::HexRgb));

    m_ui->label_startingPositionValue->setText(init.startingPositionText);
    m_ui->label_startingPositionValue->setStyleSheet(
        "color:" + init.startingPositionColor.name(QColor::HexRgb));

    m_ui->label_finalPositionValue->setText(init.finalPositionText);
    m_ui->label_finalPositionValue->setStyleSheet(
        "color:" + init.finalPositionColor.name(QColor::HexRgb));

    m_ui->label_valveStroke_range->setText(strokeResult.range);
    m_ui->lineEdit_crossingLimits_range_value->setText(strokeResult.range);
}

void TelemetryUiMapper::updateStrokeTest(const Domain::Tests::Stroke::Result& result)
{
    m_ui->lineEdit_strokeTest_forwardTime->setText(
        QTime(0, 0).addMSecs(result.forwardTimeMs).toString("mm:ss.zzz"));

    m_ui->lineEdit_resultsTable_strokeTest_forwardTime->setText(
        QTime(0, 0).addMSecs(result.forwardTimeMs).toString("mm:ss.zzz"));

    m_ui->lineEdit_strokeTest_backwardTime->setText(
        QTime(0, 0).addMSecs(result.backwardTimeMs).toString("mm:ss.zzz"));

    m_ui->lineEdit_resultsTable_strokeTest_backwardTime->setText(
        QTime(0, 0).addMSecs(result.backwardTimeMs).toString("mm:ss.zzz"));

    m_ui->lineEdit_strokeTest_respondingReactionbackwardTime->setText(
        QTime(0, 0).addMSecs(result.backwardSignalDelayMs).toString("mm:ss.zzz"));

    m_ui->lineEdit_strokeTest_respondingReactionforwardTime->setText(
        QTime(0, 0).addMSecs(result.forwardSignalDelayMs).toString("mm:ss.zzz"));
}

void TelemetryUiMapper::updateMainTest(
    const Domain::Tests::Main::Result& mainResult,
    const ValveStrokeRecord& strokeResult)
{
    m_ui->label_pressureDifferenceValue->setText(
        QString("%1").arg(mainResult.pressureDiff, 0, 'f', 3));

    m_ui->label_frictionForceValue->setText(
        QString("%1").arg(mainResult.frictionForce, 0, 'f', 3));

    m_ui->label_frictionPercentValue->setText(
        QString("%1").arg(mainResult.frictionPercent, 0, 'f', 2));

    m_ui->lineEdit_resultsTable_frictionForceValue->setText(
        QString("%1").arg(mainResult.frictionForce, 0, 'f', 3));

    m_ui->lineEdit_resultsTable_frictionPercentValue->setText(
        QString("%1").arg(mainResult.frictionPercent, 0, 'f', 2));

    m_ui->label_dynamicErrorMeanPercent->setText(
        QString("%1 %").arg(mainResult.dynamicErrorMeanPercent, 0, 'f', 2));

    m_ui->label_dynamicErrorMean->setText(
        QString("%1 мА").arg(mainResult.dynamicErrorMean, 0, 'f', 3));

    m_ui->label_dynamicErrorMaxPercent->setText(
        QString("%1 %")
            .arg(mainResult.dynamicErrorMaxPercent, 0, 'f', 2)
        );

    m_ui->label_dynamicErrorMax->setText(
        QString("%1 мА").arg(mainResult.dynamicErrorMax, 0, 'f', 3));

    m_ui->lineEdit_resultsTable_dynamicErrorReal->setText(
        QString("%1").arg(mainResult.dynamicErrorReal, 0, 'f', 2));

    m_ui->lineEdit_resultsTable_strokeReal->setText(
        QString("%1").arg(strokeResult.real, 0, 'f', 2));

    m_ui->label_lowLimitValue->setText(
        QString("%1").arg(mainResult.lowLimitPressure));

    m_ui->label_highLimitValue->setText(
        QString("%1").arg(mainResult.highLimitPressure));

    m_ui->lineEdit_resultsTable_rangePressure->setText(
        QString("%1–%2")
            .arg(mainResult.lowLimitPressure, 0, 'f', 2)
            .arg(mainResult.highLimitPressure, 0, 'f', 2)
        );

    m_ui->lineEdit_resultsTable_driveRangeReal->setText(
        QString("%1–%2")
            .arg(mainResult.springLow, 0, 'f', 2)
            .arg(mainResult.springHigh, 0, 'f', 2)
        );

    m_ui->label_valveStroke_range->setText(strokeResult.range);
}

void TelemetryUiMapper::updateCrossingValues(
    const Domain::Tests::Main::Result& mainResult,
    const ValveStrokeRecord& strokeResult)
{
    m_ui->lineEdit_crossingLimits_dynamicError_value->setText(
        QString::number(mainResult.dynamicErrorReal, 'f', 2));

    m_ui->lineEdit_crossingLimits_linearCharacteristic_value->setText(
        QString::number(mainResult.linearityError, 'f', 2));

    m_ui->lineEdit_crossingLimits_range_value->setText(
        QString::number(strokeResult.real, 'f', 2));

    m_ui->lineEdit_crossingLimits_spring_value->setText(
        QString("%1–%2")
            .arg(mainResult.springLow, 0, 'f', 2)
            .arg(mainResult.springHigh, 0, 'f', 2));

    m_ui->lineEdit_crossingLimits_coefficientFriction_value->setText(
        QString::number(mainResult.frictionPercent, 'f', 2));
}

void TelemetryUiMapper::updateStepTest(const Domain::Tests::Option::Step::Result& result)
{
    const auto& steps = result.steps;

    m_ui->tableWidget_stepResults->clearContents();

    m_ui->tableWidget_stepResults->setColumnCount(2);
    m_ui->tableWidget_stepResults->setHorizontalHeaderLabels({
        QObject::tr("T%1").arg(result.testValue),
        QObject::tr("Перерегулирование")
    });

    m_ui->tableWidget_stepResults->setRowCount(steps.size());

    QStringList rowNames;
    rowNames.reserve(steps.size());

    for (int i = 0; i < steps.size(); ++i)
    {
        const auto& step = steps.at(i);

        const QString time = step.T_value == 0
                                 ? QObject::tr("Ошибка")
                                 : QTime(0, 0)
                                       .addMSecs(step.T_value)
                                       .toString("mm:ss.zzz");

        const QString overshoot =
            QString("%1%").arg(step.overshoot, 4, 'f', 2);

        const QString rowName =
            QString("%1-%2").arg(step.from).arg(step.to);

        m_ui->tableWidget_stepResults->setItem(
            i, 0, new QTableWidgetItem(time));

        m_ui->tableWidget_stepResults->setItem(
            i, 1, new QTableWidgetItem(overshoot));

        rowNames << rowName;
    }

    m_ui->tableWidget_stepResults->setVerticalHeaderLabels(rowNames);
    m_ui->tableWidget_stepResults->resizeColumnsToContents();
}