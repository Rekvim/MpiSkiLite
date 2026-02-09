#ifndef OBJECT_WINDOW_H
#define OBJECT_WINDOW_H

#pragma once
#include <QDialog>
#include <QMessageBox>
#include <QScreen>
#include "Registry.h"

namespace Ui {
class ObjectWindow;
}

class ObjectWindow : public QDialog
{
    Q_OBJECT

public:
    explicit ObjectWindow(QWidget *parent = nullptr);
    void LoadFromReg(Registry *registry);
    ~ObjectWindow() = default;

private:
    Ui::ObjectWindow *ui = nullptr;
    Registry *m_registry = nullptr;

    void saveToRegistry();

private slots:
    void ButtonClick();
};

#endif // OBJECT_WINDOW_H
