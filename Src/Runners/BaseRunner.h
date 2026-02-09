#ifndef BASERUNNER_H
#define BASERUNNER_H

#include <QThread>
#include <QMetaObject>
#include <QObject>
#include "./Src/Runners/AbstractTestRunner.h"

class Test;
class QThread;
class Mpi;
class Registry;

struct RunnerConfig {
    Test* worker = nullptr;
    quint64 totalMs = 0;
    int chartToClear = -1;
};

class BaseRunner : public AbstractTestRunner {
    Q_OBJECT
public:
    BaseRunner(Mpi& mpi, Registry& reg, QObject* parent=nullptr);
    ~BaseRunner() override;

public slots:
    void start() final;
    void stop() override;
    void releaseBlock() final override;

protected:
    virtual RunnerConfig buildConfig() = 0;
    virtual void wireSpecificSignals(Test& t) {}

    Mpi& m_mpi;
    Registry& m_reg;

private:
    QThread* m_thread = nullptr;
    Test* m_worker = nullptr;
};

#endif // BASERUNNER_H
