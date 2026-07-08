#include "Manager.h"

namespace Widgets::Chart {
ChartView* Manager::chart(ChartType type)
{
    return m_charts.value(type, nullptr);
}

void Manager::addPoints(Widgets::Chart::ChartType chart, const QVector<Widgets::Chart::Point> &points)
{
    if (!m_charts.contains(chart))
        return;

    for (const auto& p : points) {
        m_charts[chart]->addPoint(p.seriesNum, p.X, p.Y);
    }
}

void Manager::clearPoints(Widgets::Chart::ChartType chart)
{
    if (!m_charts.contains(chart))
        return;

    m_charts[chart]->clear();
}

void Manager::setVisible(ChartType chart, quint16 series, bool visible)
{
    if (!m_charts.contains(chart))
        return;

    m_charts[chart]->visible(series, visible);
}

void Manager::showDots(bool visible)
{
    m_charts[ChartType::Task]->showDots(visible);

    if (m_charts.contains(ChartType::Pressure))
        m_charts[ChartType::Pressure]->showDots(visible);
}

void Manager::duplicateMainChartsSeries()
{
    if (!m_charts.contains(ChartType::Task))
        return;

    auto* task = m_charts[ChartType::Task];

    task->duplicateChartSeries(1);
    task->duplicateChartSeries(2);
    task->duplicateChartSeries(3);
    task->duplicateChartSeries(4);

    if (m_charts.contains(ChartType::Pressure))
        m_charts[ChartType::Pressure]->duplicateChartSeries(0);
}

QPair<QList<QPointF>, QList<QPointF>>
Manager::getPoints(ChartType chart, int series)
{
    if (!m_charts.contains(chart))
        return {};

    return m_charts[chart]->getPoints(series);
}

QPixmap Manager::grabChart(ChartType chart)
{
    if (!m_charts.contains(chart))
        return {};

    return m_charts[chart]->grab();
}

ChartView* Manager::createTrendChart(ChartView* chart, const QColor& linearColor)
{
    m_charts[ChartType::Trend] = chart;

    chart->useTimeaxis(true);
    chart->addAxis(QStringLiteral("%.2f%%"));
    chart->addSeries(0, QObject::tr("Задание"), QColor::fromRgb(0, 0, 0));
    chart->addSeries(0, QObject::tr("Датчик линейных перемещений"), linearColor);
    chart->setMaxRange(60000);

    return chart;
}

ChartView* Manager::createStrokeChart(ChartView* chart, const QColor& linearColor)
{
    m_charts[ChartType::Stroke] = chart;

    chart->setName(QStringLiteral("Stroke"));
    chart->useTimeaxis(true);
    chart->addAxis(QStringLiteral("%.2f%%"));
    chart->addSeries(0, QObject::tr("Задание"), QColor::fromRgb(0, 0, 0));
    chart->addSeries(0, QObject::tr("Датчик линейных перемещений"), linearColor);

    return chart;
}

ChartView* Manager::createTaskChart(
    ChartView* chart,
    const QString& strokeAxisFormat,
    const QColor& linearColor,
    const QColor& pressure1Color,
    const QColor& pressure2Color,
    const QColor& pressure3Color
    ) {
    m_charts[ChartType::Task] = chart;

    chart->setName(QStringLiteral("Task"));
    chart->useTimeaxis(false);
    chart->addAxis(QStringLiteral("%.2f bar"));
    chart->addAxis(strokeAxisFormat);
    chart->addSeries(1, QObject::tr("Задание"), QColor::fromRgb(0, 0, 0));
    chart->addSeries(1, QObject::tr("Датчик линейных перемещений"), linearColor);
    chart->addSeries(0, QObject::tr("Датчик давления 1"), pressure1Color);
    chart->addSeries(0, QObject::tr("Датчик давления 2"), pressure2Color);
    chart->addSeries(0, QObject::tr("Датчик давления 3"), pressure3Color);

    return chart;
}

ChartView* Manager::createFrictionChart(
    ChartView* chart,
    const QString& strokeAxisFormat,
    const QColor& linearColor
    ) {
    m_charts[ChartType::Friction] = chart;

    chart->setName(QStringLiteral("Friction"));
    chart->addAxis(QStringLiteral("%.2f N"));
    chart->addSeries(0, QObject::tr("Трение от перемещения"), linearColor);
    chart->setLabelXformat(strokeAxisFormat);

    return chart;
}

ChartView* Manager::createPressureChart(
    ChartView* chart,
    const QString& strokeAxisFormat,
    const QColor& linearColor
    ) {
    m_charts[ChartType::Pressure] = chart;

    chart->setName(QStringLiteral("Pressure"));
    chart->useTimeaxis(false);
    chart->setLabelXformat(QStringLiteral("%.2f bar"));
    chart->addAxis(strokeAxisFormat);
    chart->addSeries(0, QObject::tr("Перемещение от давления"), linearColor);
    chart->addSeries(0, QObject::tr("Регрессия (прямой ход)"),  QColor::fromRgb(30, 100, 200));
    chart->addSeries(0, QObject::tr("Регрессия (обратный ход)"), QColor::fromRgb(200, 50, 50));
    chart->visible(1, false);
    chart->visible(2, false);
    chart->setSeriesDraggable(1, true);
    chart->setSeriesDraggable(2, true);

    return chart;
}

ChartView* Manager::createResponseChart(
    ChartView* chart,
    const QColor& linearColor
    ) {
    m_charts[ChartType::Response] = chart;

    chart->setName(QStringLiteral("Response"));
    chart->useTimeaxis(true);
    chart->addAxis(QStringLiteral("%.2f%%"));
    chart->addSeries(0, QObject::tr("Задание"), QColor::fromRgb(0, 0, 0));
    chart->addSeries(0, QObject::tr("Датчик линейных перемещений"), linearColor);

    return chart;
}

ChartView* Manager::createResolutionChart(
    ChartView* chart,
    const QColor& linearColor
    ) {
    m_charts[ChartType::Resolution] = chart;

    chart->setName(QStringLiteral("Resolution"));
    chart->useTimeaxis(true);
    chart->addAxis(QStringLiteral("%.2f%%"));
    chart->addSeries(0, QObject::tr("Задание"), QColor::fromRgb(0, 0, 0));
    chart->addSeries(0, QObject::tr("Датчик линейных перемещений"), linearColor);

    return chart;
}

ChartView* Manager::createStepChart(ChartView* chart, const QColor& linearColor)
{
    m_charts[ChartType::Step] = chart;

    chart->setName(QStringLiteral("Step"));
    chart->useTimeaxis(true);
    chart->addAxis(QStringLiteral("%.2f%%"));
    chart->addSeries(0, QObject::tr("Задание"), QColor(0,0,0));
    chart->addSeries(0, QObject::tr("Датчик линейных перемещений"), linearColor);

    return chart;
}
}