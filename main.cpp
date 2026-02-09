#include <QApplication>
#include <QTranslator>
#include <QDebug>

#include "./Src/Ui/MainWindow/MainWindow.h"
#include "./Src/Ui/Setup/SelectTests.h"
#include "./Src/Ui/Setup/ObjectWindow.h"
#include "Registry.h"
#include "./Src/Ui/Setup/ValveWindow.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qRegisterMetaType<MainTestSettings::TestParameters>("MainTestSettings::TestParameters");
    qRegisterMetaType<TelemetryStore>("TelemetryStore");

    QTranslator qtTranslator;

    if (qtTranslator.load("qt_ru.qm", ":/translations"))
        a.installTranslator(&qtTranslator);

    Registry registry;

    ObjectWindow objectWindow;
    objectWindow.LoadFromReg(&registry);
    if (objectWindow.exec() == QDialog::Rejected)
        return 0;

    SelectTests selectTests;
    if (selectTests.exec() == QDialog::Rejected)
        return 0;

    auto selectedPattern = selectTests.currentPattern();

    ValveWindow valveWindow;
    valveWindow.setRegistry(&registry);
    valveWindow.setPatternType(selectedPattern);

    if (valveWindow.exec() == QDialog::Rejected)
        return 0;

    MainWindow mainWindow;
    mainWindow.setPatternType(selectedPattern);
    mainWindow.setRegistry(&registry);
    mainWindow.show();
    QTimer::singleShot(0, &mainWindow, [&]{ mainWindow.showMaximized(); });
    return a.exec();
}
