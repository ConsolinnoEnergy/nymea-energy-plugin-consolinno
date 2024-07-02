/* Copyright (C) Consolinno Energy GmbH - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include "batteryconfiguration.h"

BatteryConfiguration::BatteryConfiguration()
{

}

ThingId BatteryConfiguration::batteryThingId() const
{
    return m_batteryThingId;
}

void BatteryConfiguration::setBatteryThingId(const ThingId &batteryThingId)
{
    m_batteryThingId = batteryThingId;
}

bool BatteryConfiguration::optimizationEnabled() const
{
    return m_optimizationEnabled;
}

void BatteryConfiguration::setOptimizationEnabled( bool optimizationEnabled)
{
    m_optimizationEnabled = optimizationEnabled;
}

bool BatteryConfiguration::controllableLocalSystem() const
{
    return m_controllableLocalSystem;
}

void BatteryConfiguration::setControllableLocalSystem(bool controllableLocalSystem)
{
    m_controllableLocalSystem = controllableLocalSystem;
}

bool BatteryConfiguration::operator==(const BatteryConfiguration &other) const
{
    return m_batteryThingId == other.batteryThingId() &&
            m_optimizationEnabled == other.optimizationEnabled();
}

bool BatteryConfiguration::operator!=(const BatteryConfiguration &other) const
{
    return !(*this == other);
}

QDebug operator<<(QDebug debug, const BatteryConfiguration &batteryConfig)
{
    debug.nospace() << "BatteryConfiguration(" << batteryConfig.batteryThingId().toString();
    debug.nospace() << ", optimization: " << (batteryConfig.optimizationEnabled() ? "enabled" : "disabled");
    debug.nospace() << ", CLS: " << (batteryConfig.controllableLocalSystem() ? "enabled" : "disabled");
    debug.nospace() << ")";
    return debug.maybeSpace();
}





