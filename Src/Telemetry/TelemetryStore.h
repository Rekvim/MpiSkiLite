#ifndef TELEMETRYSTORE_H
#define TELEMETRYSTORE_H

#pragma once

#include <QVector>
#include <QString>
#include <QColor>

#include <QMetaType>

struct InitState {
    QString deviceStatusText = "";
    QColor  deviceStatusColor = QColor();
    QString initStatusText = "";
    QColor  initStatusColor = QColor();
    QString connectedSensorsText = "";
    QColor  connectedSensorsColor = QColor();
    QString startingPositionText = "";
    QColor  startingPositionColor = QColor();
    QString finalPositionText = "";
    QColor  finalPositionColor = QColor();
};

struct StepTestRecord {
    quint16 from = 0;
    quint16 to = 0;
    quint32 T_value = 0;
    qreal overshoot = 0.0;
};

struct RangeDeviationRecord {
    qint16 rangePercent = 0;
    double maxForwardValue = 0.0;
    qint16 maxForwardCycle = 0;
    double maxReverseValue = 0.0;
    qint16 maxReverseCycle = 0;
};

struct StrokeTestRecord {
    QString timeForwardMs = "";
    QString timeBackwardMs = "";
};

struct CrossingStatus {
    enum class State {
        Unknown,
        Ok,
        Fail
    };

    State frictionPercent = State::Unknown;
    State range = State::Unknown;
    State dynamicError = State::Unknown;
    State spring = State::Unknown;
    State linearCharacteristic = State::Unknown;
};


struct MainTestRecord {
    double dynamicError_mean = 0.0;
    double dynamicError_meanPercent = 0.0; // %

    double dynamicError_max = 0.0;   // %
    double dynamicError_maxPercent = 0.0;   // %

    double dynamicErrorReal = 0.0;   // %

    double lowLimitPressure = 0.0;  // бар
    double highLimitPressure = 0.0;  // бар

    double springLow = 0.0;  // Н
    double springHigh = 0.0;  // Н

    double pressureDifference = 0.0;  // бар
    double frictionForce = 0.0;  //
    double frictionPercent = 0.0;  // %

    double linearityError = 0.0;
    double linearity = 0.0;
};

struct ValveStrokeRecord {
    QString range = "";
    qreal real= 0.0;
};

struct SupplyRecord {
    double pressure_bar = 0.0;
};

class TelemetryStore {
public:
    InitState init;
    QVector<StepTestRecord> stepResults;
    StrokeTestRecord strokeTestRecord;
    ValveStrokeRecord valveStrokeRecord;
    SupplyRecord supplyRecord;
    MainTestRecord mainTestRecord;
    CrossingStatus crossingStatus;

    TelemetryStore() = default;

    void clearAll() {
        init = {};
        stepResults.clear();
        strokeTestRecord = {};
        valveStrokeRecord = {};
        supplyRecord = {};
        mainTestRecord = {};
    }
};

#endif // TELEMETRYSTORE_H
