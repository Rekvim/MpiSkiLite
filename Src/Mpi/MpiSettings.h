#ifndef MPISETTINGS_H
#define MPISETTINGS_H

#pragma once
#include <QObject>
#include <QApplication>
#include <QSettings>

class MpiSettings : public QObject
{
    Q_OBJECT
public:
    explicit MpiSettings(QObject *parent = nullptr);

    struct MinMax
    {
        qreal min;
        qreal max;
    };

    struct DAC
    {
        qreal min;
        qreal max;
        qreal bias;
    };

    qreal GetAdc(quint8 num) const;
    MinMax GetSensor(quint8 num) const;
    DAC GetDac() const;

private:
    QVector<qreal> m_adc;
    QVector<MinMax> m_sensors;
    DAC m_dac;
signals:

};

#endif // MPISETTINGS_H
