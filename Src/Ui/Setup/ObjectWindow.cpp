#include "ObjectWindow.h"
#include "ui_ObjectWindow.h"

ObjectWindow::ObjectWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ObjectWindow)
{
    ui->setupUi(this);

    QRect scr = QApplication::primaryScreen()->geometry();
    move(scr.center() - rect().center());

    ui->dateEdit->setDate(QDate::currentDate());

    QRegularExpression regular;
    regular.setPattern("[^/?*:;{}\\\\]+");
    QRegularExpressionValidator *validator = new QRegularExpressionValidator(regular, this);

    ui->lineEdit_object->setValidator(validator);
    ui->lineEdit_manufactory->setValidator(validator);
    ui->lineEdit_department->setValidator(validator);

    connect(ui->pushButton, &QPushButton::clicked, this, &ObjectWindow::ButtonClick);
}

void ObjectWindow::LoadFromReg(Registry *registry)
{
    m_registry = registry;

    m_registry->loadObjectInfo();

    const auto& o = m_registry->objectInfo();
    ui->lineEdit_object->setText(o.object);
    ui->lineEdit_manufactory->setText(o.manufactory);
    ui->lineEdit_department->setText(o.department);
    ui->lineEdit_FIO->setText(o.FIO);
}

void ObjectWindow::saveToRegistry()
{
    auto& o = m_registry->objectInfo();
    o.object = ui->lineEdit_object->text();
    o.manufactory = ui->lineEdit_manufactory->text();
    o.department = ui->lineEdit_department->text();
    o.FIO = ui->lineEdit_FIO->text();

    m_registry->saveObjectInfo();
}

void ObjectWindow::ButtonClick()
{
    if (ui->lineEdit_object->text().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите наименование объекта");
        return;
    }
    if (ui->lineEdit_manufactory->text().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите наименование цеха");
        return;
    }
    if (ui->lineEdit_department->text().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите наименование отделение/установки");
        return;
    }

    // Проверки в БД (QSettings)
    if (!m_registry->checkObject(ui->lineEdit_object->text())) {
        auto button = QMessageBox::question(this, "Предупреждение",
                                            "Объекта нет в базе. Вы действительно хотите продождить?");
        if (button == QMessageBox::Yes) {
            saveToRegistry();
            accept();
        }
        return;
    }

    // чтобы checkManufactory/checkDepartment смотрели в правильные группы
    m_registry->objectInfo().object = ui->lineEdit_object->text();

    if (!m_registry->checkManufactory(ui->lineEdit_manufactory->text())) {
        auto button = QMessageBox::question(this, "Предупреждение",
                                            "Цеха нет в базе. Вы действительно хотите продождить?");
        if (button == QMessageBox::Yes) {
            saveToRegistry();
            accept();
        }
        return;
    }

    m_registry->objectInfo().manufactory = ui->lineEdit_manufactory->text();

    if (!m_registry->checkDepartment(ui->lineEdit_department->text())) {
        auto button = QMessageBox::question(this, "Предупреждение",
                                            "Отделения/установки нет в базе. Вы действительно хотите продождить?");
        if (button == QMessageBox::Yes) {
            saveToRegistry();
            accept();
        }
        return;
    }

    auto& other = m_registry->otherParameters();
    other.date = ui->dateEdit->date().toString("dd.MM.yyyy");

    saveToRegistry();
    accept();
}

