#pragma once

#include "IReportBlock.h"

struct StepReactionLayout {
    QString sheet;
    quint16 imageRow;
    quint16 imageCol;
    quint16 resultsRowStart;
};

class StepReactionBlock : public IReportBlock {
public:
    explicit StepReactionBlock(StepReactionLayout layout)
        : m_layout(std::move(layout)) {}

    void build(ReportWriter& writer,
               const ReportContext& ctx) override
    {
        writer.image(m_layout.sheet,
                     m_layout.imageRow,
                     m_layout.imageCol,
                     ctx.chartStep);

        quint16 row = m_layout.resultsRowStart;

        for (const auto& sr : ctx.telemetry.stepResults) {
            writer.cell(m_layout.sheet, row, 3,
                        QString("%1->%2").arg(sr.from).arg(sr.to));

            writer.cell(m_layout.sheet, row, 5,
                        QTime(0,0)
                            .addMSecs(sr.T_value)
                            .toString("m:ss.zzz"));

            writer.cell(m_layout.sheet, row, 7,
                        QString::number(sr.overshoot, 'f', 2));

            ++row;
        }
    }

private:
    StepReactionLayout m_layout;
};
