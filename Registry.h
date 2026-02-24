#pragma once

#include <QDate>
#include <QObject>
#include <QSettings>

struct CrossingLimits {

    bool frictionEnabled = false;
    bool linearCharacteristicEnabled = false;
    bool rangeEnabled = false;
    bool springEnabled = false;
    bool dynamicErrorEnabled = false;

    double frictionCoefLowerLimit = 0.0;
    double frictionCoefUpperLimit = 0.0;

    double linearCharacteristicLowerLimit = 0.0;

    double rangeUpperLimit  = 0.0;

    double springLowerLimit = 0.0;
    double springUpperLimit = 0.0;
};

struct ObjectInfo
{
    QString object = "";
    QString manufactory = "";
    QString department = "";
    QString FIO = "";
};

struct MaterialsOfComponentParts {
    QString plunger;
    QString saddle;
    QString cap;
    QString corpus;
    QString ball;
    QString disk;
    QString shaft;
    QString stock;
    QString guideSleeve;
    QString stuffingBoxSeal;
    QString CV;
};

struct ValveInfo
{
    QString positionNumber = "";

    QString manufacturer = "";
    QString valveModel = "";
    QString serialNumber = "";
    QString DN = "";
    QString PN = "";
    QString positionerModel = "";
    QString positionerType = "";
    qreal dinamicErrorRecomend = 0.0;

    QString solenoidValveModel = "";
    QString limitSwitchModel = "";
    QString positionSensorModel = "";

    quint32 strokeMovement = 0;
    QString strokValve = "";
    QString driveModel = "";
    quint32 safePosition = 0;
    quint32 driveType = 0;

    QString driveRecomendRange = "";
    double driveRangeLow = 0.0;
    double driveRangeHigh = 0.0;

    qreal driveDiameter = 0.0;

    quint32 toolNumber = 0;
    qreal diameterPulley = 0.0;
    QString materialStuffingBoxSeal = "";

    CrossingLimits crossingLimits;
};

struct OtherParameters
{
    QString date = "";
    QString safePosition = "";
    QString strokeMovement = "";
};

class Registry
{
public:
    Registry();

    // ObjectInfo
    void loadObjectInfo();
    void saveObjectInfo();

    ObjectInfo& objectInfo();
    const ObjectInfo& objectInfo() const;

    // ValveInfo
    bool loadMaterialsOfComponentParts(const QString& position);
    void saveMaterialsOfComponentParts();

    MaterialsOfComponentParts& materialsOfComponentParts();
    const MaterialsOfComponentParts& materialsOfComponentParts() const;

    // ValveInfo
    bool loadValveInfo(const QString& position);
    void saveValveInfo();

    ValveInfo& valveInfo();
    const ValveInfo& valveInfo() const;

    // Other
    OtherParameters& otherParameters();
    const OtherParameters& otherParameters() const;

    QStringList positions();
    QString lastPosition();

    bool checkObject(const QString &object);
    bool checkManufactory(const QString &manufactory);
    bool checkDepartment(const QString &department);
    bool checkPosition(const QString &position);

private:
    QSettings m_settings;
    ObjectInfo m_objectInfo;
    MaterialsOfComponentParts m_materialsOfComponentParts;
    ValveInfo m_valveInfo;

    OtherParameters m_otherParameters;
};
