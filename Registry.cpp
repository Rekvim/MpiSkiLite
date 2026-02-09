#include "Registry.h"

Registry::Registry() : m_settings("MPPI", "Data")
{}

void Registry::loadObjectInfo()
{
    m_objectInfo.object = m_settings.value("object").toString();
    m_objectInfo.manufactory = m_settings.value("manufactory").toString();
    m_objectInfo.department = m_settings.value("department").toString();
    m_objectInfo.FIO = m_settings.value("FIO").toString();
}

ObjectInfo& Registry::objectInfo()
{
    return m_objectInfo;
}

const ObjectInfo& Registry::objectInfo() const
{
    return m_objectInfo;
}

void Registry::saveObjectInfo()
{
    m_settings.setValue("object", m_objectInfo.object);
    m_settings.setValue("manufactory", m_objectInfo.manufactory);
    m_settings.setValue("department", m_objectInfo.department);
    m_settings.setValue("FIO", m_objectInfo.FIO);
}

bool Registry::loadMaterialsOfComponentParts(const QString& position)
{
    m_settings.beginGroup(m_objectInfo.object);
    m_settings.beginGroup(m_objectInfo.manufactory);
    m_settings.beginGroup(m_objectInfo.department);

    if (!m_settings.childGroups().contains(position)) {
        m_settings.endGroup();
        m_settings.endGroup();
        m_settings.endGroup();
        return false;
    }

    m_settings.beginGroup(position);

    m_materialsOfComponentParts = {};

    m_materialsOfComponentParts.saddle = m_settings.value("saddle", "").toString();
    m_materialsOfComponentParts.CV = m_settings.value("CV", "").toString();
    m_materialsOfComponentParts.stuffingBoxSeal = m_settings.value("stuffingBoxSeal", "").toString();
    m_materialsOfComponentParts.corpus = m_settings.value("corpus", "").toString();
    m_materialsOfComponentParts.cap = m_settings.value("cap", "").toString();
    m_materialsOfComponentParts.ball = m_settings.value("ball", "").toString();
    m_materialsOfComponentParts.disk = m_settings.value("disk", "").toString();
    m_materialsOfComponentParts.plunger = m_settings.value("plunger", "").toString();
    m_materialsOfComponentParts.shaft = m_settings.value("shaft", "").toString();
    m_materialsOfComponentParts.stock = m_settings.value("stock", "").toString();
    m_materialsOfComponentParts.guideSleeve = m_settings.value("guideSleeve", "").toString();

    m_settings.endGroup();
    m_settings.endGroup();
    m_settings.endGroup();
    m_settings.endGroup();

    return true;
}

void Registry::saveMaterialsOfComponentParts()
{
    m_settings.beginGroup(m_objectInfo.object);
    m_settings.beginGroup(m_objectInfo.manufactory);
    m_settings.beginGroup(m_objectInfo.department);

    m_settings.beginGroup(m_valveInfo.positionNumber);

    m_settings.setValue("CV", m_materialsOfComponentParts.CV);
    m_settings.setValue("saddle", m_materialsOfComponentParts.saddle);
    m_settings.setValue("stuffingBoxSeal", m_materialsOfComponentParts.stuffingBoxSeal);
    m_settings.setValue("corpus", m_materialsOfComponentParts.corpus);
    m_settings.setValue("cap", m_materialsOfComponentParts.cap);
    m_settings.setValue("ball", m_materialsOfComponentParts.ball);
    m_settings.setValue("disk", m_materialsOfComponentParts.disk);
    m_settings.setValue("plunger", m_materialsOfComponentParts.plunger);
    m_settings.setValue("shaft", m_materialsOfComponentParts.shaft);
    m_settings.setValue("stock", m_materialsOfComponentParts.stock);
    m_settings.setValue("guideSleeve", m_materialsOfComponentParts.guideSleeve);

    m_settings.endGroup();
    m_settings.endGroup();
    m_settings.endGroup();
    m_settings.endGroup();
    m_settings.sync();
}

MaterialsOfComponentParts& Registry::materialsOfComponentParts()
{
    return m_materialsOfComponentParts;
}

const MaterialsOfComponentParts& Registry::materialsOfComponentParts() const
{
    return m_materialsOfComponentParts;
}

bool Registry::loadValveInfo(const QString& position)
{
    m_settings.beginGroup(m_objectInfo.object);
    m_settings.beginGroup(m_objectInfo.manufactory);
    m_settings.beginGroup(m_objectInfo.department);

    if (!m_settings.childGroups().contains(position)) {
        m_settings.endGroup();
        m_settings.endGroup();
        m_settings.endGroup();
        return false;
    }

    m_settings.beginGroup(position);

    m_valveInfo = {};
    m_valveInfo.positionNumber = position;

    m_valveInfo.manufacturer = m_settings.value("manufacturer", "").toString();
    m_valveInfo.valveModel = m_settings.value("valveModel", "").toString();
    m_valveInfo.serialNumber = m_settings.value("serialNumber", "").toString();
    m_valveInfo.DN = m_settings.value("DN", "").toString();
    m_valveInfo.PN = m_settings.value("PN", "").toString();
    m_valveInfo.positionerModel = m_settings.value("positionerModel", "").toString();
    m_valveInfo.solenoidValveModel = m_settings.value("solenoidValveModel", "").toString();
    m_valveInfo.limitSwitchModel = m_settings.value("limitSwitchModel", "").toString();
    m_valveInfo.positionSensorModel = m_settings.value("positionSensorModel", "").toString();
    m_valveInfo.dinamicErrorRecomend = m_settings.value("dinamicErrorRecomend", "").toDouble();
    m_valveInfo.strokeMovement = m_settings.value("strokeMovement", "").toInt();
    m_valveInfo.strokValve = m_settings.value("strokValve", "").toString();
    m_valveInfo.driveModel = m_settings.value("driveModel", "").toString();
    m_valveInfo.safePosition = m_settings.value("safePosition", "").toInt();
    m_valveInfo.driveType = m_settings.value("driveType", "").toInt();

    m_valveInfo.driveRangeLow = m_settings.value("driveRangeLow", 0.0).toDouble();
    m_valveInfo.driveRangeHigh = m_settings.value("driveRangeHigh", 0.0).toDouble();

    m_valveInfo.driveDiameter = m_settings.value("driveDiameter", "").toDouble();
    m_valveInfo.toolNumber = m_settings.value("toolNumber", "").toInt();
    m_valveInfo.diameterPulley = m_settings.value("diameterPulley", "").toInt();
    m_valveInfo.materialStuffingBoxSeal = m_settings.value("materialStuffingBoxSeal", "").toString();

    // --- crossing limits: enable-флаги ---
    m_valveInfo.crossingLimits.frictionEnabled =
        m_settings.value("crossing_enable_friction", false).toBool();
    m_valveInfo.crossingLimits.linearCharacteristicEnabled =
        m_settings.value("crossing_enable_linearCharacteristic", false).toBool();
    m_valveInfo.crossingLimits.rangeEnabled =
        m_settings.value("crossing_enable_range", false).toBool();
    m_valveInfo.crossingLimits.springEnabled =
        m_settings.value("crossing_enable_spring", false).toBool();
    m_valveInfo.crossingLimits.dynamicErrorEnabled =
        m_settings.value("crossing_enable_dynamicError", false).toBool();

    // --- новые поля crossingLimits ---
    m_valveInfo.crossingLimits.frictionCoefLowerLimit =
        m_settings.value("crossing_frictionCoefLowerLimit", 0.0).toDouble();
    m_valveInfo.crossingLimits.frictionCoefUpperLimit =
        m_settings.value("crossing_frictionCoefUpperLimit", 0.0).toDouble();

    m_valveInfo.crossingLimits.linearCharacteristicLowerLimit =
        m_settings.value("crossing_linearCharacteristicLowerLimit", 0.0).toDouble();

    m_valveInfo.crossingLimits.rangeUpperLimit =
        m_settings.value("crossing_rangeUpperLimit", 0.0).toDouble();

    m_valveInfo.crossingLimits.springLowerLimit =
        m_settings.value("crossing_springLowerLimit", 0.0).toDouble();
    m_valveInfo.crossingLimits.springUpperLimit =
        m_settings.value("crossing_springUpperLimit", 0.0).toDouble();

    m_settings.endGroup();
    m_settings.endGroup();
    m_settings.endGroup();
    m_settings.endGroup();

    return true;
}

void Registry::saveValveInfo()
{
    m_settings.beginGroup(m_objectInfo.object);
    m_settings.beginGroup(m_objectInfo.manufactory);
    m_settings.beginGroup(m_objectInfo.department);

    m_settings.setValue("last_position", m_valveInfo.positionNumber);

    m_settings.beginGroup(m_valveInfo.positionNumber);

    m_settings.setValue("manufacturer", m_valveInfo.manufacturer);
    m_settings.setValue("valveModel", m_valveInfo.valveModel);
    m_settings.setValue("serialNumber", m_valveInfo.serialNumber);
    m_settings.setValue("DN", m_valveInfo.DN);
    m_settings.setValue("PN", m_valveInfo.PN);
    m_settings.setValue("positionerModel", m_valveInfo.positionerModel);
    m_settings.setValue("solenoidValveModel", m_valveInfo.solenoidValveModel);
    m_settings.setValue("limitSwitchModel", m_valveInfo.limitSwitchModel);
    m_settings.setValue("positionSensorModel", m_valveInfo.positionSensorModel);
    m_settings.setValue("dinamicErrorRecomend", m_valveInfo.dinamicErrorRecomend);
    m_settings.setValue("strokeMovement", m_valveInfo.strokeMovement);
    m_settings.setValue("strokValve", m_valveInfo.strokValve);
    m_settings.setValue("driveModel", m_valveInfo.driveModel);
    m_settings.setValue("safePosition", m_valveInfo.safePosition);
    m_settings.setValue("driveType", m_valveInfo.driveType);
    // m_settings.setValue("driveRecomendRange", m_valveInfo.driveRecomendRange);
    m_settings.setValue("driveRangeLow",  m_valveInfo.driveRangeLow);
    m_settings.setValue("driveRangeHigh", m_valveInfo.driveRangeHigh);
    m_settings.setValue("driveDiameter", m_valveInfo.driveDiameter);
    m_settings.setValue("toolNumber", m_valveInfo.toolNumber);
    m_settings.setValue("diameterPulley", m_valveInfo.diameterPulley);
    m_settings.setValue("materialStuffingBoxSeal", m_valveInfo.materialStuffingBoxSeal);

    // crossing limits
    m_settings.setValue("crossing_enable_friction",
                        m_valveInfo.crossingLimits.frictionEnabled);
    m_settings.setValue("crossing_enable_linearCharacteristic",
                        m_valveInfo.crossingLimits.linearCharacteristicEnabled);
    m_settings.setValue("crossing_enable_range",
                        m_valveInfo.crossingLimits.rangeEnabled);
    m_settings.setValue("crossing_enable_spring",
                        m_valveInfo.crossingLimits.springEnabled);
    m_settings.setValue("crossing_enable_dynamicError",
                        m_valveInfo.crossingLimits.dynamicErrorEnabled);

    m_settings.setValue("crossing_frictionCoefLowerLimit",
                        m_valveInfo.crossingLimits.frictionCoefLowerLimit);
    m_settings.setValue("crossing_frictionCoefUpperLimit",
                        m_valveInfo.crossingLimits.frictionCoefUpperLimit);

    m_settings.setValue("crossing_linearCharacteristicLowerLimit",
                        m_valveInfo.crossingLimits.linearCharacteristicLowerLimit);

    m_settings.setValue("crossing_rangeUpperLimit",
                        m_valveInfo.crossingLimits.rangeUpperLimit);

    m_settings.setValue("crossing_springLowerLimit",
                        m_valveInfo.crossingLimits.springLowerLimit);
    m_settings.setValue("crossing_springUpperLimit",
                        m_valveInfo.crossingLimits.springUpperLimit);

    m_settings.endGroup();
    m_settings.endGroup();
    m_settings.endGroup();
    m_settings.endGroup();
}


ValveInfo& Registry::valveInfo()
{
    return m_valveInfo;
}

const ValveInfo& Registry::valveInfo() const
{
    return m_valveInfo;
}


OtherParameters& Registry::otherParameters()
{
    return m_otherParameters;
}


const OtherParameters& Registry::otherParameters() const
{
    return m_otherParameters;
}


bool Registry::checkObject(const QString &object)
{
    return m_settings.childGroups().contains(object);
}

bool Registry::checkManufactory(const QString &manufactory)
{
    m_settings.beginGroup(m_objectInfo.object);

    bool result = m_settings.childGroups().contains(manufactory);

    m_settings.endGroup();

    return result;
}

bool Registry::checkDepartment(const QString &department)
{
    m_settings.beginGroup(m_objectInfo.object);
    m_settings.beginGroup(m_objectInfo.manufactory);

    bool result = m_settings.childGroups().contains(department);

    m_settings.endGroup();
    m_settings.endGroup();

    return result;
}

bool Registry::checkPosition(const QString &position)
{
    m_settings.beginGroup(m_objectInfo.object);
    m_settings.beginGroup(m_objectInfo.manufactory);
    m_settings.beginGroup(m_objectInfo.department);

    bool result = m_settings.childGroups().contains(position);

    m_settings.endGroup();
    m_settings.endGroup();
    m_settings.endGroup();

    return result;
}

QStringList Registry::positions()
{
    m_settings.beginGroup(m_objectInfo.object);
    m_settings.beginGroup(m_objectInfo.manufactory);
    m_settings.beginGroup(m_objectInfo.department);

    QStringList result = m_settings.childGroups();

    m_settings.endGroup();
    m_settings.endGroup();
    m_settings.endGroup();

    return result;
}

QString Registry::lastPosition()
{
    m_settings.beginGroup(m_objectInfo.object);
    m_settings.beginGroup(m_objectInfo.manufactory);
    m_settings.beginGroup(m_objectInfo.department);

    QString result = m_settings.value("last_position", "").toString();

    m_settings.endGroup();
    m_settings.endGroup();
    m_settings.endGroup();

    return result;
}
