#include "CyclicTestSettings.h"
#include "ui_CyclicTestSettings.h"
#include "./Src/ValidatorFactory/RegexPatterns.h"
#include "Registry.h"
#include <QMessageBox>

void CyclicTestSettings::setPattern(SelectTests::PatternType pattern)
{
    ui->comboBox_testSelection->clear();
    m_pattern = pattern;

    auto add = [&](QString label, TestParameters::Type t){
        ui->comboBox_testSelection->addItem(label, static_cast<int>(t));
    };

    switch (m_pattern) {
    case SelectTests::Pattern_B_CVT:
    case SelectTests::Pattern_C_CVT:
        add(QStringLiteral("Регулирующий"), TestParameters::Regulatory);
        break;
    case SelectTests::Pattern_C_SOVT:
        add(QStringLiteral("Отсечной"), TestParameters::Shutoff);
        break;
    case SelectTests::Pattern_B_SACVT:
    case SelectTests::Pattern_C_SACVT:
        add(QStringLiteral("Запорно-регулирующий"), TestParameters::Combined);
        add(QStringLiteral("Регулирующий"), TestParameters::Regulatory);
        add(QStringLiteral("Отсечной"), TestParameters::Shutoff);
        break;
    default: break;
    }

    ui->comboBox_testSelection->setCurrentIndex(0);
    onTestSelectionChanged();
}

void CyclicTestSettings::onTestSelectionChanged()
{
    const int idx = ui->comboBox_testSelection->currentIndex();
    if (idx < 0) return;

    const auto t = static_cast<TestParameters::Type>(
        ui->comboBox_testSelection->itemData(idx).toInt()
        );
    m_parameters.testType = t;

    const bool showReg = (t == TestParameters::Regulatory || t == TestParameters::Combined);
    const bool showOff = (t == TestParameters::Shutoff || t == TestParameters::Combined);

    ui->widget_retentionTimeRegulatory->setVisible(showReg);
    ui->widget_shutOff->setVisible(showOff);
}

CyclicTestSettings::CyclicTestSettings(QWidget *parent)
    : AbstractTestSettings(parent)
    , ui(new Ui::CyclicTestSettings)
{
    ui->setupUi(this);

    fillDefaultRegulatoryPresets();
    fillDefaultShutOffPresets();

    bindRegulatoryPresetEditor();

    connect(ui->pushButton_cancel, &QPushButton::clicked,
            this, &QDialog::reject);

    connect(ui->comboBox_testSelection,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CyclicTestSettings::onTestSelectionChanged);

    clampTime(ui->timeEdit_retentionTimeRegulatory, kMinHold, kMaxHold);
    clampTime(ui->timeEdit_retentionTimeShutOff, kMinHold, kMaxHold);

    onTestSelectionChanged();
}

void CyclicTestSettings::fillDefaultRegulatoryPresets()
{
    static const QStringList presetsRange = {
        "0-25-50-75-100-75-50-25-0",
        "0-50-100-50-0",
        "0-100-0"
    };

    ui->listWidget_testRangeRegulatory->clear();
    ui->listWidget_testRangeRegulatory->addItems(presetsRange);
    ui->listWidget_testRangeRegulatory->setCurrentRow(0);

    static const QStringList presetsdelay = {
        "10",
        "30",
        "60"
    };

    ui->listWidget_delayTimeRegulatory->clear();
    ui->listWidget_delayTimeRegulatory->addItems(presetsRange);
    ui->listWidget_delayTimeRegulatory->setCurrentRow(0);

    ui->lineEdit_numberCyclesRegulatory->setText("10");
}

void CyclicTestSettings::fillDefaultShutOffPresets()
{
    static const QStringList presetsRange = {
        "0-100-0"
    };

    ui->listWidget_delayTimeShutOff->clear();
    ui->listWidget_delayTimeShutOff->addItems(presetsRange);
    ui->listWidget_delayTimeShutOff->setCurrentRow(0);

    static const QStringList presetsdelay = {
        "10",
        "30",
        "60"
    };

    ui->listWidget_delayTimeShutOff->clear();
    ui->listWidget_delayTimeShutOff->addItems(presetsRange);
    ui->listWidget_delayTimeShutOff->setCurrentRow(0);

    ui->lineEdit_numberCyclesShutOff->setText("10");
}

void CyclicTestSettings::bindRegulatoryPresetEditor()
{
    ui->pushButton_editRangeRegulatory->setEnabled(false);
    ui->pushButton_removeRangeRegulatory->setEnabled(false);

    connect(ui->listWidget_testRangeRegulatory,
            &QListWidget::currentRowChanged,
            this, [=](int row) {
                ui->pushButton_editRangeRegulatory->setEnabled(row >= 0);
                ui->pushButton_removeRangeRegulatory->setEnabled(
                    row >= 0 &&
                    ui->listWidget_testRangeRegulatory->count() > 1);
            });
}

QString CyclicTestSettings::reverseSequenceString(const QString& s)
{
    QStringList parts = s.split('-', Qt::SkipEmptyParts);
    std::reverse(parts.begin(), parts.end());
    return parts.join('-');
}

void CyclicTestSettings::applyValveInfo(const ValveInfo& info)
{
    if (info.safePosition == 0)
        return;

    for (int i = 0; i < ui->listWidget_testRangeRegulatory->count(); ++i) {
        auto* it = ui->listWidget_testRangeRegulatory->item(i);
        it->setText(reverseSequenceString(it->text()));
    }
}

void CyclicTestSettings::applyPattern(SelectTests::PatternType pattern)
{
    setPattern(pattern);
}

static bool parseSequence(const QString& src,
                          QVector<qreal>& dst,
                          QString& error)
{
    const QString s = src.trimmed();

    if (!RegexPatterns::floatSequence().match(s).hasMatch()) {
        error = QStringLiteral(
            "Формат: X или X-Y, где X и Y — числа (можно с точкой)."
            );
        return false;
    }

    dst.clear();

    const QStringList parts = s.split(
        QRegularExpression("\\s*-\\s*"),
        Qt::SkipEmptyParts
        );

    for (const QString& part : parts) {
        bool ok = false;
        qreal v = part.toDouble(&ok);
        if (!ok) {
            error = QStringLiteral("Не удалось преобразовать число.");
            return false;
        }

        if (v < 0.0 || v > 100.0) {
            error = QString(QStringLiteral(
                                "Значение «%1» вне диапазона 0–100%."
                                )).arg(part);
            return false;
        }

        dst.push_back(v);
    }

    return true;
}

// --- Регулирующий
void CyclicTestSettings::on_pushButton_addRangeRegulatory_clicked()
{
    bool ok;
    QString text = QInputDialog::getText(this, QStringLiteral("Добавить последовательность"),
                                         QStringLiteral("Введите значения (через «-»):"),
                                         QLineEdit::Normal, "", &ok);
    if (ok && !text.trimmed().isEmpty()) {
        ui->listWidget_testRangeRegulatory->addItem(text.trimmed());
        ui->listWidget_testRangeRegulatory->setCurrentRow(ui->listWidget_testRangeRegulatory->count() - 1);
    }
}

void CyclicTestSettings::on_pushButton_editRangeRegulatory_clicked()
{
    if (auto *item = ui->listWidget_testRangeRegulatory->currentItem()) {
        bool ok;
        QString txt = QInputDialog::getText(this, QStringLiteral("Изменить последовательность"),
                                            QStringLiteral("Новое значение:"), QLineEdit::Normal,
                                            item->text(), &ok);
        if (ok && !txt.trimmed().isEmpty()) {
            item->setText(txt.trimmed());
        }
    }
}


void CyclicTestSettings::on_pushButton_removeRangeRegulatory_clicked()
{
    delete ui->listWidget_testRangeRegulatory->currentItem();
}

void CyclicTestSettings::on_pushButton_addDelayRegulatory_clicked()
{
    bool ok;
    int v = QInputDialog::getInt(this, QStringLiteral("Добавить задержку"),
                                 QStringLiteral("Введите значение в секундах:"), 10, 1, 3600, 1, &ok);
    if (ok) {
        ui->listWidget_delayTimeRegulatory->addItem(QString::number(v));
        ui->listWidget_delayTimeRegulatory->setCurrentRow(ui->listWidget_delayTimeRegulatory->count() - 1);
    }
}


void CyclicTestSettings::on_pushButton_editDelayRegulatory_clicked()
{
    if (auto *item = ui->listWidget_delayTimeRegulatory->currentItem()) {
        bool ok;
        int v = QInputDialog::getInt(this, QStringLiteral("Изменить задержку"),
                                     QStringLiteral("Новое значение (сек):"),
                                     item->text().toInt(), 1, 3600, 1, &ok);
        if (ok) {
            item->setText(QString::number(v));
        }
    }
}


void CyclicTestSettings::on_pushButton_removeDelayRegulatory_clicked()
{
    delete ui->listWidget_delayTimeRegulatory->currentItem();

}

// --- Отсечной
void CyclicTestSettings::on_pushButton_addDelayShutOff_clicked()
{
    bool ok;
    int v = QInputDialog::getInt(this, QStringLiteral("Добавить задержку (Отсечной)"),
                                 QStringLiteral("Введите значение в секундах:"), 10, 1, 3600, 1, &ok);
    if (ok) {
        ui->listWidget_delayTimeShutOff->addItem(QString::number(v));
        ui->listWidget_delayTimeShutOff->setCurrentRow(ui->listWidget_delayTimeShutOff->count() - 1);
    }
}


void CyclicTestSettings::on_pushButton_editDelayShutOff_clicked()
{
    if (auto *item = ui->listWidget_delayTimeShutOff->currentItem()) {
        bool ok;
        int v = QInputDialog::getInt(this, QStringLiteral("Изменить задержку (Отсечной)"),
                                     QStringLiteral("Новое значение (сек):"),
                                     item->text().toInt(), 1, 3600, 1, &ok);
        if (ok) {
            item->setText(QString::number(v));
        }
    }
}


void CyclicTestSettings::on_pushButton_removeDelayShutOff_clicked()
{
    delete ui->listWidget_delayTimeShutOff->currentItem();
}

void CyclicTestSettings::on_pushButton_start_clicked()
{
    using TP = TestParameters;

    // --- Regulatory или Combined
    if (m_parameters.testType == TP::Regulatory || m_parameters.testType == TP::Combined) {
        // последовательность
        if (!ui->listWidget_testRangeRegulatory->currentItem()) {
            QMessageBox::warning(this, QStringLiteral("Ошибка"),
                                 QStringLiteral("Выберите хотя бы одну последовательность (регулирующий)."));
            return;
        }
        QString seqReg = ui->listWidget_testRangeRegulatory->currentItem()->text().trimmed();
        QString err;
        if (!parseSequence(seqReg, m_parameters.regSeqValues, err)) {
            QMessageBox::warning(this, QStringLiteral("Ошибка"), QStringLiteral("Регулирующая последовательность: ") + err);
            return;
        }

        // задержка
        if (!ui->listWidget_delayTimeRegulatory->currentItem()) {
            QMessageBox::warning(this, QStringLiteral("Ошибка"),
                                 QStringLiteral("Выберите время задержки (регулирующий)."));
            return;
        }
        m_parameters.regulatory_delayMs =
            ui->listWidget_delayTimeRegulatory->currentItem()->text().toInt() * 1000;

        // удержание

        m_parameters.regulatory_holdMs = ui->timeEdit_retentionTimeRegulatory->time().msecsSinceStartOfDay();

        // число циклов
        {
            QString txt = ui->lineEdit_numberCyclesRegulatory->text().trimmed();
            auto m3 = RegexPatterns::digits().match(txt);
            if (!m3.hasMatch() || txt.isEmpty() || txt.toInt() <= 0) {
                QMessageBox::warning(this, QStringLiteral("Ошибка"),
                                     QStringLiteral("Число циклов (регулирующий): введите положительное целое."));
                return;
            }
            m_parameters.regulatory_numCycles = txt.toInt();
        }
        m_parameters.regulatory_enable_20mA = ui->checkBox_20mA_enable->isChecked();
    }
    else {
        m_parameters.regSeqValues.clear();
        m_parameters.regulatory_delayMs = 0;
        m_parameters.regulatory_holdMs = 0;
        m_parameters.regulatory_numCycles = 0;
        m_parameters.regulatory_enable_20mA = false;
    }

    // --- Shutoff или Combined
    if (m_parameters.testType == TP::Shutoff || m_parameters.testType == TP::Combined) {
        // последовательность
        if (!ui->listWidget_testRangeShutOff->currentItem()) {
            QMessageBox::warning(this, QStringLiteral("Ошибка"),
                                 QStringLiteral("Выберите хотя бы одну последовательность (отсечной)."));
            return;
        }

        m_parameters.offSeqValues = {0.0, 100.0, 0.0};

        // задержка
        if (!ui->listWidget_delayTimeShutOff->currentItem()) {
            QMessageBox::warning(this, QStringLiteral("Ошибка"),
                                 QStringLiteral("Выберите время задержки (отсечной)."));
            return;
        }
        m_parameters.shutoff_delayMs =
            ui->listWidget_delayTimeShutOff->currentItem()->text().toInt() * 1000;

        // удержание
        m_parameters.shutoff_holdMs = ui->timeEdit_retentionTimeShutOff->time().msecsSinceStartOfDay();

        // число циклов
        {
            QString txt = ui->lineEdit_numberCyclesShutOff->text().trimmed();
            auto m6 = RegexPatterns::digits().match(txt);
            if (!m6.hasMatch() || txt.isEmpty() || txt.toInt() <= 0) {
                QMessageBox::warning(this, QStringLiteral("Ошибка"),
                                     QStringLiteral("Число циклов (отсечной): введите положительное целое."));
                return;
            }
            m_parameters.shutoff_numCycles = txt.toInt();
        }

        m_parameters.shutoff_DO[0] = ui->pushButton_DO0_ShutOff->isChecked();
        m_parameters.shutoff_DO[1] = ui->pushButton_DO1_ShutOff->isChecked();
        m_parameters.shutoff_DO[2] = ui->pushButton_DO2_ShutOff->isChecked();
        m_parameters.shutoff_DO[3] = ui->pushButton_DO3_ShutOff->isChecked();

        m_parameters.shutoff_DI[0] = ui->checkBox_switch_3_0_ShutOff->isChecked();
        m_parameters.shutoff_DI[1] = ui->checkBox_switch_0_3_ShutOff->isChecked();
    }
    else {
        // когда не Shutoff/Combined:
        m_parameters.offSeqValues.clear();
        m_parameters.shutoff_delayMs = 0;
        m_parameters.shutoff_holdMs = 0;
        m_parameters.shutoff_numCycles = 0;
        m_parameters.shutoff_DO.fill(false);
    }

    accept();
}

