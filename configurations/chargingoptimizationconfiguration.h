/* Copyright (C) Consolinno Energy GmbH - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef CHARGINGOPTIMIZATIONCONFIGURATION_H
#define CHARGINGOPTIMIZATIONCONFIGURATION_H

#include <QDebug>
#include <QObject>

#include <typeutils.h>

class ChargingOptimizationConfiguration
{
    Q_GADGET
    Q_PROPERTY(QUuid evChargerThingId READ evChargerThingId WRITE setEvChargerThingId)
    Q_PROPERTY(bool reenableChargepoint READ reenableChargepoint WRITE setReenableChargepoint USER true)
    Q_PROPERTY(float p_value READ p_value WRITE setP_value USER true)
    Q_PROPERTY(float i_value READ i_value WRITE setI_value USER true)
    Q_PROPERTY(float d_value READ d_value WRITE setD_value USER true)
    Q_PROPERTY(float setpoint READ setpoint WRITE setSetpoint USER true)
    Q_PROPERTY(bool controllableLocalSystem READ controllableLocalSystem WRITE setControllableLocalSystem USER true)


public:
    ChargingOptimizationConfiguration();

    ThingId evChargerThingId() const;
    void setEvChargerThingId(const ThingId &evChargerThingId);

    bool reenableChargepoint() const;
    void setReenableChargepoint(bool reenableChargepoint);

    float p_value() const;
    void setP_value(const float p_value);

    float i_value() const;
    void setI_value(const float i_value);

    float d_value() const;
    void setD_value(const float d_value);

    float setpoint() const;
    void setSetpoint(const float setpoint);

    bool controllableLocalSystem() const;
    void setControllableLocalSystem(bool controllableLocalSystem);

    bool operator==(const ChargingOptimizationConfiguration &other) const;
    bool operator!=(const ChargingOptimizationConfiguration &other) const;

private:
    ThingId m_evChargerThingId;
    bool m_reenableChargepoint = false;
    float m_p_value = 0.0001;
    float m_i_value = 0.0001;
    float m_d_value = 0;
    float m_setpoint = 0;
    bool m_controllableLocalSystem = false;
};

QDebug operator<<(QDebug debug, const ChargingOptimizationConfiguration &chargingConfig);


#endif // CHARGINGOPTIMIZATIONCONFIGURATION_H
