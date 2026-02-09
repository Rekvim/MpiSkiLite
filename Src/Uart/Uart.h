#ifndef UART_H
#define UART_H

#pragma once
#include <QByteArray>
#include <QObject>
#include <QSerialPort>

class Uart : public QObject
{
    Q_OBJECT
public:
    explicit Uart(QObject *parent = nullptr);
    ~Uart();

private:
    QSerialPort *m_serialPort;

public slots:
    void open(const QString &portName);
    void close();
    void writeAndRead(const QByteArray &dataToWrite, QByteArray &readData);
signals:
    void portOpened(QString portName);
    void portClosed();
    void errorOccurred(QSerialPort::SerialPortError error);
};

#endif // UART_H
