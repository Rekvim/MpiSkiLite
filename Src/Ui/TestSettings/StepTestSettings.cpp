#include <QInputDialog>
#include "StepTestSettings.h"
#include "ui_StepTestSettings.h"

StepTestSettings::StepTestSettings(QWidget* parent)
    : AbstractTestSettings(parent)
    , ui(new Ui::StepTestSettings)
{
    ui->setupUi(this);

    m_sequence = {10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 70.0, 80.0, 90.0};
    fillNumericList(ui->listWidget_value, m_sequence);

    bindNumericListEditor(
        ui->listWidget_value,
        ui->pushButton_add_value,
        ui->pushButton_change_value,
        ui->pushButton_delete_value,
        "50.0"
        );

    clampTime(ui->timeEdit, m_minTime, m_maxTime);
}

QVector<qreal>& StepTestSettings::sequence()
{
    return m_sequence;
}

QListWidget* StepTestSettings::sequenceListWidget()
{
    return ui->listWidget_value;
}

StepTestSettings::~StepTestSettings()
{
    delete ui;
}

StepTestSettings::TestParameters StepTestSettings::getParameters()
{
    TestParameters testParameters;

    testParameters.delay = ui->timeEdit->time().msecsSinceStartOfDay();
    testParameters.testValue = ui->spinBox_T_value->value();
    for (int i = 0; i < ui->listWidget_value->count(); i++) {
        testParameters.points.append(ui->listWidget_value->item(i)->text().toDouble());
    }

    return testParameters;
}

void StepTestSettings::applyValveInfo(const ValveInfo& info)
{
    if (info.safePosition != 0)
        reverseSequence();
}

void StepTestSettings::applyPattern(SelectTests::PatternType pattern)
{
    Q_UNUSED(pattern);
}
