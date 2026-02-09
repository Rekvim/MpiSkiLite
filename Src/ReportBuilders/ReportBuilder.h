#ifndef REPORTBUILDER_H
#define REPORTBUILDER_H

#pragma once
#include "ReportSaver.h"
#include "./Src/Telemetry/TelemetryStore.h"
#include "./Registry.h"

#include <QWidget>

class ReportBuilder {
public:
    ReportBuilder() = default;
    ~ReportBuilder() = default;

    void buildReport(
        ReportSaver::Report& report,
        const TelemetryStore& telemetryStore,
        const ObjectInfo& objectInfo,
        const ValveInfo& valveInfo,
        const OtherParameters& otherParams,
        const MaterialsOfComponentParts& materialsOfComponentParts,
        const QImage& imageChartTask = QImage(),
        const QImage& imageChartPressure = QImage(),
        const QImage& imageChartFriction = QImage(),
        const QImage& imageChartResponse = QImage(),
        const QImage& imageChartResolution = QImage(),
        const QImage& imageChartStep = QImage()
        );

    QString templatePath() const { return QStringLiteral(":/excel/Reports/Report.xlsx"); }

private:
    QString m_sheetStepReactionTest = "ТШР";
    QString m_sheetGraphsOptionalTests = "Графики Опц Тестов";
    QString m_sheetTechnicalInspection = "ТО";

    QString resultOk(CrossingStatus::State state) const {
        using State = CrossingStatus::State;
        switch (state) {
        case State::Ok: return "соответствует";
        case State::Fail: return "не соответствует";
        case State::Unknown: return "не определено";
        }
        return {};
    }

    QString resultLimit(CrossingStatus::State state) const {
        using State = CrossingStatus::State;
        switch (state) {
        case State::Ok: return "не превышает";
        case State::Fail: return "превышает";
        case State::Unknown: return "не определено";
        }
        return {};
    }

    void cell(ReportSaver::Report& report,
              const QString& sheet,
              quint16 row, quint16 col,
              const QVariant& value)
    {
        report.data.push_back({sheet, row, col, value.toString()});
    }

    void image(ReportSaver::Report& report,
               const QString& sheet,
               quint16 row, quint16 col,
               const QImage& img)
    {
        report.images.push_back({sheet, row, col, img});
    }
};

#endif // REPORTBUILDER_H
