#include "OtherTestSettings.h"
#include "ui_OtherTestSettings.h"
#include "Registry.h"

OtherTestSettings::OtherTestSettings(QWidget* parent)
    : AbstractTestSettings(parent)
    , ui(new Ui::OtherTestSettings)
{
    ui->setupUi(this);

    m_sequence = {25.0, 50.0, 75.0};
    fillNumericList(ui->listWidget_value, m_sequence);

    bindNumericListEditor(
        ui->listWidget_value,
        ui->pushButton_add_value,
        ui->pushButton_change_value,
        ui->pushButton_delete_value,
        "50.0"
        );

    clampTime(ui->timeEdit, m_minTime, m_maxTime);

    // --- step list ---
    ui->pushButton_change_step->setEnabled(false);
    ui->pushButton_delete_step->setEnabled(false);

    connect(ui->listWidget_step, &QListWidget::currentRowChanged,
            this, [=](int v) {
                ui->pushButton_change_step->setEnabled(v >= 0);
                ui->pushButton_delete_step->setEnabled(v >= 0 && ui->listWidget_step->count() > 1);
            });

    connect(ui->pushButton_add_step, &QPushButton::clicked,
            this, [=]() {
                ui->listWidget_step->addItem("3.0");
                ui->listWidget_step->setCurrentRow(ui->listWidget_step->count() - 1);
            });

    connect(ui->pushButton_delete_step, &QPushButton::clicked,
            this, [=]() {
                delete ui->listWidget_step->currentItem();
                ui->pushButton_delete_step->setEnabled(ui->listWidget_step->count() > 1);
            });

    connect(ui->pushButton_change_step, &QPushButton::clicked,
            this, [=]() {
                auto* it = ui->listWidget_step->currentItem();
                if (!it) return;

                bool ok;
                double d = QInputDialog::getDouble(
                    this,
                    tr("Ввод числа"),
                    tr("Значение:"),
                    it->text().toDouble(),
                    0.0, 100.0, 1, &ok
                    );

                if (ok)
                    it->setText(QString::number(d, 'f', 1));
            });
}

OtherTestSettings::~OtherTestSettings()
{
    delete ui;
}

void OtherTestSettings::applyValveInfo(const ValveInfo& info)
{
    if (info.safePosition != 0)
        reverseSequence();
}

void OtherTestSettings::applyPattern(SelectTests::PatternType pattern)
{
    Q_UNUSED(pattern);
}

QVector<qreal>& OtherTestSettings::sequence()
{
    return m_sequence;
}

QListWidget* OtherTestSettings::sequenceListWidget()
{
    return ui->listWidget_value;
}

OtherTestSettings::TestParameters OtherTestSettings::getParameters()
{
    TestParameters testParameters;

    testParameters.delay = ui->timeEdit->time().msecsSinceStartOfDay();

    for (int i = 0; i < ui->listWidget_value->count(); i++) {
        testParameters.points.append(ui->listWidget_value->item(i)->text().toDouble());
    }

    for (int i = 0; i < ui->listWidget_step->count(); i++) {
        testParameters.steps.append(ui->listWidget_step->item(i)->text().toDouble());
    }

    return testParameters;
}
