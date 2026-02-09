#ifndef MAINTESTRUNNER_H
#define MAINTESTRUNNER_H

#pragma once
#include <QObject>
#include "BaseRunner.h"
#include "./Src/Ui/TestSettings/MainTestSettings.h"

class MainTestRunner : public BaseRunner {
    Q_OBJECT
public:
    using BaseRunner::BaseRunner;

signals:
    void getParameters_mainTest(MainTestSettings::TestParameters&);

protected:
    RunnerConfig buildConfig() override;
    void wireSpecificSignals(Test& t) override;
};

#endif // MAINTESTRUNNER_H
