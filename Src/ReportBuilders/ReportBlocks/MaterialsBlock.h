#pragma once

#include "IReportBlock.h"

struct MaterialsLayout {
    QString sheet;
    quint16 rowStart;
    quint16 column;
};

class MaterialsBlock : public IReportBlock
{
public:
    explicit MaterialsBlock(MaterialsLayout layout)
        : m_layout(std::move(layout)) {}

    void build(ReportWriter& writer,
               const ReportContext& ctx) override
    {
        const auto& m = ctx.materials;

        quint16 row = m_layout.rowStart;
        const quint16 col = m_layout.column;

        writer.cell(m_layout.sheet, row++, col, m.corpus);
        writer.cell(m_layout.sheet, row++, col, m.cap);
        writer.cell(m_layout.sheet, row, col, m.saddle);
        writer.cell(m_layout.sheet, row++, col + 2, m.CV);
        writer.cell(m_layout.sheet, row++, col, m.ball);
        writer.cell(m_layout.sheet, row++, col, m.disk);
        writer.cell(m_layout.sheet, row++, col, m.plunger);
        writer.cell(m_layout.sheet, row++, col, m.shaft);
        writer.cell(m_layout.sheet, row++, col, m.stock);
        writer.cell(m_layout.sheet, row++, col, m.guideSleeve);
    }

private:
    MaterialsLayout m_layout;
};
