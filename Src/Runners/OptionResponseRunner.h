#ifndef OPTIONRESPONSERUNNER_H
#define OPTIONRESPONSERUNNER_H

#pragma once
#include "BaseRunner.h"
#include "./Src/Ui/TestSettings/OtherTestSettings.h"

class OptionResponseRunner : public BaseRunner {
    Q_OBJECT
public:
    using BaseRunner::BaseRunner;

signals:
    void getParameters_responseTest(OtherTestSettings::TestParameters&);

protected:
    RunnerConfig buildConfig() override;
    void wireSpecificSignals(Test& t) override;
};


#endif // OPTIONRESPONSERUNNER_H
