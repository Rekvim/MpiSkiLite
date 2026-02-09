#ifndef OTHERTESTSETTINGS_H
#define OTHERTESTSETTINGS_H

#pragma once
#include <QInputDialog>
#include <QDialog>
#include <QListWidget>

#include "qdatetime.h"
#include "AbstractTestSettings.h"

namespace Ui {
class OtherTestSettings;
}

class OtherTestSettings : public AbstractTestSettings
{
    Q_OBJECT

public:
    explicit OtherTestSettings(QWidget* parent = nullptr);
    ~OtherTestSettings();

    void applyValveInfo(const ValveInfo& info) override;
    void applyPattern(SelectTests::PatternType pattern) override;

    struct TestParameters
    {
        quint32 delay;
        QVector<qreal> points;
        QVector<qreal> steps;
    };

    TestParameters getParameters();

private:
    Ui::OtherTestSettings *ui;

    QVector<qreal> m_sequence;

    const QTime m_maxTime = QTime(0, 4, 0, 0);
    const QTime m_minTime = QTime(0, 0, 5, 0);

protected:
    QVector<qreal>& sequence() override;
    QListWidget* sequenceListWidget() override;
};

#endif // OTHERTESTSETTINGS_H
