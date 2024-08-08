/* Copyright (C) Consolinno Energy GmbH - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include "evchargerconfiguration.h"

EVchargerConfiguration::EVchargerConfiguration()
{

}

ThingId EVchargerConfiguration::evchargerThingId() const
{
    return m_evchargerThingId;
}

void EVchargerConfiguration::setEVchargerThingId(const ThingId &evchargerThingId)
{
    m_evchargerThingId = evchargerThingId;
}



bool EVchargerConfiguration::controllableLocalSystem() const
{
    return m_controllableLocalSystem;
}

void EVchargerConfiguration::setControllableLocalSystem(bool controllableLocalSystem)
{
    m_controllableLocalSystem = controllableLocalSystem;
}

bool EVchargerConfiguration::operator==(const EVchargerConfiguration &other) const
{
    return m_evchargerThingId == other.evchargerThingId() 
    && m_controllableLocalSystem == other.optimizationEnabled();
}

bool EVchargerConfiguration::operator!=(const EVchargerConfiguration &other) const
{
    return !(*this == other);
}

QDebug operator<<(QDebug debug, const EVchargerConfiguration &evchargerConfig)
{
    debug.nospace() << "EVchargerConfiguration(" << evchargerConfig.evchargerThingId().toString();
    debug.nospace() << ", CLS: " << (evchargerConfig.controllableLocalSystem() ? "enabled" : "disabled");
    debug.nospace() << ")";
    return debug.maybeSpace();
}





