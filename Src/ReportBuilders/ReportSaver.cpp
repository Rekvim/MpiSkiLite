#include "ReportSaver.h"

#include "./Src/CustomChart/MyChart.h"
#include "Registry.h"

#include <QCoreApplication>
#include <QDate>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QOpenGLWidget>
#include <QPainter>
#include <QDataStream>
#include <QDebug>
#include <QMap>

#include "xlsxdocument.h"
#include "xlsxdatavalidation.h"

using namespace QXlsx;

ReportSaver::ReportSaver(QObject *parent)
    : QObject(parent)
{
}

void ReportSaver::setRegistry(Registry *registry)
{
    m_registry = registry;
}

const QDir &ReportSaver::directory() const
{
    return m_dir;
}

void ReportSaver::saveImage(MyChart *chart)
{
    if (!chart)
        return;

    if (!m_isDirectoryCreated)
        createDir();

    static QMap<QString, quint16> chartCounter;

    const QString name = chart->getname();
    const quint16 index = ++chartCounter[name];

    QPixmap pixmap = chart->grab();

    if (auto *glWidget = chart->findChild<QOpenGLWidget *>()) {
        QPainter painter(&pixmap);
        const QPoint delta =
            glWidget->mapToGlobal(QPoint()) - chart->mapToGlobal(QPoint());
        painter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
        painter.drawImage(delta, glWidget->grabFramebuffer());
    }

    const QString baseName = QStringLiteral("%1_%2").arg(name).arg(index);

    const QString bmpPath = m_dir.filePath(baseName + QStringLiteral(".bmp"));
    pixmap.save(bmpPath);

    QFile out(m_dir.filePath(baseName + QStringLiteral(".data")));
    if (out.open(QIODevice::WriteOnly)) {
        QDataStream stream(&out);
        stream.setVersion(QDataStream::Qt_6_2);
        chart->saveToStream(stream);
        out.flush();
        out.close();
    }
}

bool ReportSaver::saveReport(const Report &report, const QString &templatePath)
{
    if (!m_isDirectoryCreated)
        createDir();

    Document xlsx(templatePath);

    for (const auto &item : report.data) {
        if (!xlsx.selectSheet(item.sheet)) {
            qWarning() << "Не найден лист" << item.sheet << "для записи данных!";
            continue;
        }

        if (item.row <= 0 || item.col <= 0) {
            qWarning() << "Невалидные координаты ячейки" << item.row << item.col;
            continue;
        }

        xlsx.write(item.row, item.col, item.value);
    }

    constexpr int targetWidth = 885;
    constexpr int targetHeight = 460;

    for (const auto &img : report.images) {
        if (img.image.isNull())
            continue;

        if (!xlsx.selectSheet(img.sheet)) {
            qWarning() << "Не найден лист" << img.sheet << "для вставки изображения!";
            continue;
        }

        if (img.row <= 0 || img.col <= 0) {
            qWarning() << "Невалидные координаты изображения" << img.row << img.col;
            continue;
        }

        const QImage scaled =
            img.image.scaled(targetWidth,
                             targetHeight,
                             Qt::IgnoreAspectRatio,
                             Qt::SmoothTransformation);

        xlsx.insertImage(img.row, img.col, scaled);
    }

    // 3. Валидации
    for (const auto &v : report.validation) {
        DataValidation validation(DataValidation::List,
                                  DataValidation::Equal,
                                  v.formula);
        validation.addRange(v.range);
        xlsx.addDataValidation(validation);
    }

    const QString reportPath = m_dir.filePath(QStringLiteral("report.xlsx"));
    xlsx.saveAs(reportPath);

    return QFile::exists(reportPath);
}

void ReportSaver::createDir()
{
    if (!m_registry) {
        qWarning() << "ReportSaver::createDir(): registry is null";
        return;
    }

    auto& objectInfo = m_registry->objectInfo();
    auto& valveInfo = m_registry->valveInfo();

    const QString basePath =
        QStringLiteral("%1/%2/%3/%4")
            .arg(objectInfo.object,
                 objectInfo.manufactory,
                 objectInfo.department,
                 valveInfo.positionNumber);

    const QString dateFolder =
        QDate::currentDate().toString(QStringLiteral("dd_MM_yyyy"));

    m_dir.setPath(QCoreApplication::applicationDirPath());

    if (m_dir.mkpath(basePath) && m_dir.cd(basePath)) {

        if (m_dir.exists(dateFolder)) {
            for (int i = 2; i < 50 && !m_isDirectoryCreated; ++i) {
                const QString folder = QStringLiteral("%1_%2")
                .arg(dateFolder)
                    .arg(i);
                if (m_dir.mkdir(folder)) {
                    m_isDirectoryCreated = m_dir.cd(folder);
                }
            }
        } else if (m_dir.mkdir(dateFolder)) {
            m_isDirectoryCreated = m_dir.cd(dateFolder);
        }
    }

    while (!m_isDirectoryCreated) {
        QString folderPath;

        do {
            emit getDirectory(m_dir.path(), folderPath);
        } while (folderPath.isEmpty());

        QDir chosenDir(folderPath);

        if (!chosenDir.exists()) {
            if (!QDir().mkpath(folderPath)) {
                continue;
            }
        }

        if (!chosenDir.isEmpty()) {
            bool answer = false;
            emit question(
                QStringLiteral("Внимание!"),
                QStringLiteral("Папка не пуста. Вы действительно хотите выбрать эту папку?"),
                answer);

            if (!answer) {
                continue;
            }
        }

        m_dir = chosenDir;
        m_isDirectoryCreated = true;
    }
}
