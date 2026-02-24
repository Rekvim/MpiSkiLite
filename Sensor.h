#pragma once

#include <QObject>

class Sensor : public QObject
{
    Q_OBJECT
public:
    explicit Sensor(QObject *parent = nullptr);

    [[nodiscard]] quint16 rawValue() const;
    [[nodiscard]] quint16 rawFromValue(qreal physicalValue) const;
    [[nodiscard]] qreal value() const;
    [[nodiscard]] qreal valueFromPercent(qreal percent) const;
    [[nodiscard]] qreal percent() const;
    [[nodiscard]] QString percentFormatted() const;
    [[nodiscard]] QString formattedValue() const;

    void setValue(quint16 value);
    void setCoefficients(qreal k, qreal b);
    void correctCoefficients(qreal scale);
    void captureMin();
    void captureMax();
    void setUnit(const QString &unit);

private:
    quint16 m_value = 0;
    quint16 m_minValue = 0;
    quint16 m_maxValue = 0;
    QString m_unit = "%.2f";
    qreal m_k = 1.0;
    qreal m_b = 0.0;
};
