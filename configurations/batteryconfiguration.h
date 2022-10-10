/* Copyright (C) Consolinno Energy GmbH - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef BATTERYCONFIGURATION_H
#define BATTERYCONFIGURATION_H
#include <QObject>
#include <QDebug>

#include <typeutils.h>

class BatteryConfiguration
{
    Q_GADGET
    Q_PROPERTY(QUuid batteryThingId READ batteryThingId WRITE setBatteryThingId)
    Q_PROPERTY(bool optimizationEnabled READ optimizationEnabled WRITE setOptimizationEnabled USER true)

public:
    BatteryConfiguration();

    ThingId batteryThingId() const;
    void setBatteryThingId(const ThingId &batteryThingId);

    bool optimizationEnabled() const;
    void setOptimizationEnabled(bool optimizationEnabled) ;

    bool operator==(const BatteryConfiguration &other) const;
    bool operator!=(const BatteryConfiguration &other) const;

private:
    ThingId m_batteryThingId;
    bool m_optimizationEnabled = true;

};

QDebug operator<<(QDebug debug, const BatteryConfiguration &batteryConfig);

#endif // BATTERYCONFIGURATION_H
