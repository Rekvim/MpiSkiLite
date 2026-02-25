#include "TechnicalResultsBlock.h"

static QString f2(qreal v) { return QString::number(v, 'f', 2); }
static QString f3(qreal v) { return QString::number(v, 'f', 3); }

QString TechnicalResultsBlock::resultOk(CrossingStatus::State state) const {
    using State = CrossingStatus::State;
    switch (state) {
    case State::Ok: return "соответствует";
    case State::Fail: return "не соответствует";
    case State::Unknown: return "не определено";
    }
    return {};
}

QString TechnicalResultsBlock::resultLimit(CrossingStatus::State state) const {
    using State = CrossingStatus::State;
    switch (state) {
    case State::Ok: return "не превышает";
    case State::Fail: return "превышает";
    case State::Unknown: return "не определено";
    }
    return {};
}

void TechnicalResultsBlock::build(ReportWriter& w,
                                  const ReportContext& ctx) const
{
    const auto& t = ctx.telemetry;
    const auto& v = ctx.valve;

    const quint16 r0 = m_layout.rowStart; // dynamic error
    const quint16 r1 = r0 + 2; // stroke
    const quint16 r2 = r0 + 4; // spring
    const quint16 r3 = r0 + 6; // pressure
    const quint16 r4 = r0 + 8; // friction %
    const quint16 r5 = r0 + 10; // friction force

    // dynamic error
    w.cell(m_layout.sheet, r0, m_layout.colFact, f2(t.mainTestRecord.dynamicErrorReal));
    w.cell(m_layout.sheet, r0, m_layout.colNorm, f2(v.dinamicErrorRecomend));
    w.cell(m_layout.sheet, r0, m_layout.colResult, resultOk(t.crossingStatus.dynamicError));

    // stroke
    w.cell(m_layout.sheet, r1, m_layout.colFact, f2(t.valveStrokeRecord.real));
    w.cell(m_layout.sheet, r1, m_layout.colNorm, v.strokValve);
    w.cell(m_layout.sheet, r1, m_layout.colResult, resultOk(t.crossingStatus.range));

    // spring
    w.cell(m_layout.sheet, r2, m_layout.colFact,
           QString("%1–%2").arg(f2(t.mainTestRecord.springLow),
                                f2(t.mainTestRecord.springHigh)));
    w.cell(m_layout.sheet, r2, m_layout.colNorm,
           QString("%1–%2").arg(f2(v.driveRangeLow),
                                f2(v.driveRangeHigh)));
    w.cell(m_layout.sheet, r2, m_layout.colResult, resultOk(t.crossingStatus.spring));

    // pressure limits
    w.cell(m_layout.sheet, r3, m_layout.colFact,
           QString("%1–%2").arg(f2(t.mainTestRecord.lowLimitPressure),
                                f2(t.mainTestRecord.highLimitPressure)));

    // friction percent
    w.cell(m_layout.sheet, r4, m_layout.colFact, f2(t.mainTestRecord.frictionPercent));
    w.cell(m_layout.sheet, r4, m_layout.colResult, resultLimit(t.crossingStatus.frictionPercent));

    // friction force
    w.cell(m_layout.sheet, r5, m_layout.colFact, f3(t.mainTestRecord.frictionForce));

    // stroke times
    w.cell(m_layout.sheet, m_layout.rowStrokeTime, 5, t.strokeTestRecord.timeForwardMs);
    w.cell(m_layout.sheet, m_layout.rowStrokeTime, 8, t.strokeTestRecord.timeBackwardMs);
}
