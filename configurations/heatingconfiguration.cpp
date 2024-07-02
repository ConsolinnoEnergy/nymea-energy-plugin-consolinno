/* Copyright (C) Consolinno Energy GmbH - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include "heatingconfiguration.h"

HeatingConfiguration::HeatingConfiguration()
{
    // By default invalid
}

ThingId HeatingConfiguration::heatPumpThingId() const
{
    return m_heatPumpThingId;
}

void HeatingConfiguration::setHeatPumpThingId(const ThingId &heatPumpThingId)
{
    m_heatPumpThingId = heatPumpThingId;
}

bool HeatingConfiguration::optimizationEnabled() const
{
    return m_optimizationEnabled;
}

void HeatingConfiguration::setOptimizationEnabled(bool optimizationEnabled)
{
    m_optimizationEnabled = optimizationEnabled;
}

double HeatingConfiguration::maxElectricalPower() const
{
    return m_maxElectricalPower;
}

void HeatingConfiguration::setMaxElectricalPower(double maxElectricalPower)
{
    m_maxElectricalPower = maxElectricalPower;
}

double HeatingConfiguration::maxThermalEnergy() const
{
    return m_maxThermalEnergy;
}

void HeatingConfiguration::setMaxThermalEnergy(double maxThermalEnergy)
{
    m_maxThermalEnergy = maxThermalEnergy;
}

HeatingConfiguration::HouseType HeatingConfiguration::houseType() const
{
    return m_houseType;
}

void HeatingConfiguration::setHouseType(HouseType houseType)
{
    m_houseType = houseType;
}

double HeatingConfiguration::floorHeatingArea() const
{
    return m_floorHeatingArea;
}

void HeatingConfiguration::setFloorHeatingArea(double floorHeatingArea)
{
    m_floorHeatingArea = floorHeatingArea;
}

ThingId HeatingConfiguration::heatMeterThingId() const
{
    return m_heatMeterThingId;
}

void HeatingConfiguration::setHeatMeterThingId(const ThingId &heatMeterThingId)
{
    m_heatMeterThingId = heatMeterThingId;
}

bool HeatingConfiguration::isValid() const
{
    return !m_heatPumpThingId.isNull() && m_maxElectricalPower != 0 && m_maxThermalEnergy != 0 && m_floorHeatingArea != 0;
}

bool HeatingConfiguration::controllableLocalSystem() const
{
    return m_controllableLocalSystem;
}

void HeatingConfiguration::setControllableLocalSystem(bool controllableLocalSystem)
{
    m_controllableLocalSystem = controllableLocalSystem;
}

bool HeatingConfiguration::operator==(const HeatingConfiguration &other) const
{
    return m_heatMeterThingId == other.heatPumpThingId() &&
            m_optimizationEnabled == other.optimizationEnabled() &&
            m_maxElectricalPower == other.maxElectricalPower() &&
            m_maxThermalEnergy == other.maxThermalEnergy() &&
            m_houseType == other.houseType() &&
            m_floorHeatingArea == other.floorHeatingArea() &&
            m_heatMeterThingId == other.heatMeterThingId();
}

bool HeatingConfiguration::operator!=(const HeatingConfiguration &other) const
{
    return !(*this == other);
}

QDebug operator<<(QDebug debug, const HeatingConfiguration &heatingConfig)
{
    debug.nospace() << "HeatingConfiguration(" << heatingConfig.heatPumpThingId().toString();
    debug.nospace() << ", " << (heatingConfig.optimizationEnabled() ? "enabled" : "disabled");
    debug.nospace() << ", " << "max power: " << heatingConfig.maxElectricalPower() << "kW";
    debug.nospace() << ", " << "max thermal energy: " << heatingConfig.maxThermalEnergy() << "kWh";
    debug.nospace() << ", " << heatingConfig.houseType();
    debug.nospace() << ", area: " << heatingConfig.floorHeatingArea() << "m^2";
    if (!heatingConfig.heatMeterThingId().isNull()) {
        debug.nospace() << ", heat meter: " << heatingConfig.heatMeterThingId().toString();
    }
    debug.nospace() << ", CLS: " << (heatingConfig.controllableLocalSystem() ? "enabled" : "disabled");
    debug.nospace() << ")";
    return debug.maybeSpace();
}
