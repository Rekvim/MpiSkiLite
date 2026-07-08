#include "DeviceInitializer.h"
#include "Domain/Measurement/Sensor.h"
#include <QtMath>

// Подключение и инициализация устройства
bool DeviceInitializer::connectAndInitDevice()
{
    bool ok = m_device.isConnect();

    m_telemetry.init.deviceStatusText =
        ok ? QString("Успешное подключение к порту %1").arg(m_device.portName())
           : "Ошибка подключения";
    m_telemetry.init.deviceStatusColor = ok ? Qt::darkGreen : Qt::red;

    if (!ok) return false;

    ok = m_device.initialize();

    m_telemetry.init.initStatusText = ok ? "Успешная инициализация" : "Ошибка инициализации";
    m_telemetry.init.initStatusColor = ok ? Qt::darkGreen : Qt::red;

    return ok;
}

// Обнаружение и отчёт по датчикам
bool DeviceInitializer::detectSensors()
{
    int cnt = m_device.sensorCount();

    if (cnt == 0) {
        m_telemetry.init.connectedSensorsText = "Датчики не обнаружены";
        m_telemetry.init.connectedSensorsColor = Qt::red;
    }
    else if (cnt == 1) {
        m_telemetry.init.connectedSensorsText = "Обнаружен 1 датчик";
        m_telemetry.init.connectedSensorsColor = Qt::darkYellow;
    }
    else {
        m_telemetry.init.connectedSensorsText =
            QString("Обнаружено %1 датчика").arg(cnt);

        m_telemetry.init.connectedSensorsColor = Qt::darkGreen;
    }

    m_telemetry.init.startingPositionText = "Измерение";
    m_telemetry.init.startingPositionColor = Qt::darkYellow;

    return cnt > 0;
}

void DeviceInitializer::measureStartPosition()
{
    if (m_config.normalClosed)
        m_device[0]->captureMin();
    else
        m_device[0]->captureMax();

    const qreal value = m_device[0]->value();

    m_telemetry.init.startingPositionText = QString("%1 мм").arg(value, 0, 'f', 2);
    m_telemetry.init.startingPositionColor = Qt::darkGreen;

    m_telemetry.init.finalPositionText = "Измерение";
    m_telemetry.init.finalPositionColor = Qt::darkYellow;
}

void DeviceInitializer::measureEndPosition()
{
    if (m_config.normalClosed)
        m_device[0]->captureMax();
    else
        m_device[0]->captureMin();

    const qreal value = m_device[0]->value();

    m_telemetry.init.finalPositionText = QString("%1 мм").arg(value, 0, 'f', 2);

    m_telemetry.init.finalPositionColor = Qt::darkGreen;
}

void DeviceInitializer::measureStartPositionShutoff(
    QVector<bool>& initialStates,
    const QVector<bool>& savedStates)
{
    m_telemetry.init.startingPositionText = "Измерение";
    m_telemetry.init.startingPositionColor = Qt::darkYellow;

    for (int i = 0; i < savedStates.size(); ++i) {
        if (savedStates[i]) {
            initialStates[i] = false;
            m_device.setDiscreteOutput(i, false);
        }
    }

    if (m_config.normalClosed)
        m_device[0]->captureMin();
    else
        m_device[0]->captureMax();

    const qreal value = m_device[0]->value();

    m_telemetry.init.startingPositionText = QString("%1 мм").arg(value, 0, 'f', 2);

    m_telemetry.init.startingPositionColor = Qt::darkGreen;
}

void DeviceInitializer::measureEndPositionShutoff(
    QVector<bool>& initialStates,
    const QVector<bool>& savedStates)
{
    m_telemetry.init.finalPositionText = "Измерение";
    m_telemetry.init.finalPositionColor = Qt::darkYellow;

    for (int i = 0; i < savedStates.size(); ++i) {
        if (savedStates[i]) {
            initialStates[i] = true;
            m_device.setDiscreteOutput(i, true);
        }
    }

    if (m_config.normalClosed)
        m_device[0]->captureMax();
    else
        m_device[0]->captureMin();

    const qreal value = m_device[0]->value();

    m_telemetry.init.finalPositionText = QString("%1 мм").arg(value, 0, 'f', 2);
    m_telemetry.init.finalPositionColor = Qt::darkGreen;
}

void DeviceInitializer::calculateCoefficients()
{
    qreal coeff = 1.0;

    if (m_config.strokeMovement == StrokeMovement::Rotary &&
        !qFuzzyIsNull(m_config.diameterPulley)) {
        coeff = qRadiansToDegrees(2.0 / m_config.diameterPulley);
    }

    m_device[0]->correctCoefficients(coeff);
}

void DeviceInitializer::recordStrokeRange()
{
    const qreal value = m_device[0]->range();

    m_telemetry.valveStrokeRecord.range = QString("%1").arg(value, 0, 'f', 2);

    m_telemetry.valveStrokeRecord.real = value;
}
