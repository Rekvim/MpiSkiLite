#ifndef OPTIONRESOLUTIONRUNNER_H
#define OPTIONRESOLUTIONRUNNER_H

#pragma once
#include "BaseRunner.h"
#include "./Src/Ui/TestSettings/OtherTestSettings.h"

class OptionResolutionRunner : public BaseRunner {
    Q_OBJECT
public:
    using BaseRunner::BaseRunner;

signals:
    void getParameters_resolutionTest(OtherTestSettings::TestParameters&);

protected:
    RunnerConfig buildConfig() override;
    void wireSpecificSignals(Test& t) override;
};



#endif // OPTIONRESOLUTIONRUNNER_H
