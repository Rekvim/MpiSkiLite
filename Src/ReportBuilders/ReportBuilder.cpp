#include "Src/ReportBuilders/ReportBuilder.h"
#include "Src/ReportBuilders/ReportWriter.h"

#include "Src/ReportBuilders/ReportBlocks/ObjectInfoBlock.h"
#include "Src/ReportBuilders/ReportBlocks/ValveSpecBlock.h"
#include "Src/ReportBuilders/ReportBlocks/MaterialsBlock.h"
#include "Src/ReportBuilders/ReportBlocks/StepReactionBlock.h"
#include "Src/ReportBuilders/ReportBlocks/TechnicalResultsBlock.h"

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
    )
{ // 168
    ReportWriter writer(report);

    ReportContext ctx{
        telemetryStore,
        objectInfo,
        valveInfo,
        otherParams,
        materialsOfComponentParts,
        imageChartTask,
        imageChartPressure,
        imageChartFriction,
        imageChartStep
    };

    writer.cell(m_sheetStepReactionTest, 1, 9, ctx.valve.positionNumber);

    ObjectInfoBlock({m_sheetStepReactionTest, 4, 4}).build(writer, ctx);
    ValveSpecBlock({m_sheetStepReactionTest, 4, 13}).build(writer, ctx);
    StepReactionBlock({m_sheetStepReactionTest, 18, 2, 55}).build(writer, ctx);

    writer.cell(m_sheetStepReactionTest, 76, 12, ctx.params.date);

    ObjectInfoBlock({m_sheetTechnicalInspection, 5, 4}).build(writer, ctx);
    ValveSpecBlock({m_sheetTechnicalInspection, 5, 13}).build(writer, ctx);
    MaterialsBlock({m_sheetTechnicalInspection, 11, 4})
        .build(writer, ctx);

    TechnicalResultsBlock({m_sheetTechnicalInspection,
                              29, // rowStart
                              5, // colFact
                              8, // colNorm
                              11, // colResult
                              51 // rowStrokeTime
                          }).build(writer, ctx);

    writer.validation(m_sheetTechnicalInspection, "=Заключение!$B$1:$B$4", "E41");
    writer.validation(m_sheetTechnicalInspection, "=Заключение!$C$1:$C$3", "E43");
    writer.validation(m_sheetTechnicalInspection, "=Заключение!$E$1:$E$4", "E45");
    writer.validation(m_sheetTechnicalInspection, "=Заключение!$D$1:$D$5", "E47");
    writer.validation(m_sheetTechnicalInspection, "=Заключение!$F$3", "E49");

    writer.validation(m_sheetTechnicalInspection, "=ЗИП!$A$1:$A$37", "J55:J64");

    // Дата и Исполнитель
    writer.cell(m_sheetTechnicalInspection, 65, 12, ctx.params.date);
    writer.cell(m_sheetTechnicalInspection, 73, 4, ctx.object.FIO);

    // Страница: Отчет; Блок: Диагностические графики
    writer.image(m_sheetTechnicalInspection, 83, 1, imageChartTask);
    writer.image(m_sheetTechnicalInspection, 108, 1, imageChartPressure);
    writer.image(m_sheetTechnicalInspection, 133, 1, imageChartFriction);

    // Страница: Отчет; Блок: Дата
    writer.cell(m_sheetTechnicalInspection, 158, 12, ctx.params.date);

    // Страница: Отчет; Блок: Диагностические графики
    writer.cell(m_sheetGraphsOptionalTests, 1, 13, ctx.valve.positionNumber);

    writer.image(m_sheetGraphsOptionalTests, 5, 1, imageChartResponse);
    writer.image(m_sheetGraphsOptionalTests, 30, 1, imageChartResolution);
    writer.image(m_sheetGraphsOptionalTests, 55, 1, imageChartStep);

    // Страница: Отчет; Блок: Дата
    writer.cell(m_sheetGraphsOptionalTests, 80, 12, ctx.params.date);
}
