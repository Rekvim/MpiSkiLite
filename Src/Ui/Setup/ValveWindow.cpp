#include "ValveWindow.h"
#include "ui_ValveWindow.h"
#include "./Src/ValidatorFactory/ValidatorFactory.h"
#include <QDebug>
#include <QTimer>
#include <QScreen>
#include <QGuiApplication>
#include <QShortcut>

namespace {

double toDouble(QString s, bool* okOut = nullptr)
{
    s = s.trimmed();
    s.replace(',', '.');

    bool ok = false;
    const double v = QLocale::c().toDouble(s, &ok);

    if (okOut)
        *okOut = ok;

    return v;
}

std::optional<QPair<double,double>> parseRange2(QString s)
{
    s = s.trimmed();

    // нормализуем все тире
    s.replace(QChar(0x2013), '-');
    s.replace(QChar(0x2014), '-');
    s.replace(QChar(0x2212), '-');

    // разделяем по тире
    const QStringList parts = s.split('-', Qt::SkipEmptyParts);
    if (parts.size() != 2)
        return std::nullopt;

    bool ok1 = false, ok2 = false;
    const double a = toDouble(parts[0], &ok1);
    const double b = toDouble(parts[1], &ok2);

    if (!ok1 || !ok2)
        return std::nullopt;

    const double low  = qMin(a, b);
    const double high = qMax(a, b);

    return QPair<double,double>(low, high);
}

} // namespace

ValveWindow::ValveWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ValveWindow)
{
    ui->setupUi(this);

    QTimer::singleShot(0, this, [this]{
        auto *scr = screen(); // экран, где окно
        if (!scr) scr = QGuiApplication::primaryScreen();

        const QRect r = scr->availableGeometry();
        setWindowState(Qt::WindowNoState);
        setGeometry(r);
    });

    ui->tabWidget->setCurrentIndex(0);

    connect(ui->comboBox_manufacturer, &QComboBox::currentTextChanged,
            this, [this](const QString& text) {
                const bool manual = (text == kManualInput);
                ui->lineEdit_manufacturer->setEnabled(manual);
                if (!manual)
                    ui->lineEdit_manufacturer->clear();
    });

    {
        const QString text = ui->comboBox_manufacturer->currentText();
        const bool manual = (text == kManualInput);
        ui->lineEdit_manufacturer->setEnabled(manual);
        if (!manual)
            ui->lineEdit_manufacturer->clear();
    }

    auto bindTab = [&](int key, QWidget* tab) {
        auto* sc = new QShortcut(QKeySequence(QString::number(key)), this);
        sc->setContext(Qt::ApplicationShortcut);
        connect(sc, &QShortcut::activated, this, [=] {
            ui->tabWidget->setCurrentWidget(tab);
        });
    };

    bindTab(1, ui->tab_data);
    bindTab(2, ui->tab_passFail);
    bindTab(3, ui->tab_materials);

    QValidator *validatorDigits = ValidatorFactory::create(ValidatorFactory::Type::Digits, this);
    QValidator *validatorDigitsDot = ValidatorFactory::create(ValidatorFactory::Type::DigitsDot, this);
    QValidator *noSpecialChars = ValidatorFactory::create(ValidatorFactory::Type::NoSpecialChars, this);

    ui->lineEdit_positionNumber->setValidator(noSpecialChars);
    ui->lineEdit_manufacturer->setValidator(noSpecialChars);
    ui->lineEdit_serialNumber->setValidator(noSpecialChars);
    ui->lineEdit_DN->setValidator(validatorDigits);
    ui->lineEdit_PN->setValidator(validatorDigits);
    ui->lineEdit_strokValve->setValidator(validatorDigitsDot);
    ui->lineEdit_driveModel->setValidator(noSpecialChars);
    // ui->lineEdit_driveRange->setValidator(validatorDigitsDot);
    ui->lineEdit_driveDiameter->setValidator(validatorDigitsDot);
    ui->lineEdit_pulleyDiameter->setValidator(validatorDigitsDot);

    connect(ui->lineEdit_pulleyDiameter, &QLineEdit::textChanged,
            this, &ValveWindow::diameterChanged);

    connect(ui->comboBox_strokeMovement, &QComboBox::currentIndexChanged,
            this, &ValveWindow::strokeChanged);

    connect(ui->comboBox_toolNumber, &QComboBox::currentIndexChanged,
            this, &ValveWindow::toolChanged);

    connect(ui->lineEdit_driveDiameter, &QLineEdit::textChanged,
            this, &ValveWindow::diameterChanged);

    connect(ui->comboBox_positionerType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ValveWindow::onPositionerTypeChanged);

    connect(ui->comboBox_materialStuffingBoxSeal, &QComboBox::currentTextChanged,
            this, [this](const QString&) {
                applyFrictionLimitsFromStuffingBoxSeal();
            });

    // Чтобы сразу выставить по текущему значению при открытии окна:
    applyFrictionLimitsFromStuffingBoxSeal();

    onPositionerTypeChanged(ui->comboBox_positionerType->currentIndex());

    ui->lineEdit_pulleyDiameter->setText(m_diameter[0]);
    diameterChanged(m_diameter[0]);
}

void ValveWindow::setPatternType(SelectTests::PatternType pattern)
{
    m_patternType = pattern;
    applyPatternVisibility();
}

void ValveWindow::applyPatternVisibility()
{
    switch (m_patternType) {
    case SelectTests::Pattern_B_CVT:
        ui->widget_positionSensorModel->setVisible(false);
        break;
    case SelectTests::Pattern_C_CVT:
        ui->widget_positionSensorModel->setVisible(false);
        ui->widget_solenoidValveModel->setVisible(false);
        ui->widget_limitSwitchModel->setVisible(false);
        break;
    case SelectTests::Pattern_B_SACVT:
        break;
    case SelectTests::Pattern_C_SACVT:
        break;
    case SelectTests::Pattern_C_SOVT:
        ui->widget_positionerModel->setVisible(false);
        ui->widget_dinamicError_positionerType->setVisible(false);

        break;
    default:
        QMessageBox::warning(this, tr("Ошибка"), tr("Не выбран корректный паттерн!"));
        return;
    }
}

void ValveWindow::onPositionerTypeChanged(quint8 index)
{
    const QString selected = ui->comboBox_positionerType->itemText(index);

    ui->comboBox_dinamicError->clear();

    if (selected == tr("Интеллектуальный ЭПП")) {
        ui->comboBox_dinamicError->addItem(QStringLiteral("1.5"));
        ui->comboBox_dinamicError->setCurrentIndex(0);
    }
    else if (selected == tr("ЭПП") || selected == tr("ПП")) {
        ui->comboBox_dinamicError->addItem(QStringLiteral("2.5"));
        ui->comboBox_dinamicError->setCurrentIndex(0);
    }
}

void ValveWindow::setRegistry(Registry *registry)
{
    m_registry = registry;

    m_registry->loadObjectInfo();

    ui->comboBox_positionNumber->clear();
    ui->comboBox_positionNumber->addItems(m_registry->positions());
    ui->comboBox_positionNumber->addItem(kManualInput);

    const QString last = m_registry->lastPosition();
    if (!last.isEmpty() && m_registry->loadValveInfo(last)) {
        m_local = m_registry->valveInfo();
        loadToUi(m_local);
    }

    if (last == "") {
        ui->comboBox_positionNumber->setCurrentIndex(ui->comboBox_positionNumber->count() - 1);
    } else {
        ui->comboBox_positionNumber->setCurrentIndex(ui->comboBox_positionNumber->findText(last));
        positionChanged(last);
    }

    connect(ui->comboBox_positionNumber, &QComboBox::currentTextChanged,
        this, &ValveWindow::positionChanged);
}

void ValveWindow::applyFrictionLimitsFromStuffingBoxSeal()
{
    const QString seal = ui->comboBox_materialStuffingBoxSeal->currentText().trimmed();

    // Маппинг: материал -> диапазон (%)
    // Если строки в комбобоксе отличаются (например "PTFE (тефлон)"),
    // замени сравнение на contains(...) или приведи варианты.
    double lo = 0.0;
    double hi = 0.0;
    bool known = true;

    if (seal == "PTFE") {
        lo = 1.0;
        hi = 9.0;
    } else if (seal == "Graphite") {
        lo = 8.0;
        hi = 15.0;
    } else {
        known = false;
    }

    if (!known)
        return; // ничего не трогаем, если неизвестное значение

    ui->lineEdit_crossingLimits_coefficientFriction_lowerLimit
        ->setText(QString::number(lo, 'f', 2));

    ui->lineEdit_crossingLimits_coefficientFriction_upperLimit
        ->setText(QString::number(hi, 'f', 2));
}

void ValveWindow::saveValveInfo()
{
    readFromUi(m_local);

    m_registry->valveInfo() = m_local;
    m_registry->saveValveInfo();
}

void ValveWindow::readMaterialsFromUi(MaterialsOfComponentParts& m)
{
    m.saddle = ui->lineEdit_materialSaddle->text();
    m.corpus = ui->lineEdit_materialCorpus->text();
    m.CV = ui->lineEdit_CV->text();
    m.cap = ui->lineEdit_materialCap->text();
    m.ball = ui->lineEdit_materialBall->text();
    m.disk = ui->lineEdit_materialDisk->text();
    m.plunger = ui->lineEdit_materialPlunger->text();
    m.shaft = ui->lineEdit_materialShaft->text();
    m.stock = ui->lineEdit_materialStock->text();
    m.guideSleeve = ui->lineEdit_materialGuideSleeve->text();
    m.stuffingBoxSeal = ui->comboBox_materialStuffingBoxSeal->currentText();

    m_registry->materialsOfComponentParts() = m;
    m_registry->saveMaterialsOfComponentParts();
}

void ValveWindow::loadMaterialsToUi(const MaterialsOfComponentParts& m)
{
    ui->lineEdit_materialSaddle->setText(m.saddle);
    ui->lineEdit_CV->setText(m.CV);
    ui->lineEdit_materialCorpus->setText(m.corpus);
    ui->lineEdit_materialCap->setText(m.cap);
    ui->lineEdit_materialBall->setText(m.ball);
    ui->lineEdit_materialDisk->setText(m.disk);
    ui->lineEdit_materialPlunger->setText(m.plunger);
    ui->lineEdit_materialShaft->setText(m.shaft);
    ui->lineEdit_materialStock->setText(m.stock);
    ui->lineEdit_materialGuideSleeve->setText(m.guideSleeve);

    ui->comboBox_materialStuffingBoxSeal
        ->setCurrentText(m.stuffingBoxSeal);

    {
        QSignalBlocker b(ui->comboBox_materialStuffingBoxSeal);
        ui->comboBox_materialStuffingBoxSeal->setCurrentText(m.stuffingBoxSeal);
    }

    // гарантированно применяем после установки
    applyFrictionLimitsFromStuffingBoxSeal();
}

void ValveWindow::readFromUi(ValveInfo& v)
{
    // --- position ---
    const QString posFromCombo =
        ui->comboBox_positionNumber->currentText().trimmed();

    if (posFromCombo == kManualInput) {
        v.positionNumber =
            ui->lineEdit_positionNumber->text().trimmed();
    } else {
        v.positionNumber = posFromCombo;
    }

    // --- manufacturer ---
    const QString manufFromCombo =
        ui->comboBox_manufacturer->currentText().trimmed();

    if (manufFromCombo == kManualInput) {
        v.manufacturer =
            ui->lineEdit_manufacturer->text().trimmed();
    } else {
        v.manufacturer = manufFromCombo;
    }

    v.valveModel = ui->lineEdit_valveModel->text();
    v.serialNumber = ui->lineEdit_serialNumber->text();

    v.DN = ui->lineEdit_DN->text();
    v.PN = ui->lineEdit_PN->text();

    v.positionerModel = ui->lineEdit_positionerModel->text();

    v.positionerType = m_patternType == SelectTests::Pattern_C_SOVT
                                      ? ui->comboBox_positionerType->currentText()
                                      : "";

    v.dinamicErrorRecomend = ui->comboBox_dinamicError->currentText().toDouble();

    v.solenoidValveModel = ui->lineEdit_solenoidValveModel->text();
    v.limitSwitchModel = ui->lineEdit_limitSwitchModel->text();
    v.positionSensorModel = ui->lineEdit_positionSensorModel->text();

    v.strokeMovement = ui->comboBox_strokeMovement->currentIndex();
    v.strokValve = ui->lineEdit_strokValve->text();
    v.driveModel = ui->lineEdit_driveModel->text();
    v.safePosition = ui->comboBox_safePosition->currentIndex();
    v.driveType = ui->comboBox_driveType->currentIndex();

    auto r = parseRange2(ui->lineEdit_driveRange->text());
    if (r) {
        v.driveRangeLow = r->first;
        v.driveRangeHigh = r->second;
    }

    v.driveDiameter = ui->lineEdit_driveDiameter->text().toDouble();
    v.toolNumber = ui->comboBox_toolNumber->currentIndex();
    v.diameterPulley = ui->lineEdit_pulleyDiameter->text().toDouble();
    v.materialStuffingBoxSeal = ui->comboBox_materialStuffingBoxSeal->currentText();

    v.crossingLimits.frictionCoefLowerLimit =
        ui->lineEdit_crossingLimits_coefficientFriction_lowerLimit->text().toDouble();
    v.crossingLimits.frictionCoefUpperLimit =
        ui->lineEdit_crossingLimits_coefficientFriction_upperLimit->text().toDouble();

    // Линейная характеристика — один нижний порог
    v.crossingLimits.linearCharacteristicLowerLimit =
        ui->lineEdit_crossingLimits_linearCharacteristic_lowerLimit->text().toDouble();

    // Диапазон хода — один верхний порог (берём из твоего range_lowerLimit)
    v.crossingLimits.rangeUpperLimit =
        ui->lineEdit_crossingLimits_range_lowerLimit->text().toDouble();

    // Пружина
    v.crossingLimits.springLowerLimit =
        ui->lineEdit_crossingLimits_spring_lowerLimit->text().toDouble();
    v.crossingLimits.springUpperLimit =
        ui->lineEdit_crossingLimits_spring_upperLimit->text().toDouble();

    v.crossingLimits.frictionEnabled =
        ui->checkBox_crossingLimits_coefficientFriction->isChecked();

    v.crossingLimits.linearCharacteristicEnabled =
        ui->checkBox_crossingLimits_linearCharacteristic->isChecked();

    v.crossingLimits.rangeEnabled =
        ui->checkBox_crossingLimits_range->isChecked();

    v.crossingLimits.springEnabled =
        ui->checkBox_crossingLimits_spring->isChecked();

    v.crossingLimits.dynamicErrorEnabled =
        ui->checkBox_crossingLimits_dinamicError->isChecked();
}

void ValveWindow::loadToUi(const ValveInfo& v)
{
    const bool manual = v.positionNumber.isEmpty();

    ui->lineEdit_positionNumber->setEnabled(manual);
    ui->lineEdit_positionNumber->setText(v.positionNumber);

    const int idx = ui->comboBox_manufacturer->findText(v.manufacturer);

    if (idx >= 0) {
        ui->comboBox_manufacturer->setCurrentIndex(idx);
        ui->lineEdit_manufacturer->setEnabled(false);
        ui->lineEdit_manufacturer->clear();
    } else {
        ui->comboBox_manufacturer->setCurrentText(kManualInput);
        ui->lineEdit_manufacturer->setEnabled(true);
        ui->lineEdit_manufacturer->setText(v.manufacturer);
    }


    ui->lineEdit_valveModel->setText(v.valveModel);
    ui->lineEdit_serialNumber->setText(v.serialNumber);
    ui->lineEdit_DN->setText(v.DN);
    ui->lineEdit_PN->setText(v.PN);
    ui->lineEdit_strokValve->setText(v.strokValve);
    ui->lineEdit_positionerModel->setText(v.positionerModel);

    ui->lineEdit_solenoidValveModel->setText(v.solenoidValveModel);
    ui->lineEdit_limitSwitchModel->setText(v.limitSwitchModel);
    ui->lineEdit_positionSensorModel->setText(v.positionSensorModel);

    ui->lineEdit_driveModel->setText(v.driveModel);
    ui->lineEdit_driveRange->setText(
        QString("%1-%2")
            .arg(v.driveRangeLow,  0, 'f', 2)
            .arg(v.driveRangeHigh, 0, 'f', 2)
        );

    ui->lineEdit_pulleyDiameter->setText(QString::number(v.diameterPulley));

    ui->lineEdit_driveDiameter->setText(QString::number(v.driveDiameter));

    ui->comboBox_safePosition->setCurrentIndex(v.safePosition);
    ui->comboBox_driveType->setCurrentIndex(v.driveType);
    ui->comboBox_strokeMovement->setCurrentIndex(v.strokeMovement);
    ui->comboBox_toolNumber->setCurrentIndex(v.toolNumber);

    ui->checkBox_crossingLimits_coefficientFriction->setChecked(
        v.crossingLimits.frictionEnabled);

    ui->checkBox_crossingLimits_linearCharacteristic->setChecked(
        v.crossingLimits.linearCharacteristicEnabled);

    ui->checkBox_crossingLimits_range->setChecked(
        v.crossingLimits.rangeEnabled);

    ui->checkBox_crossingLimits_spring->setChecked(
        v.crossingLimits.springEnabled);

    ui->checkBox_crossingLimits_dinamicError->setChecked(
        v.crossingLimits.dynamicErrorEnabled);

    ui->lineEdit_crossingLimits_coefficientFriction_lowerLimit->setText(
        QString::number(v.crossingLimits.frictionCoefLowerLimit, 'f', 2));
    ui->lineEdit_crossingLimits_coefficientFriction_upperLimit->setText(
        QString::number(v.crossingLimits.frictionCoefUpperLimit, 'f', 2));

    ui->lineEdit_crossingLimits_linearCharacteristic_lowerLimit->setText(
        QString::number(v.crossingLimits.linearCharacteristicLowerLimit, 'f', 2));

    ui->lineEdit_crossingLimits_range_lowerLimit->setText(
        QString::number(v.crossingLimits.rangeUpperLimit, 'f', 2));

    ui->lineEdit_crossingLimits_spring_lowerLimit->setText(
        QString::number(v.crossingLimits.springLowerLimit, 'f', 2));
    ui->lineEdit_crossingLimits_spring_upperLimit->setText(
        QString::number(v.crossingLimits.springUpperLimit, 'f', 2));
}

void ValveWindow::positionChanged(const QString &position)
{
    if (position == kManualInput) {
        ui->lineEdit_positionNumber->clear();
        ui->lineEdit_positionNumber->setEnabled(true);
        return;
    }

    if (m_registry->loadValveInfo(position)) {
        m_local = m_registry->valveInfo();
        loadToUi(m_local);
    }

    if (m_registry->loadMaterialsOfComponentParts(position)) {
        m_materialsLocal = m_registry->materialsOfComponentParts();
        loadMaterialsToUi(m_materialsLocal);
    }
}

void ValveWindow::strokeChanged(quint16 n)
{
    ui->comboBox_toolNumber->setEnabled(n == 1);
    ui->lineEdit_pulleyDiameter->setEnabled(
        (n == 1)
        && (ui->comboBox_toolNumber->currentIndex() == ui->comboBox_toolNumber->count() - 1));
}

void ValveWindow::toolChanged(quint16 n)
{
    if (ui->comboBox_toolNumber->currentText() == kManualInput) {
        ui->lineEdit_pulleyDiameter->setEnabled(true);
    } else {
        ui->lineEdit_pulleyDiameter->setEnabled(false);
        ui->lineEdit_pulleyDiameter->setText(m_diameter[n]);
    }
}

void ValveWindow::diameterChanged(const QString &text)
{
    double value = text.toDouble();
    ui->label_valueSquare->setText(QString().asprintf("%.2f", M_PI * value * value / 4));
}

void ValveWindow::on_pushButton_netWindow_clicked()
{
    if (ui->lineEdit_positionNumber->text().isEmpty()) {
        QMessageBox::warning(this, 
            tr("Ошибка"),
            tr("Введите номер позиции"));
        return;
    }

    const QString manuf =
        (ui->comboBox_manufacturer->currentText() == kManualInput)
            ? ui->lineEdit_manufacturer->text()
            : ui->comboBox_manufacturer->currentText();

    if (manuf.isEmpty()
        || ui->lineEdit_valveModel->text().isEmpty()
        || ui->lineEdit_serialNumber->text().isEmpty()
        || ui->lineEdit_DN->text().isEmpty()
        || ui->lineEdit_PN->text().isEmpty()
        || ui->lineEdit_strokValve->text().isEmpty()
        || ui->lineEdit_driveRange->text().isEmpty()) {

        QMessageBox::StandardButton button
            = QMessageBox::question(this,
                                    tr("Предупреждение"),
                                    tr("Введены не все данные, вы действительно хотите продолжить?"));

        if (button == QMessageBox::StandardButton::No) {
            return;
        }
    }

    auto& other = m_registry->otherParameters();
    other.safePosition  = ui->comboBox_safePosition->currentText();
    other.strokeMovement = ui->comboBox_strokeMovement->currentText();
    saveValveInfo();
    readMaterialsFromUi(m_materialsLocal);
    accept();
}

void ValveWindow::on_pushButton_clear_clicked()
{
    ui->lineEdit_manufacturer->clear();
    ui->lineEdit_valveModel->clear();
    ui->lineEdit_serialNumber->clear();
    ui->lineEdit_DN->clear();
    ui->lineEdit_PN->clear();
    ui->lineEdit_strokValve->clear();
    ui->lineEdit_positionNumber->clear();
    ui->lineEdit_valveModel->clear();
    ui->lineEdit_driveRange->clear();
    ui->lineEdit_driveDiameter->clear();

    ui->lineEdit_pulleyDiameter->setText(m_diameter[0]);

    ui->comboBox_materialStuffingBoxSeal->setCurrentIndex(0);
    ui->comboBox_dinamicError->setCurrentIndex(0);
    ui->comboBox_safePosition->setCurrentIndex(0);
    ui->comboBox_driveType->setCurrentIndex(0);
    ui->comboBox_strokeMovement->setCurrentIndex(0);
    ui->comboBox_toolNumber->setCurrentIndex(0);
}

