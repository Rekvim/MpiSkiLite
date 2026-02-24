#pragma once

#include "Src/ReportBuilders/ReportWriter.h"
#include "Src/ReportBuilders/ReportBuilder.h"

class TechnicalResultsBlock {
public:
    struct Layout {
        QString sheet;
        quint16 rowStart; // 22
        quint16 colFact; // 5
        quint16 colNorm; // 8
        quint16 colResult; // 11
        quint16 rowStrokeTime; // 48
    };

    explicit TechnicalResultsBlock(Layout l) : m(l) {}

    void build(ReportWriter& w, const ReportContext& ctx) const;

private:
    QString resultOk(CrossingStatus::State state) const;
    QString resultLimit(CrossingStatus::State state) const;

    Layout m;
};
