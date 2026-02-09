#ifndef CYCLICTESTSETTINGS_H
#define CYCLICTESTSETTINGS_H

#pragma once

#include <QDialog>
#include <QTime>
#include <QListWidget>
#include <array>

#include "AbstractTestSettings.h"
#include "./Src/Ui/Setup/SelectTests.h"

namespace Ui {
class CyclicTestSettings;
}

class CyclicTestSettings : public AbstractTestSettings
{
    Q_OBJECT
public:
    explicit CyclicTestSettings(QWidget* parent = nullptr);
    ~CyclicTestSettings() = default;

    void applyPattern(SelectTests::PatternType pattern) override;
    void applyValveInfo(const ValveInfo& info) override;

    struct TestParameters
    {
        enum Type { Regulatory, Shutoff, Combined } testType = Regulatory;

        QVector<qreal> regSeqValues;
        quint16 regulatory_numCycles = 0;
        quint32 regulatory_delayMs = 0;
        quint32 regulatory_holdMs = 0;
        bool regulatory_enable_20mA = false;

        QVector<qreal> offSeqValues;
        quint16 shutoff_numCycles = 0;
        quint32 shutoff_delayMs = 0;
        quint32 shutoff_holdMs = 0;
        std::array<bool,4> shutoff_DO {{false,false,false,false}};
        bool shutoff_DI[2] {false,false};
    };

    TestParameters getParameters() const { return m_parameters; }

private slots:
    void onTestSelectionChanged();
    void on_pushButton_start_clicked();

    void on_pushButton_addRangeRegulatory_clicked();
    void on_pushButton_editRangeRegulatory_clicked();
    void on_pushButton_removeRangeRegulatory_clicked();

    void on_pushButton_addDelayRegulatory_clicked();
    void on_pushButton_editDelayRegulatory_clicked();
    void on_pushButton_removeDelayRegulatory_clicked();

    void on_pushButton_addDelayShutOff_clicked();
    void on_pushButton_editDelayShutOff_clicked();
    void on_pushButton_removeDelayShutOff_clicked();

private:
    Ui::CyclicTestSettings* ui;
    TestParameters m_parameters;
    void setPattern(SelectTests::PatternType pattern);

    SelectTests::PatternType m_pattern = SelectTests::Pattern_None;

    // === helpers ===
    void fillDefaultRegulatoryPresets();
    void fillDefaultShutOffPresets();

    void bindRegulatoryPresetEditor();
    static QString reverseSequenceString(const QString& s);

    inline static const QTime kMaxHold = QTime(0,1,5,0);
    inline static const QTime kMinHold = QTime(0,0,0,0);

protected:
    virtual QVector<qreal>& sequence() {
        static QVector<qreal> dummy;
        return dummy;
    }

    virtual QListWidget* sequenceListWidget() {
        return nullptr;
    }
};

#endif // CYCLICTESTSETTINGS_H
