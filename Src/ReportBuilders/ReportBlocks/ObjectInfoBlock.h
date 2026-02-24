#pragma once

#include "IReportBlock.h"

struct ObjectInfoLayout {
    QString sheet;
    quint16 rowStart;
    quint16 column;
};

class ObjectInfoBlock : public IReportBlock {
public:
    explicit ObjectInfoBlock(ObjectInfoLayout layout)
        : m_layout(layout) {}

    void build(ReportWriter& writer,
               const ReportContext& ctx) override
    {
        writer.cell(m_layout.sheet, m_layout.rowStart++, m_layout.column, ctx.object.object);
        writer.cell(m_layout.sheet, m_layout.rowStart++, m_layout.column, ctx.object.manufactory);
        writer.cell(m_layout.sheet, m_layout.rowStart++, m_layout.column, ctx.object.department);
    }

private:
    ObjectInfoLayout m_layout;
};
