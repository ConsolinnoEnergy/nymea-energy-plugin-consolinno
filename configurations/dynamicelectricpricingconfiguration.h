/* Copyright (C) Consolinno Energy GmbH - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef DYNAMICELECTRICPRICINGCONFIGURATION_H
#define DYNAMICELECTRICPRICINGCONFIGURATION_H

#include <QObject>
#include <QDebug>

#include <typeutils.h>

class DynamicElectricPricingConfiguration
{
    Q_GADGET

    Q_PROPERTY(QUuid dynamicElectricPricingThingId READ dynamicElectricPricingThingId WRITE setDynamicElectricPricingThingId)
    Q_PROPERTY(bool optimizationEnabled READ optimizationEnabled WRITE setOptimizationEnabled USER true)
    Q_PROPERTY(double maxElectricalPower READ maxElectricalPower WRITE setMaxElectricalPower USER true)
public:
    DynamicElectricPricingConfiguration();

    ThingId dynamicElectricPricingThingId() const;
    void setDynamicElectricPricingThingId(const ThingId &dynamicElectricPricingThingId);

    bool optimizationEnabled() const;
    void setOptimizationEnabled(bool optimizationEnabled);

    // The maximal electric power in W the dynamic electric pricing can consume
    double maxElectricalPower() const;
    void setMaxElectricalPower(double maxElectricalPower);

    bool isValid() const;

    bool operator==(const DynamicElectricPricingConfiguration &other) const;
    bool operator!=(const DynamicElectricPricingConfiguration &other) const;

private:
    ThingId m_dynamicElectricPricingThingId;
    bool m_optimizationEnabled = false;
    double m_maxElectricalPower = 9;
    ThingId m_heatMeterThingId;
};

QDebug operator<<(QDebug debug, const DynamicElectricPricingConfiguration &dynamicElectricPricingConfig);

#endif // DYNAMICELECTRICPRICINGCONFIGURATION_H
