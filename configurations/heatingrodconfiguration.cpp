/* Copyright (C) Consolinno Energy GmbH - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include "heatingrodconfiguration.h"

HeatingRodConfiguration::HeatingRodConfiguration()
{
    // By default invalid
}

ThingId HeatingRodConfiguration::heatingRodThingId() const
{
    return m_heatingRodThingId;
}

void HeatingRodConfiguration::setHeatingRodThingId(const ThingId &heatingRodThingId)
{
    m_heatingRodThingId = heatingRodThingId;
}

bool HeatingRodConfiguration::optimizationEnabled() const
{
    return m_optimizationEnabled;
}

void HeatingRodConfiguration::setOptimizationEnabled(bool optimizationEnabled)
{
    m_optimizationEnabled = optimizationEnabled;
}

double HeatingRodConfiguration::maxElectricalPower() const
{
    return m_maxElectricalPower;
}

void HeatingRodConfiguration::setMaxElectricalPower(double maxElectricalPower)
{
    m_maxElectricalPower = maxElectricalPower;
}

bool HeatingRodConfiguration::isValid() const
{
    return !m_heatingRodThingId.isNull() && m_maxElectricalPower != 0;
}

bool HeatingRodConfiguration::controllableLocalSystem() const
{
    return m_controllableLocalSystem;
}

void HeatingRodConfiguration::setControllableLocalSystem(bool controllableLocalSystem)
{
    m_controllableLocalSystem = controllableLocalSystem;
}

bool HeatingRodConfiguration::operator==(const HeatingRodConfiguration &other) const
{
    return m_heatMeterThingId == other.heatingRodThingId() &&
            m_optimizationEnabled == other.optimizationEnabled() &&
            m_maxElectricalPower == other.maxElectricalPower() &&
            m_controllableLocalSystem == other.optimizationEnabled();
}

bool HeatingRodConfiguration::operator!=(const HeatingRodConfiguration &other) const
{
    return !(*this == other);
}

QDebug operator<<(QDebug debug, const HeatingRodConfiguration &heatingRodConfig)
{
    debug.nospace() << "HeatingRodConfiguration(" << heatingRodConfig.heatingRodThingId().toString();
    debug.nospace() << ", " << (heatingRodConfig.optimizationEnabled() ? "enabled" : "disabled");
    debug.nospace() << ", " << "max power: " << heatingRodConfig.maxElectricalPower() << "W";
    debug.nospace() << ", CLS: " << (heatingRodConfig.controllableLocalSystem() ? "enabled" : "disabled");
    debug.nospace() << ")";
    return debug.maybeSpace();
}
