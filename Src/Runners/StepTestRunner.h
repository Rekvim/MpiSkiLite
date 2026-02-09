#ifndef STEPTESTRUNNER_H
#define STEPTESTRUNNER_H

#pragma once
#include "BaseRunner.h"
#include "./Src/Tests/StepTest.h"
#include "./Src/Ui/TestSettings/StepTestSettings.h"

class StepTestRunner : public BaseRunner {
    Q_OBJECT
public:
    using BaseRunner::BaseRunner;

protected:
    RunnerConfig buildConfig() override;
    void wireSpecificSignals(Test& t) override;
};


#endif // STEPTESTRUNNER_H
