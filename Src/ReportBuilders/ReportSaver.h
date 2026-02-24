#pragma once

#include <QObject>
#include <QDir>
#include <QImage>

class MyChart;
class Registry;

struct ImageCell {
    QString sheet;
    int row = 0;
    int col = 0;
    QImage image;
};

class ReportSaver : public QObject
{
    Q_OBJECT
public:
    struct ReportData {
        QString sheet;
        quint16 row = 0;
        quint16 col = 0;
        QString value;
    };

    struct ValidationData {
        QString sheet;
        QString formula;
        QString range;
    };

    struct Report {
        QVector<ReportData> data;
        QVector<ValidationData> validation;
        QVector<ImageCell> images;
    };

    explicit ReportSaver(QObject *parent = nullptr);

    void setRegistry(Registry *registry);
    void saveImage(MyChart *chart);

    [[nodiscard]] const QDir &directory() const;
    void createDir();
    bool saveReport(const Report &report, const QString &templatePath);

private:
    QDir m_dir;
    bool m_isDirectoryCreated = false;
    Registry *m_registry = nullptr;

signals:
    void question(const QString &title, const QString &text, bool &result);
    void getDirectory(const QString &currentPath, QString &result);
};
