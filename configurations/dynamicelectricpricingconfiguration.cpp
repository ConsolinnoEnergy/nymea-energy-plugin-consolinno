/* Copyright (C) Consolinno Energy GmbH - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include "dynamicelectricpricingconfiguration.h"

DynamicElectricPricingConfiguration::DynamicElectricPricingConfiguration()
{
    // By default invalid
}

ThingId DynamicElectricPricingConfiguration::dynamicElectricPricingThingId() const
{
    return m_dynamicElectricPricingThingId;
}

void DynamicElectricPricingConfiguration::setDynamicElectricPricingThingId(const ThingId &dynamicElectricPricingThingId)
{
    m_dynamicElectricPricingThingId = dynamicElectricPricingThingId;
}

bool DynamicElectricPricingConfiguration::optimizationEnabled() const
{
    return m_optimizationEnabled;
}

void DynamicElectricPricingConfiguration::setOptimizationEnabled(bool optimizationEnabled)
{
    m_optimizationEnabled = optimizationEnabled;
}

double DynamicElectricPricingConfiguration::maxElectricalPower() const
{
    return m_maxElectricalPower;
}

void DynamicElectricPricingConfiguration::setMaxElectricalPower(double maxElectricalPower)
{
    m_maxElectricalPower = maxElectricalPower;
}

bool DynamicElectricPricingConfiguration::isValid() const
{
    return !m_dynamicElectricPricingThingId.isNull() && m_maxElectricalPower != 0;
}

bool DynamicElectricPricingConfiguration::operator==(const DynamicElectricPricingConfiguration &other) const
{
    return m_heatMeterThingId == other.dynamicElectricPricingThingId() &&
            m_optimizationEnabled == other.optimizationEnabled() &&
            m_maxElectricalPower == other.maxElectricalPower();
}

bool DynamicElectricPricingConfiguration::operator!=(const DynamicElectricPricingConfiguration &other) const
{
    return !(*this == other);
}

QDebug operator<<(QDebug debug, const DynamicElectricPricingConfiguration &dynamicElectricPricingConfig)
{
    debug.nospace() << "DynamicElectricPricingConfiguration(" << dynamicElectricPricingConfig.dynamicElectricPricingThingId().toString();
    debug.nospace() << ", " << (dynamicElectricPricingConfig.optimizationEnabled() ? "enabled" : "disabled");
    debug.nospace() << ", " << "max power: " << dynamicElectricPricingConfig.maxElectricalPower() << "W";
    debug.nospace() << ")";
    return debug.maybeSpace();
}
