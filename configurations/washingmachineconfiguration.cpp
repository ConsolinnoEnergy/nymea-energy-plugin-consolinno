/* Copyright (C) Consolinno Energy GmbH - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include "washingmachineconfiguration.h"

WashingmachineConfiguration::WashingMachineConfiguration()
{
    // By default invalid
}

ThingId WashingMachineConfiguration::washingMachineThingId() const
{
    return m_washingMachineThingId;
}

void WashingMachineConfiguration::setWashingMachineThingId(const ThingId &washingMachineThingId)
{
    m_washingMachineThingId = washingMachineThingId;
}

bool WashingMachineConfiguration::optimizationEnabled() const
{
    return m_optimizationEnabled;
}

void WashingMachineConfiguration::setOptimizationEnabled(bool optimizationEnabled)
{
    m_optimizationEnabled = optimizationEnabled;
}

double WashingMachineConfiguration::maxElectricalPower() const
{
    return m_maxElectricalPower;
}

void WashingMachineConfiguration::setMaxElectricalPower(double maxElectricalPower)
{
    m_maxElectricalPower = maxElectricalPower;
}

bool WashingMachineConfiguration::isValid() const
{
    return !m_washingMachineThingId.isNull() && m_maxElectricalPower != 0;
}

bool WashingMachineConfiguration::operator==(const WashingmachineConfiguration &other) const
{
    return m_heatMeterThingId == other.washingmachineThingId() &&
            m_optimizationEnabled == other.optimizationEnabled() &&
            m_maxElectricalPower == other.maxElectricalPower();
}

bool WashingMachineConfiguration::operator!=(const WashingMachineConfiguration &other) const
{
    return !(*this == other);
}

QDebug operator<<(QDebug debug, const WashingMachineConfiguration &washingMachineConfig)
{
    debug.nospace() << "WashingmachineConfiguration(" << washingMachineConfig.washingMachineThingId().toString();
    debug.nospace() << ", " << (washingMachineConfig.optimizationEnabled() ? "enabled" : "disabled");
    debug.nospace() << ", " << "max power: " << washingMachineConfig.maxElectricalPower() << "W";
    debug.nospace() << ")";
    return debug.maybeSpace();
}
