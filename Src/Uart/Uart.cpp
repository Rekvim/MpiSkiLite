#include "Uart.h"
#include <QElapsedTimer>
#include <QDebug>

static constexpr quint8 START = 0xAA;

Uart::Uart(QObject *parent)
    : QObject{parent}
{
    m_serialPort = new QSerialPort(this);

    m_serialPort->setBaudRate(QSerialPort::Baud115200);
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setParity(QSerialPort::NoParity);
    m_serialPort->setStopBits(QSerialPort::OneStop);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

    m_serialPort->setReadBufferSize(0);

    connect(m_serialPort, &QSerialPort::errorOccurred,
            this, [this](QSerialPort::SerialPortError e) {
                if (e == QSerialPort::NoError) return;

                qWarning() << "[UART] error:" << e << m_serialPort->errorString();

                if (e == QSerialPort::ResourceError ||
                    e == QSerialPort::DeviceNotFoundError ||
                    e == QSerialPort::PermissionError ||
                    e == QSerialPort::WriteError ||
                    e == QSerialPort::ReadError) {
                    close();
                }

                emit errorOccurred(e);
            });
}

Uart::~Uart()
{
    close();
}

void Uart::open(const QString &portName)
{
    if (m_serialPort->isOpen()) {
        if (m_serialPort->portName() == portName)
            return;
        close();
    }

    m_serialPort->setPortName(portName);

    if (!m_serialPort->open(QSerialPort::ReadWrite)) {
        qWarning() << "[UART] open failed:" << portName << m_serialPort->errorString();
        emit errorOccurred(m_serialPort->error());
        return;
    }

    m_serialPort->clear(QSerialPort::AllDirections);

    emit portOpened(portName);
}

void Uart::close()
{
    if (!m_serialPort->isOpen())
        return;

    m_serialPort->clear(QSerialPort::AllDirections);
    m_serialPort->close();
    emit portClosed();
}

static bool readOneFrame(QSerialPort* sp, QByteArray& out, int totalTimeoutMs)
{
    out.clear();
    QByteArray buf;
    buf.reserve(64);

    QElapsedTimer t;
    t.start();

    while (t.elapsed() < totalTimeoutMs) {
        if (!sp->waitForReadyRead(50))
            continue;

        buf += sp->readAll();

        int s = buf.indexOf(char(START));
        if (s < 0) {
            if (buf.size() > 2) buf = buf.right(2);
            continue;
        }
        if (s > 0) buf.remove(0, s);

        if (buf.size() < 2) continue;

        const quint8 len = quint8(buf[1]);
        const int frameSize = int(len) + 4;

        if (buf.size() < frameSize) continue;

        out = buf.left(frameSize);
        return true;
    }

    return false;
}

void Uart::writeAndRead(const QByteArray &dataToWrite, QByteArray &readData)
{
    readData.clear();

    if (!m_serialPort->isOpen()) {
        qWarning() << "[UART] writeAndRead while port is closed";
        return;
    }

    m_serialPort->clear(QSerialPort::Input);

    const qint64 written = m_serialPort->write(dataToWrite);
    if (written != dataToWrite.size()) {
        qWarning() << "[UART] write() failed/partial:"
                   << written << "/" << dataToWrite.size()
                   << m_serialPort->errorString();
        return;
    }

    if (!m_serialPort->waitForBytesWritten(100)) {
        qWarning() << "[UART] waitForBytesWritten timeout:" << m_serialPort->errorString();
        return;
    }

    if (!readOneFrame(m_serialPort, readData, 500)) {
        return;
    }
}
