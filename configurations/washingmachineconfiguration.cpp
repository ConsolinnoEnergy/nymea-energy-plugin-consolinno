/* Copyright (C) Consolinno Energy GmbH - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include "washingmachineconfiguration.h"

WashingmachineConfiguration::washingmachineConfiguration()
{
    // By default invalid
}

ThingId WashingmachineConfiguration::washingmachineThingId() const
{
    return m_washingmachineThingId;
}

void WashingmachineConfiguration::setWashingmachineThingId(const ThingId &washingmachineThingId)
{
    m_washingmachineThingId = washingmachineThingId;
}

bool WashingmachineConfiguration::optimizationEnabled() const
{
    return m_optimizationEnabled;
}

void WashingmachineConfiguration::setOptimizationEnabled(bool optimizationEnabled)
{
    m_optimizationEnabled = optimizationEnabled;
}

double WashingmachineConfiguration::maxElectricalPower() const
{
    return m_maxElectricalPower;
}

void WashingmachineConfiguration::setMaxElectricalPower(double maxElectricalPower)
{
    m_maxElectricalPower = maxElectricalPower;
}

bool WashingmachineConfiguration::isValid() const
{
    return !m_washingmachineThingId.isNull() && m_maxElectricalPower != 0;
}

bool WashingmachineConfiguration::operator==(const WashingmachineConfiguration &other) const
{
    return m_heatMeterThingId == other.washingmachineThingId() &&
            m_optimizationEnabled == other.optimizationEnabled() &&
            m_maxElectricalPower == other.maxElectricalPower();
}

bool WashingmachineConfiguration::operator!=(const WashingmachineConfiguration &other) const
{
    return !(*this == other);
}

QDebug operator<<(QDebug debug, const washingmachineConfiguration &washingmachineConfig)
{
    debug.nospace() << "WashingmachineConfiguration(" << washingmachineConfig.washingmachineThingId().toString();
    debug.nospace() << ", " << (washingmachineConfig.optimizationEnabled() ? "enabled" : "disabled");
    debug.nospace() << ", " << "max power: " << washingmachineConfig.maxElectricalPower() << "W";
    debug.nospace() << ")";
    return debug.maybeSpace();
}
