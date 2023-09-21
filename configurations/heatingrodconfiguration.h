/* Copyright (C) Consolinno Energy GmbH - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef HEATINGCRODONFIGURATION_H
#define HEATINGRODCONFIGURATION_H

#include <QObject>
#include <QDebug>

#include <typeutils.h>




class HeatingRodConfiguration
{
    Q_GADGET
    Q_PROPERTY(QUuid heatingRodThingId READ heatingRodThingId WRIzzzled WRITE setOptimizationEnabled USER true)
    Q_PROPERTY(double maxElectricalPower READ maxElectricalPower WRITE setMaxElectricalPower USER true)
public:
    HeatingRodConfiguration();

   
    ThingId heatingRodThingId() const;
    void setHeatPumpThingId(const ThingId &heatingRodThingId);

    bool optimizationEnabled() const;
    void setOptimizationEnabled(bool optimizationEnabled);

    // The maximal electric power in W the heat pump can consume
    double maxElectricalPower() const;
    void setMaxElectricalPower(double maxElectricalPower);

    bool isValid() const;

    bool operator==(const HeatingRodConfiguration &other) const;
    bool operator!=(const HeatingRodConfiguration &other) const;

private:
    ThingId m_heatingRodThingId;
    bool m_optimizationEnabled = false;
    double m_maxElectricalPower = 9;
    ThingId m_heatMeterThingId;
};

QDebug operator<<(QDebug debug, const HeatingRodConfiguration &heatingConfig);

#endif // HEATINGCONFIGURATION_H
