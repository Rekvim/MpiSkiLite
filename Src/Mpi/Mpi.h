#ifndef MPI_H
#define MPI_H

#pragma once
#include <QObject>
#include <QDebug>
#include <QEventLoop>
#include <QThread>

#include "Sensor.h"
#include "./Src/Uart/UartReader.h"

class Mpi : public QObject
{
    Q_OBJECT

public:
    explicit Mpi(QObject* parent = nullptr);
    ~Mpi();

    bool initialize();
    bool isConnect();
    const QString &portName() const;

    quint8 version();

    quint8 digitalOutputs();
    quint8 digitalInputs();

    void setDacRaw(quint16 value);
    void setDacValue(qreal value);

    quint16 dacMin();
    quint16 dacMax();

    quint8 sensorCount() const;

    Sensor* operator[](quint8 n);

    Sensor* dac() const;

    void setDiscreteOutput(quint8 index, bool state);

public slots:
    void onAdcData(const QVector<quint16>& adc);
    void onUartConnected(const QString portName);
    void onUartDisconnected();
    void onUartError(QSerialPort::SerialPortError err);

signals:
    void errorOccured(const QString& message);

    void requestConnect();
    void requestVersion(quint8& version);
    void requestSetDac(quint16 value);

    void requestSetAdcPolling(bool enable, quint16 interval);
    void requestSetAdcChannelMask(quint8 channels);
    void requestSetAdcTimerArr(quint16 timer);
    void requestAdcRead(QVector<quint16>& adc);
    void requestEnableAdc();
    void requestDisableAdc();

    void setDigitalOutput(quint8 index, bool state);
    void requestDigitalOutputs(quint8& DO);
    void requestDigitalInputs(quint8& DI);

private:
    std::array<Sensor*, 6> m_sensorByAdc { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

    UartReader *m_uartReader;
    QThread *m_uartThread;
    bool m_isConnected = false;
    QString m_portName;

    QVector<Sensor *> m_sensors;
    Sensor *m_dac = nullptr;

    quint16 m_dacMin = 65536 * 3 / 24;
    quint16 m_dacMax = 65536 * 21 / 24;

    void sleep(quint16 msecs);
};

#endif // MPI_H
