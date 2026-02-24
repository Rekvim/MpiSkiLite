#pragma once

#include "ReportSaver.h"
#include "./Src/Telemetry/TelemetryStore.h"
#include "./Registry.h"

struct ReportContext {
    const TelemetryStore& telemetry;
    const ObjectInfo& object;
    const ValveInfo& valve;
    const OtherParameters& params;
    const MaterialsOfComponentParts& materials;
    const QImage& chartTask;
    const QImage& chartPressure;
    const QImage& chartFriction;
    const QImage& chartStep;
};

class ReportBuilder {
public:
    ReportBuilder() = default;
    ~ReportBuilder() = default;

    void buildReport(
        ReportSaver::Report &report,
        const TelemetryStore &telemetryStore,
        const ObjectInfo &objectInfo,
        const ValveInfo &valveInfo,
        const OtherParameters &otherParams,
        const MaterialsOfComponentParts &materialsOfComponentParts,
        const QImage &imageChartTask = QImage(),
        const QImage &imageChartPressure = QImage(),
        const QImage &imageChartFriction = QImage(),
        const QImage &imageChartResponse = QImage(),
        const QImage &imageChartResolution = QImage(),
        const QImage &imageChartStep = QImage()
        );

    QString templatePath() const { return QStringLiteral(":/excel/Reports/Report.xlsx"); }

private:
    QString m_sheetStepReactionTest = "ТШР";
    QString m_sheetGraphsOptionalTests = "Графики Опц Тестов";
    QString m_sheetTechnicalInspection = "ТО";
};
