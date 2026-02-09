#include "Mpi.h"
#include <utility>
#include "./Src/Mpi/MpiSettings.h"

namespace {
// DAC: 16-bit scale
constexpr quint32 kDacFullScale = 1u << 16;  // 65536 = 2^16 (полная шкала 16-бит)
constexpr quint16 kDacMaxRaw = (1u << 16) - 1;
// ADC word packing: [adcIndex:4 bits][raw:12 bits]
constexpr quint8  kAdcShift = 12; // raw занимает младшие 12 бит, индекс в старших
constexpr quint16 kAdcRawMask = 0x0FFF; // маска для выделения 12-бит raw

// Каналы
constexpr quint8  kAll6Channels = 0x3F; // включить 6 каналов

// Порог "датчик обнаружен"
// constexpr quint16 kAdcDetectThreshold = 0x0050; // эмпирический порог сырого ADC

constexpr qreal kDacRange_mA = 24.0; // диапазон преобразования (0..24 мА)

constexpr quint16 kDefaultAdcPollingMs = 50;
}

Mpi::Mpi(QObject *parent)
    : QObject{parent}
{
    m_dac = new Sensor(this);

    m_uartReader = new UartReader;
    m_uartThread = new QThread(this);

    connect(m_uartThread, &QThread::finished,
            m_uartReader, &QObject::deleteLater);

    m_uartReader->moveToThread(m_uartThread);
    m_uartThread->start();

    connect(this, &Mpi::requestConnect,
            m_uartReader, &UartReader::autoConnect,
            Qt::BlockingQueuedConnection);

    connect(this, &Mpi::requestVersion,
            m_uartReader, &UartReader::readVersion,
            Qt::BlockingQueuedConnection);

    connect(this, &Mpi::requestSetDac,
            m_uartReader, &UartReader::setDacValue,
            Qt::BlockingQueuedConnection);

    connect(this, &Mpi::requestSetAdcChannelMask,
            m_uartReader, &UartReader::setAdcChannels,
            Qt::BlockingQueuedConnection);

    connect(this, &Mpi::requestSetAdcTimerArr,
            m_uartReader, &UartReader::setAdcTimerInterval,
            Qt::BlockingQueuedConnection);

    connect(this, &Mpi::requestEnableAdc,
            m_uartReader, &UartReader::enableAdc,
            Qt::BlockingQueuedConnection);

    connect(this, &Mpi::requestDisableAdc,
            m_uartReader, &UartReader::disableAdc,
            Qt::BlockingQueuedConnection);

    connect(this, &Mpi::requestAdcRead,
            m_uartReader, &UartReader::readAdcValues,
            Qt::BlockingQueuedConnection);

    connect(this, &Mpi::requestSetAdcPolling,
            m_uartReader, &UartReader::setAdcPolling,
            Qt::BlockingQueuedConnection);

    connect(this, &Mpi::setDigitalOutput,
            m_uartReader, &UartReader::setDigitalOutput,
            Qt::BlockingQueuedConnection);

    connect(this, &Mpi::requestDigitalOutputs,
            m_uartReader, &UartReader::readDigitalOutputs,
            Qt::BlockingQueuedConnection);

    connect(this, &Mpi::requestDigitalInputs,
            m_uartReader, &UartReader::readDigitalInputs,
            Qt::BlockingQueuedConnection);

    connect(m_uartReader, &UartReader::adcDataReady,
            this, &Mpi::onAdcData);

    connect(m_uartReader, &UartReader::portOpened,
            this, &Mpi::onUartConnected,
            Qt::QueuedConnection);

    connect(m_uartReader, &UartReader::portClosed,
            this, &Mpi::onUartDisconnected,
            Qt::QueuedConnection);

    connect(m_uartReader, &UartReader::portError,
            this, &Mpi::onUartError,
            Qt::QueuedConnection);

    // connect(m_uartReader, &UartReader::errorOccured,
    //         this, &Mpi::errorOccured,
    //         Qt::DirectConnection);
}

Mpi::~Mpi()
{
    m_uartThread->quit();
    m_uartThread->wait();
}

bool Mpi::isConnect()
{
    emit requestConnect();

    bool ok = false;
    QString pn;

    QMetaObject::invokeMethod(m_uartReader, "isConnected",
                              Qt::BlockingQueuedConnection,
                              Q_RETURN_ARG(bool, ok));

    QMetaObject::invokeMethod(m_uartReader, "portName",
                              Qt::BlockingQueuedConnection,
                              Q_RETURN_ARG(QString, pn));

    m_isConnected = ok;
    if (ok) m_portName = pn;

    return ok;
}

quint8 Mpi::version()
{
    if (!m_isConnected)
        return 0;
    quint8 version = 0;
    emit requestVersion(version);
    return version;
}

quint8 Mpi::digitalOutputs()
{
    if (!m_isConnected)
        return 0;

    quint8 DO = 0;
    emit requestDigitalOutputs(DO);
    return DO;
}

quint8 Mpi::digitalInputs()
{
    if (!m_isConnected)
        return 0;

    quint8 DI = 0;
    emit requestDigitalInputs(DI);
    return DI;
}

bool Mpi::initialize()
{
    emit requestSetAdcPolling(false, kDefaultAdcPollingMs);

    if (!m_isConnected) return false;

    qDeleteAll(m_sensors);
    m_sensors.clear();
    m_sensorByAdc.fill(nullptr);

    const MpiSettings mpiSettings;

    m_dac->setCoefficients(kDacRange_mA / kDacMaxRaw, mpiSettings.GetDac().bias);

    m_dacMin = kDacFullScale * (mpiSettings.GetDac().min - mpiSettings.GetDac().bias) / kDacRange_mA;
    m_dacMax = kDacFullScale * (mpiSettings.GetDac().max - mpiSettings.GetDac().bias) / kDacRange_mA;

    emit requestSetAdcChannelMask(kAll6Channels);
    emit requestEnableAdc();

    QVector<quint16> adc;
    sleep(1000);
    emit requestAdcRead(adc);

    quint8 sensorNum = 0;
    quint8 channelMask = 0;

    for (const auto& a : std::as_const(adc)) {
        if ((a & 0xFFF) > 0x050) {

            quint8 adcNum = a >> kAdcShift;

            channelMask |= quint8(1u << adcNum);

            Sensor *sensor = new Sensor(this);
            m_sensorByAdc[adcNum] = sensor;

            const qreal adcCur = mpiSettings.GetAdc(adcNum);

            const auto sensorSettings = mpiSettings.GetSensor(sensorNum);
            qreal k = ((sensorSettings.max - sensorSettings.min) * adcCur) / (16 * 0xFFF);
            qreal b = (5 * sensorSettings.min - sensorSettings.max) / 4;

            sensor->setCoefficients(k, b);

            sensor->setUnit((sensorNum++ == 0) ? "мм" : "bar");
            m_sensors.push_back(sensor);
        }
    }

    if (sensorNum == 0) {
        emit requestDisableAdc();
        return true;
    }

    emit requestSetAdcChannelMask(channelMask);
    emit requestSetAdcTimerArr(40 / sensorNum);
    emit requestSetAdcPolling(true, kDefaultAdcPollingMs);

    return true;
}

void Mpi::setDacRaw(quint16 value)
{
    if (value < m_dacMin)
        value = m_dacMin;
    if (value > m_dacMax)
        value = m_dacMax;
    m_dac->setValue(value);
    emit requestSetDac(m_dac->rawValue());
}

void Mpi::setDacValue(qreal value)
{
    quint16 newRaw = m_dac->rawFromValue(value);
    quint16 curRaw = m_dac->rawValue();

    if (newRaw != curRaw) {
        m_dac->setValue(newRaw);
        emit requestSetDac(newRaw);
    }
}

quint16 Mpi::dacMin()
{
    return m_dacMin;
}

quint16 Mpi::dacMax()
{
    return m_dacMax;
}

quint8 Mpi::sensorCount() const
{
    return m_sensors.size();
}

const QString &Mpi::portName() const
{
    return m_portName;
}

Sensor *Mpi::operator[](quint8 n)
{
    if (n >= m_sensors.size())
        return nullptr;
    return m_sensors.at(n);
}

Sensor *Mpi::dac() const
{
    return m_dac;
}

void Mpi::setDiscreteOutput(quint8 index, bool state)
{
    if (!m_isConnected)
        return;

    emit setDigitalOutput(index, state);
}

void Mpi::sleep(quint16 msecs)
{
    QEventLoop loop;
    QTimer::singleShot(msecs, &loop, &QEventLoop::quit);
    loop.exec();
}

void Mpi::onAdcData(const QVector<quint16>& adc)
{
    for (quint16 w : adc) {
        const quint8 adcNum = w >> kAdcShift;
        const quint16 raw = w & kAdcRawMask;

        if (adcNum < m_sensorByAdc.size() && m_sensorByAdc[adcNum])
            m_sensorByAdc[adcNum]->setValue(raw);
    }
}

void Mpi::onUartConnected(const QString portName)
{
    m_isConnected = true;
    m_portName = portName;
}

void Mpi::onUartDisconnected()
{
    m_isConnected = false;
}

void Mpi::onUartError(QSerialPort::SerialPortError err)
{
    qDebug() << err << Qt::endl;
}
