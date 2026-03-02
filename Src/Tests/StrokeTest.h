#pragma once
#include <QPointF>
#include <QTimer>
#include <QDateTime>
#include "Test.h"

class StrokeTest : public Test
{
    Q_OBJECT
public:
    explicit StrokeTest(QObject *parent = nullptr);
    void Process() override;

    struct Config {
        bool normalClosed = true; // NC: low->high->low, NO: high->low->high
    };
    void setConfig(const Config& cfg) { m_cfg = cfg; }

signals:
    void GetPoints(QVector<QVector<QPointF>> &points);
    void SetStartTime();
    void Results(quint64 forwardTime, quint64 backwardTime);

private:
    Config m_cfg; // <-- ДОБАВИТЬ
};
