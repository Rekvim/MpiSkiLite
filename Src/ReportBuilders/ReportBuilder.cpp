#include "./Src/ReportBuilders/ReportBuilder.h"

void ReportBuilder::buildReport(
    ReportSaver::Report &report,
    const TelemetryStore &telemetryStore,
    const ObjectInfo &objectInfo,
    const ValveInfo &valveInfo,
    const OtherParameters &otherParams,
    const MaterialsOfComponentParts &materialsOfComponentParts,
    const QImage &imageChartTask,
    const QImage &imageChartPressure,
    const QImage &imageChartFriction,
    const QImage &imageChartResponse,
    const QImage &imageChartResolution,
    const QImage &imageChartStep
    ) {

    cell(report, m_sheetStepReactionTest, 1, 9, valveInfo.positionNumber);

    // Лист: Результат теста шаговой реакции; Страница: 2; Блок: Данные по объекту
    cell(report, m_sheetStepReactionTest, 4, 4, objectInfo.object);
    cell(report, m_sheetStepReactionTest, 5, 4, objectInfo.manufactory);
    cell(report, m_sheetStepReactionTest, 6, 4, objectInfo.department);

    // Страница:Результат теста шаговой реакции; Блок: Краткая спецификация на клапан
    cell(report, m_sheetStepReactionTest, 4, 13, valveInfo.positionNumber);
    cell(report, m_sheetStepReactionTest, 5, 13, valveInfo.serialNumber);
    cell(report, m_sheetStepReactionTest, 6, 13, valveInfo.valveModel);
    cell(report, m_sheetStepReactionTest, 7, 13, valveInfo.manufacturer);
    cell(report, m_sheetStepReactionTest, 8, 13, QString("%1 / %2").arg(valveInfo.DN, valveInfo.PN));
    cell(report, m_sheetStepReactionTest, 9, 13, valveInfo.positionerModel);
    cell(report, m_sheetStepReactionTest, 10, 13, QString("%1")
                                      .arg(telemetryStore.supplyRecord.pressure_bar, 0, 'f', 2));
    cell(report, m_sheetStepReactionTest, 11, 13, otherParams.safePosition);
    cell(report, m_sheetStepReactionTest, 12, 13, valveInfo.driveModel);
    cell(report, m_sheetStepReactionTest, 13, 13, otherParams.strokeMovement);
    cell(report, m_sheetStepReactionTest, 14, 13, valveInfo.materialStuffingBoxSeal);

    // Страница: Результат теста шаговой реакции; Блок: График теста шаговой реакции
    report.images.push_back({m_sheetStepReactionTest, 18, 2, imageChartStep});

    // Страница: Результат теста шаговой реакции; Блок: Результат теста шаговой реакции
    {
        quint16 row = 55;
        for (auto &sr : telemetryStore.stepResults) {
            cell(report, m_sheetStepReactionTest, row, 3, QString("%1->%2").arg(sr.from).arg(sr.to));
            cell(report, m_sheetStepReactionTest, row, 4, QTime(0,0).addMSecs(sr.T_value).toString("m:ss.zzz"));
            cell(report, m_sheetStepReactionTest, row, 5, QString("%1").arg(sr.overshoot, 0, 'f', 2));
            ++row;
        }
    }

    // Страница: Отчет ЦТ; Блок: Дата
    cell(report, m_sheetStepReactionTest, 76, 12, otherParams.date);

    // Страница: Отчет; Блок: Данные по объекту
    cell(report, m_sheetTechnicalInspection, 5, 4, objectInfo.object);
    cell(report, m_sheetTechnicalInspection, 6, 4, objectInfo.manufactory);
    cell(report, m_sheetTechnicalInspection, 7, 4, objectInfo.department);

    // Страница:Отчет; Блок: Краткая спецификация на клапан
    cell(report, m_sheetTechnicalInspection, 5, 13, valveInfo.positionNumber);
    cell(report, m_sheetTechnicalInspection, 6, 13, valveInfo.serialNumber);
    cell(report, m_sheetTechnicalInspection, 7, 13, valveInfo.valveModel);
    cell(report, m_sheetTechnicalInspection, 8, 13, valveInfo.manufacturer);
    cell(report, m_sheetTechnicalInspection, 9, 13, QString("%1 / %2").arg(valveInfo.DN, valveInfo.PN));
    cell(report, m_sheetTechnicalInspection, 10, 13, valveInfo.positionerModel);
    cell(report, m_sheetTechnicalInspection, 11, 13, QString("%1").arg(telemetryStore.supplyRecord.pressure_bar, 0, 'f', 2));
    cell(report, m_sheetTechnicalInspection, 12, 13, otherParams.safePosition);
    cell(report, m_sheetTechnicalInspection, 13, 13, valveInfo.driveModel);
    cell(report, m_sheetTechnicalInspection, 14, 13, otherParams.strokeMovement);
    cell(report, m_sheetTechnicalInspection, 15, 13, valveInfo.materialStuffingBoxSeal);

    // Материал деталей
    report.data.push_back({m_sheetTechnicalInspection, 11, 4, materialsOfComponentParts.corpus});
    report.data.push_back({m_sheetTechnicalInspection, 12, 4, materialsOfComponentParts.cap});
    report.data.push_back({m_sheetTechnicalInspection, 13, 4, materialsOfComponentParts.saddle});
    report.data.push_back({m_sheetTechnicalInspection, 13, 6, materialsOfComponentParts.CV});
    report.data.push_back({m_sheetTechnicalInspection, 14, 4, materialsOfComponentParts.ball});
    report.data.push_back({m_sheetTechnicalInspection, 15, 4, materialsOfComponentParts.disk});
    report.data.push_back({m_sheetTechnicalInspection, 16, 4, materialsOfComponentParts.plunger});
    report.data.push_back({m_sheetTechnicalInspection, 17, 4, materialsOfComponentParts.shaft});
    report.data.push_back({m_sheetTechnicalInspection, 18, 4, materialsOfComponentParts.stock});
    report.data.push_back({m_sheetTechnicalInspection, 19, 4, materialsOfComponentParts.guideSleeve});

    // Страница: Отчет; Блок: Результат испытаний
    cell(report, m_sheetTechnicalInspection, 29, 5,
         QString("%1")
             .arg(telemetryStore.mainTestRecord.dynamicErrorReal, 0, 'f', 2));

    cell(report, m_sheetTechnicalInspection, 29, 8,
         QString("%1")
             .arg(valveInfo.dinamicErrorRecomend, 0, 'f', 2));
    cell(report, m_sheetTechnicalInspection, 29, 11, resultOk(telemetryStore.crossingStatus.dynamicError));

    cell(report, m_sheetTechnicalInspection, 31, 5, QString("%1")
                                     .arg(telemetryStore.valveStrokeRecord.real, 0, 'f', 2));
    cell(report, m_sheetTechnicalInspection, 31, 8, valveInfo.strokValve);
    cell(report, m_sheetTechnicalInspection, 31, 11, resultOk(telemetryStore.crossingStatus.range));

    cell(report, m_sheetTechnicalInspection, 33, 5,
         QString("%1–%2")
             .arg(telemetryStore.mainTestRecord.springLow, 0, 'f', 2)
             .arg(telemetryStore.mainTestRecord.springHigh, 0, 'f', 2)
         );
    cell(report, m_sheetTechnicalInspection, 33, 11, resultOk(telemetryStore.crossingStatus.spring));
    cell(report, m_sheetTechnicalInspection, 33, 8,
        QString("%1–%2")
            .arg(valveInfo.driveRangeLow, 0, 'f', 2)
            .arg(valveInfo.driveRangeHigh, 0, 'f', 2)
    );

    cell(report, m_sheetTechnicalInspection, 35, 5,
        QString("%1–%2")
            .arg(telemetryStore.mainTestRecord.lowLimitPressure, 0, 'f', 2)
            .arg(telemetryStore.mainTestRecord.highLimitPressure, 0, 'f', 2)
    );

    cell(report, m_sheetTechnicalInspection, 37, 5,
        QString("%1")
            .arg(telemetryStore.mainTestRecord.frictionPercent, 0, 'f', 2)
    );
    cell(report, m_sheetTechnicalInspection, 37, 11, resultLimit(telemetryStore.crossingStatus.frictionPercent));

    cell(report, m_sheetTechnicalInspection, 39, 5,
        QString("%1")
            .arg(telemetryStore.mainTestRecord.frictionForce, 0, 'f', 3)
    );
    cell(report,
        m_sheetTechnicalInspection, 51, 5, telemetryStore.strokeTestRecord.timeForwardMs
    );
    cell(report,
        m_sheetTechnicalInspection, 51, 8, telemetryStore.strokeTestRecord.timeBackwardMs
    );

    // Дата и Исполнитель
    cell(report, m_sheetTechnicalInspection, 65, 12, otherParams.date);
    cell(report, m_sheetTechnicalInspection, 73, 4, objectInfo.FIO);

    // Страница: Отчет; Блок: Диагностические графики
    image(report, m_sheetTechnicalInspection, 83, 1, imageChartTask);
    image(report, m_sheetTechnicalInspection, 108, 1, imageChartPressure);
    image(report, m_sheetTechnicalInspection, 133, 1, imageChartFriction);

    // Страница: Отчет; Блок: Дата
    cell(report, m_sheetTechnicalInspection, 158, 12, otherParams.date);

    // Страница: Отчет; Блок: Диагностические графики
    cell(report, m_sheetGraphsOptionalTests, 1, 13, valveInfo.positionNumber);

    image(report, m_sheetGraphsOptionalTests, 5, 1, imageChartResponse);
    image(report, m_sheetGraphsOptionalTests, 30, 1, imageChartResolution);
    image(report, m_sheetGraphsOptionalTests, 55, 1, imageChartStep);

    // Страница: Отчет; Блок: Дата
    cell(report, m_sheetGraphsOptionalTests, 80, 12, otherParams.date);

    report.validation.push_back({"=ЗИП!$A$1:$A$37", "J56:J65"});
    report.validation.push_back({"=Заключение!$B$1:$B$4", "E42"});
    report.validation.push_back({"=Заключение!$C$1:$C$3", "E44"});
    report.validation.push_back({"=Заключение!$E$1:$E$4", "E46"});
    report.validation.push_back({"=Заключение!$D$1:$D$5", "E48"});
    report.validation.push_back({"=Заключение!$F$3", "E50"});
}
