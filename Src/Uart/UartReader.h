#pragma once
#include <QObject>
#include <QTimer>
#include <QSerialPortInfo>
#include <QDebug>

#include "Uart.h"
#include "UartMessage.h"

class UartReader : public QObject
{
    Q_OBJECT
public:
    explicit UartReader(QObject *parent = nullptr);

private:
    QByteArray sendMessage(const UartMessage& message);

public slots:
    void autoConnect();
    void readVersion(quint8 &version);

    void setDacValue(quint16 value);
    void setAdcChannels(quint8 channels);
    void setAdcTimerInterval(quint16 timer);

    void enableAdc();
    void disableAdc();

    void readAdcValues(QVector<quint16> &adc);

    void setAdcPolling(bool enable, quint16 interval);

    void setDigitalOutput(quint8 outputNumber, bool state);
    void readDigitalOutputs(quint8 &DO);
    void readDigitalInputs(quint8 &DI);

private slots:
    void onPortOpened(const QString &portName);
    void onPortClosed();
    void onPortError(QSerialPort::SerialPortError err);
    void onAdcPollTimer();

    bool isConnected() const { return m_isConnected; }
    QString portName() const { return m_portName; }

signals:
    void openPort(const QString &portName);
    void closePort();
    void writeAndRead(const QByteArray &data_to_write, QByteArray &read_data);

    void adcDataReady(QVector<quint16> adc);
    void portOpened(const QString &portName);
    void portClosed();
    void portError(QSerialPort::SerialPortError error);
    void errorOccured(const QString &message);

private:
    Uart *m_uart;
    QString m_portName;
    bool m_isConnected = false;
    const quint8 m_maxAttempts = 5;
    quint8 m_version;
    QTimer *m_adcPollTimer;
};
