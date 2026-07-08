#pragma once

#include <QPointF>
#include <QTimer>

#include "Domain/Tests/Main/Algorithm.h"

class CyclicTestPositioner : public Domain::Tests::Main::Algorithm
{
    Q_OBJECT
public:
    explicit CyclicTestPositioner(Domain::Tests::Main::Params params, QObject *parent = nullptr);
    void run() override;

private:
    QTimer* m_cyclicGraphTimer;
signals:
    void updateCyclicTred();
};
