#pragma once

#include "../ReportWriter.h"
#include "../ReportBuilder.h"

class IReportBlock {
public:
    virtual ~IReportBlock() = default;

    virtual void build(
        ReportWriter& writer,
        const ReportContext& ctx) = 0;
};
