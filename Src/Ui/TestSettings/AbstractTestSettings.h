#pragma once
#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QTimeEdit>
#include <QInputDialog>

struct ValveInfo;

#include "./Src/Ui/Setup/SelectTests.h"

class AbstractTestSettings : public QDialog
{
    Q_OBJECT
public:
    explicit AbstractTestSettings(QWidget* parent = nullptr)
        : QDialog(parent) {}

    virtual ~AbstractTestSettings() = default;

    virtual void applyValveInfo(const ValveInfo&) {}
    virtual void applyPattern(SelectTests::PatternType) {}

protected:
    virtual QVector<qreal>& sequence() = 0;
    virtual QListWidget* sequenceListWidget() = 0;

    void reverseSequence()
    {
        auto& seq = sequence();
        std::reverse(seq.begin(), seq.end());
        fillNumericList(sequenceListWidget(), seq);
    }

    static void clampTime(QTimeEdit* te, QTime min, QTime max)
    {
        QObject::connect(te, &QTimeEdit::timeChanged, te,
                         [=](QTime t) {
                             if (t < min) te->setTime(min);
                             if (t > max) te->setTime(max);
                         });
    }

    void bindNumericListEditor(
        QListWidget* list,
        QPushButton* addBtn,
        QPushButton* editBtn,
        QPushButton* delBtn,
        const QString& defaultValue
        )
    {
        editBtn->setEnabled(false);
        delBtn->setEnabled(false);

        connect(list, &QListWidget::currentRowChanged, this,
                [=](int row) {
                    editBtn->setEnabled(row >= 0);
                    delBtn->setEnabled(row >= 0 && list->count() > 1);
                });

        connect(addBtn, &QPushButton::clicked, this,
                [=]() {
                    list->addItem(defaultValue);
                    list->setCurrentRow(list->count() - 1);
                });

        connect(delBtn, &QPushButton::clicked, this,
                [=]() {
                    delete list->currentItem();
                    delBtn->setEnabled(list->count() > 1);
                });

        connect(editBtn, &QPushButton::clicked, this,
                [=]() {
                    auto* it = list->currentItem();
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

    static QVector<qreal> readNumericList(QListWidget* list)
    {
        QVector<qreal> out;
        for (int i = 0; i < list->count(); ++i)
            out.append(list->item(i)->text().toDouble());
        return out;
    }

    static void fillNumericList(QListWidget* list, const QVector<qreal>& seq)
    {
        list->clear();
        for (qreal v : seq)
            list->addItem(QString::number(v, 'f', 2));
    }
};
