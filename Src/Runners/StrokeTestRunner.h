#ifndef STROKETESTRUNNER_H
#define STROKETESTRUNNER_H

#pragma once
#include "BaseRunner.h"
#include "./Src/Tests/StrokeTest.h"

class StrokeTestRunner : public BaseRunner {
    Q_OBJECT
public:
    using BaseRunner::BaseRunner;

protected:
    RunnerConfig buildConfig() override;
    void wireSpecificSignals(Test& t) override;
};

#endif // STROKETESTRUNNER_H
