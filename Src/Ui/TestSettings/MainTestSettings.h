#ifndef MAINTESTSETTINGS_H
#define MAINTESTSETTINGS_H

#pragma once
#include <QDialog>
#include <QMetaType>
#include "AbstractTestSettings.h"

namespace Ui {
class MainTestSettings;
}

class MainTestSettings :public AbstractTestSettings
{
    Q_OBJECT

public:
    explicit MainTestSettings(QWidget *parent = nullptr);
    ~MainTestSettings();

    void applyValveInfo(const ValveInfo& info) override;
    void applyPattern(SelectTests::PatternType pattern) override;

    struct TestParameters
    {
        bool continuous;
        quint64 Time;
        QList<qreal> points;
        QList<qreal> steps;
        quint64 delay;
        quint16 pointNumbers;
        qreal signal_min;
        qreal signal_max;
        quint16 response;
        quint16 dac_min;
        quint16 dac_max;
        bool is_cyclic = false;
        quint16 num_cycles;
        quint16 cycle_time;
    };

    TestParameters getParameters() const;

private:
    Ui::MainTestSettings *ui;

protected:
    virtual QVector<qreal>& sequence() {
        static QVector<qreal> dummy;
        return dummy;
    }

    virtual QListWidget* sequenceListWidget() {
        return nullptr;
    }
};

Q_DECLARE_METATYPE(MainTestSettings::TestParameters)

#endif // MAINTESTSETTINGS_H
