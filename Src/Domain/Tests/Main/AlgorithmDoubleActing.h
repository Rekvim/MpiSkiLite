#pragma once

#include "Algorithm.h"

namespace Domain::Tests::Main {

class AlgorithmDoubleActing : public Algorithm
{
    Q_OBJECT

public:
    explicit AlgorithmDoubleActing(Params params, QObject* parent = nullptr)
        : Algorithm(std::move(params), parent)
    {
    }

    void run() override;

signals:
    void waitingForManualResume();
};

}
