#include "./Src/CustomChart/MySeries.h"

MySeries::MySeries(QObject *parent, quint8 axN)
    : QLineSeries{parent}
{
    m_axisN = axN;
}

quint8 MySeries::getAxisN() const
{
    return m_axisN;
}

void MySeries::setMarkersOnly(bool on)
{
    m_markersOnly = on;

    setUseOpenGL(!on);

    setPointsVisible(true);

    QPen p = pen();
    p.setStyle(on ? Qt::NoPen : Qt::SolidLine);
    setPen(p);

    #if QT_VERSION >= QT_VERSION_CHECK(6,2,0)
        if (on) setMarkerSize(6.0);
    #endif
}

bool MySeries::isMarkersOnly() const
{
    return m_markersOnly;
}
