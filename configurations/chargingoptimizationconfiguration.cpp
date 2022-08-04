#include "chargingoptimizationconfiguration.h"

ChargingOptimizationConfiguration::ChargingOptimizationConfiguration()
{

}

ThingId ChargingOptimizationConfiguration::evChargerThingId() const
{
    return m_evChargerThingId;
}

void ChargingOptimizationConfiguration::setEvChargerThingId(const ThingId &evChargerThingId)
{
    m_evChargerThingId = evChargerThingId;
}

bool ChargingOptimizationConfiguration::reenableChargepoint() const
{
    return m_reenableChargepoint;
}

void ChargingOptimizationConfiguration::setReenableChargepoint(bool reenableChargepoint)
{
    m_reenableChargepoint = reenableChargepoint;
}

bool ChargingOptimizationConfiguration::operator==(const ChargingOptimizationConfiguration &other) const
{
    return m_evChargerThingId == other.evChargerThingId() &&
            m_reenableChargepoint == other.reenableChargepoint();

}

bool ChargingOptimizationConfiguration::operator!=(const ChargingOptimizationConfiguration &other) const
{
    return !(*this == other);
}

QDebug operator<<(QDebug debug, const ChargingOptimizationConfiguration &chargingOptimizationConfig)
{
    debug.nospace() << "ChargingOptimizationConfiguration(" << chargingOptimizationConfig.evChargerThingId().toString();
    debug.nospace() << "Reenable manual chargepoint: " << (chargingOptimizationConfig.reenableChargepoint() ? "enabled" : "disabled");
    debug.nospace() << ")";
    return debug.maybeSpace();
}
