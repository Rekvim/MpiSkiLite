#pragma once

#include "IReportBlock.h"

struct ValveSpecLayout {
    QString sheet;
    quint16 rowStart;
    quint16 column;
};

class ValveSpecBlock : public IReportBlock {
public:
    explicit ValveSpecBlock(ValveSpecLayout layout)
        : m_layout(layout) {}

    void build(ReportWriter& writer,
               const ReportContext& ctx) override
    {
        quint16 row = m_layout.rowStart;
        const auto& valve = ctx.valve;
        const auto& telemetry = ctx.telemetry;
        const auto& params = ctx.params;

        writer.cell(m_layout.sheet, row++, m_layout.column, valve.positionNumber);
        writer.cell(m_layout.sheet, row++, m_layout.column, valve.serialNumber);
        writer.cell(m_layout.sheet, row++, m_layout.column, valve.valveModel);
        writer.cell(m_layout.sheet, row++, m_layout.column, valve.manufacturer);
        writer.cell(m_layout.sheet, row++, m_layout.column, QString("%1 / %2").arg(valve.DN, valve.PN));
        writer.cell(m_layout.sheet, row++, m_layout.column, valve.positionerModel);
        writer.cell(m_layout.sheet, row++, m_layout.column, QString::number(telemetry.supplyRecord.pressure_bar, 'f', 2));
        writer.cell(m_layout.sheet, row++, m_layout.column, params.safePosition);
        writer.cell(m_layout.sheet, row++, m_layout.column, valve.driveModel);
        writer.cell(m_layout.sheet, row++, m_layout.column, params.strokeMovement);
        writer.cell(m_layout.sheet, row++, m_layout.column, valve.materialStuffingBoxSeal);
    }

private:
    ValveSpecLayout m_layout;
};
