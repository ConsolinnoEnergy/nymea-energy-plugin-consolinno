/* Copyright (C) Consolinno Energy GmbH - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

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

float ChargingOptimizationConfiguration::p_value() const
{
    return m_p_value;
}

void ChargingOptimizationConfiguration::setP_value(float p_value)
{
    m_p_value = p_value;
}

float ChargingOptimizationConfiguration::i_value() const
{
    return m_i_value;
}

void ChargingOptimizationConfiguration::setI_value(float i_value)
{
    m_i_value = i_value;
}

float ChargingOptimizationConfiguration::d_value() const
{
    return m_d_value;
}

void ChargingOptimizationConfiguration::setD_value(float d_value)
{
    m_d_value = d_value;
}

float ChargingOptimizationConfiguration::setpoint() const
{
    return m_setpoint;
}

void ChargingOptimizationConfiguration::setSetpoint(float setpoint)
{
    m_setpoint = setpoint;
}


bool ChargingOptimizationConfiguration::operator==(const ChargingOptimizationConfiguration &other) const
{
    return m_evChargerThingId == other.evChargerThingId() &&
            m_p_value == other.p_value() &&
            m_i_value == other.i_value() &&
            m_d_value == other.d_value() &&
            m_setpoint == other.setpoint() &&
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
    debug.nospace() << "P value: " << (chargingOptimizationConfig.p_value());
    debug.nospace() << "I value: " << (chargingOptimizationConfig.i_value());
    debug.nospace() << "D value: " << (chargingOptimizationConfig.d_value());
    debug.nospace() << "setpoint : " << (chargingOptimizationConfig.setpoint());
    debug.nospace() << ")";
    return debug.maybeSpace();
}
