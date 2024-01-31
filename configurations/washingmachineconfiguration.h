/* Copyright (C) Consolinno Energy GmbH - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef WASHINGMACHINECONFIGURATION_H
#define WASHINGMACHINECONFIGURATION_H

#include <QObject>
#include <QDebug>

#include <typeutils.h>




class WashingMachineConfiguration
{
    Q_GADGET

    Q_PROPERTY(QUuid washingMachineThingId READ washingMachineThingId WRITE setWashingMachineThingId)
    Q_PROPERTY(bool optimizationEnabled READ optimizationEnabled WRITE setOptimizationEnabled USER true)
    Q_PROPERTY(double maxElectricalPower READ maxElectricalPower WRITE setMaxElectricalPower USER true)
public:
    WashingMachineConfiguration();

   
    ThingId washingMachineThingId() const;
    void setWashingMachineThingId(const ThingId &washingMachineThingId);

    bool optimizationEnabled() const;
    void setOptimizationEnabled(bool optimizationEnabled);

    // The maximal electric power in W the washing machine can consume
    double maxElectricalPower() const;
    void setMaxElectricalPower(double maxElectricalPower);

    bool isValid() const;

    bool operator==(const WashingMachineConfiguration &other) const;
    bool operator!=(const WashingMachineConfiguration &other) const;

private:
    ThingId m_washingMachineThingId;
    bool m_optimizationEnabled = false;
    double m_maxElectricalPower = 9;
    ThingId m_heatMeterThingId;
};

QDebug operator<<(QDebug debug, const WashingMachineConfiguration &washerConfig);

#endif // WASHINGMACHINECONFIGURATION_H
