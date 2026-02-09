#include "./Src/ReportBuilders/ReportBuilder.h"

void ReportBuilder::buildReport(
    ReportSaver::Report& report,
    const TelemetryStore& telemetryStore,
    const ObjectInfo& objectInfo,
    const ValveInfo& valveInfo,
    const OtherParameters& otherParams,
    const MaterialsOfComponentParts& materialsOfComponentParts,
    const QImage& imageChartTask,
    const QImage& imageChartPressure,
    const QImage& imageChartFriction,
    const QImage& imageChartResponse,
    const QImage& imageChartResolution,
    const QImage& imageChartStep
    ) {
    QString sheet_1 = "ТШР";
    QString sheet_2 = "ТО";
    QString sheet_3 = "Графики Опц Тестов";

    cell(report, sheet_1, 1, 9, valveInfo.positionNumber);

    // Лист: Результат теста шаговой реакции; Страница: 2; Блок: Данные по объекту
    cell(report, sheet_1, 4, 4, objectInfo.object);
    cell(report, sheet_1, 5, 4, objectInfo.manufactory);
    cell(report, sheet_1, 6, 4, objectInfo.department);

    // Страница:Результат теста шаговой реакции; Блок: Краткая спецификация на клапан
    cell(report, sheet_1, 4, 13, valveInfo.positionNumber);
    cell(report, sheet_1, 5, 13, valveInfo.serialNumber);
    cell(report, sheet_1, 6, 13, valveInfo.valveModel);
    cell(report, sheet_1, 7, 13, valveInfo.manufacturer);
    cell(report, sheet_1, 8, 13, QString("%1 / %2").arg(valveInfo.DN, valveInfo.PN));
    cell(report, sheet_1, 9, 13, valveInfo.positionerModel);
    cell(report, sheet_1, 10, 13, QString("%1")
                                      .arg(telemetryStore.supplyRecord.pressure_bar, 0, 'f', 2));
    cell(report, sheet_1, 11, 13, otherParams.safePosition);
    cell(report, sheet_1, 12, 13, valveInfo.driveModel);
    cell(report, sheet_1, 13, 13, otherParams.strokeMovement);
    cell(report, sheet_1, 14, 13, valveInfo.materialStuffingBoxSeal);

    // Страница: Результат теста шаговой реакции; Блок: График теста шаговой реакции
    report.images.push_back({sheet_1, 18, 2, imageChartStep});

    // Страница: Результат теста шаговой реакции; Блок: Результат теста шаговой реакции
    {
        quint16 row = 55;
        for (auto &sr : telemetryStore.stepResults) {
            cell(report, sheet_1, row, 3, QString("%1->%2").arg(sr.from).arg(sr.to));
            cell(report, sheet_1, row, 4, QTime(0,0).addMSecs(sr.T_value).toString("m:ss.zzz"));
            cell(report, sheet_1, row, 5, QString("%1").arg(sr.overshoot, 0, 'f', 2));
            ++row;
        }
    }

    // Страница: Отчет ЦТ; Блок: Дата
    cell(report, sheet_1, 76, 12, otherParams.date);

    // Страница: Отчет; Блок: Данные по объекту
    cell(report, sheet_2, 5, 4, objectInfo.object);
    cell(report, sheet_2, 6, 4, objectInfo.manufactory);
    cell(report, sheet_2, 7, 4, objectInfo.department);

    // Страница:Отчет; Блок: Краткая спецификация на клапан
    cell(report, sheet_2, 5, 13, valveInfo.positionNumber);
    cell(report, sheet_2, 6, 13, valveInfo.serialNumber);
    cell(report, sheet_2, 7, 13, valveInfo.valveModel);
    cell(report, sheet_2, 8, 13, valveInfo.manufacturer);
    cell(report, sheet_2, 9, 13, QString("%1 / %2").arg(valveInfo.DN, valveInfo.PN));
    cell(report, sheet_2, 10, 13, valveInfo.positionerModel);
    cell(report, sheet_2, 11, 13, QString("%1").arg(telemetryStore.supplyRecord.pressure_bar, 0, 'f', 2));
    cell(report, sheet_2, 12, 13, otherParams.safePosition);
    cell(report, sheet_2, 13, 13, valveInfo.driveModel);
    cell(report, sheet_2, 14, 13, otherParams.strokeMovement);
    cell(report, sheet_2, 15, 13, valveInfo.materialStuffingBoxSeal);

    // Материал деталей
    report.data.push_back({sheet_2, 11, 4, materialsOfComponentParts.corpus});
    report.data.push_back({sheet_2, 12, 4, materialsOfComponentParts.cap});
    report.data.push_back({sheet_2, 13, 4, materialsOfComponentParts.saddle});
    report.data.push_back({sheet_2, 13, 6, materialsOfComponentParts.CV});
    report.data.push_back({sheet_2, 14, 4, materialsOfComponentParts.ball});
    report.data.push_back({sheet_2, 15, 4, materialsOfComponentParts.disk});
    report.data.push_back({sheet_2, 16, 4, materialsOfComponentParts.plunger});
    report.data.push_back({sheet_2, 17, 4, materialsOfComponentParts.shaft});
    report.data.push_back({sheet_2, 18, 4, materialsOfComponentParts.stock});
    report.data.push_back({sheet_2, 19, 4, materialsOfComponentParts.guideSleeve});


    // Страница: Отчет; Блок: Результат испытаний
    cell(report, sheet_2, 29, 5,
         QString("%1")
             .arg(telemetryStore.mainTestRecord.dynamicErrorReal, 0, 'f', 2));

    cell(report, sheet_2, 29, 8,
         QString("%1")
             .arg(valveInfo.dinamicErrorRecomend, 0, 'f', 2));
    cell(report, sheet_2, 29, 11, resultOk(telemetryStore.crossingStatus.dynamicError));

    cell(report, sheet_2, 31, 5, QString("%1")
                                     .arg(telemetryStore.valveStrokeRecord.real, 0, 'f', 2));
    cell(report, sheet_2, 31, 8, valveInfo.strokValve);
    cell(report, sheet_2, 31, 11, resultOk(telemetryStore.crossingStatus.range));

    cell(report, sheet_2, 33, 5,
         QString("%1–%2")
             .arg(telemetryStore.mainTestRecord.springLow, 0, 'f', 2)
             .arg(telemetryStore.mainTestRecord.springHigh, 0, 'f', 2)
         );
    cell(report, sheet_2, 33, 11, resultOk(telemetryStore.crossingStatus.spring));
    cell(report, sheet_2, 33, 8,
        QString("%1–%2")
            .arg(valveInfo.driveRangeLow, 0, 'f', 2)
            .arg(valveInfo.driveRangeHigh, 0, 'f', 2)
    );

    cell(report, sheet_2, 35, 5,
        QString("%1–%2")
            .arg(telemetryStore.mainTestRecord.lowLimitPressure, 0, 'f', 2)
            .arg(telemetryStore.mainTestRecord.highLimitPressure, 0, 'f', 2)
    );

    cell(report, sheet_2, 37, 5,
        QString("%1")
            .arg(telemetryStore.mainTestRecord.frictionPercent, 0, 'f', 2)
    );
    cell(report, sheet_2, 37, 11, resultLimit(telemetryStore.crossingStatus.frictionPercent));

    cell(report, sheet_2, 39, 5,
        QString("%1")
            .arg(telemetryStore.mainTestRecord.frictionForce, 0, 'f', 3)
    );
    cell(report,
        sheet_2, 51, 5, telemetryStore.strokeTestRecord.timeForwardMs
    );
    cell(report,
        sheet_2, 51, 8, telemetryStore.strokeTestRecord.timeBackwardMs
    );

    // Дата и Исполнитель
    cell(report, sheet_2, 65, 12, otherParams.date);
    cell(report, sheet_2, 73, 4, objectInfo.FIO);

    // Страница: Отчет; Блок: Диагностические графики
    image(report, sheet_2, 84, 1, imageChartTask);
    image(report, sheet_2, 109, 1, imageChartPressure);
    image(report, sheet_2, 134, 1, imageChartFriction);

    // Страница: Отчет; Блок: Дата
    cell(report, sheet_2, 158, 12, otherParams.date);

    // Страница: Отчет; Блок: Диагностические графики
    cell(report, sheet_3, 1, 13, valveInfo.positionNumber);

    image(report, sheet_3, 6, 1, imageChartResponse);
    image(report, sheet_3, 31, 1, imageChartResolution);
    image(report, sheet_3, 56, 1, imageChartStep);

    // Страница: Отчет; Блок: Дата
    cell(report, sheet_3, 80, 12, otherParams.date);

    report.validation.push_back({"=ЗИП!$A$1:$A$37", "J56:J65"});
    report.validation.push_back({"=Заключение!$B$1:$B$4", "E42"});
    report.validation.push_back({"=Заключение!$C$1:$C$3", "E44"});
    report.validation.push_back({"=Заключение!$E$1:$E$4", "E46"});
    report.validation.push_back({"=Заключение!$D$1:$D$5", "E48"});
    report.validation.push_back({"=Заключение!$F$3", "E50"});
}
