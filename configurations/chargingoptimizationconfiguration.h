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



public:
    ChargingOptimizationConfiguration();

    ThingId evChargerThingId() const;
    void setEvChargerThingId(const ThingId &evChargerThingId);

    bool reenableChargepoint() const;
    void setReenableChargepoint(bool reenableChargepoint);

    bool operator==(const ChargingOptimizationConfiguration &other) const;
    bool operator!=(const ChargingOptimizationConfiguration &other) const;

private:
    ThingId m_evChargerThingId;
    bool m_reenableChargepoint = false;


};

QDebug operator<<(QDebug debug, const ChargingOptimizationConfiguration &chargingConfig);


#endif // CHARGINGOPTIMIZATIONCONFIGURATION_H
