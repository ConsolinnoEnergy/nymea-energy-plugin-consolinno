/* Copyright (C) Consolinno Energy GmbH - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef HEATINGCONFIGURATION_H
#define HEATINGCONFIGURATION_H

#include <QObject>
#include <QDebug>

#include <typeutils.h>





class HeatingConfiguration
{
    Q_GADGET
    Q_PROPERTY(QUuid heatPumpThingId READ heatPumpThingId WRITE setHeatPumpThingId)
    Q_PROPERTY(bool optimizationEnabled READ optimizationEnabled WRITE setOptimizationEnabled USER true)
    Q_PROPERTY(double maxElectricalPower READ maxElectricalPower WRITE setMaxElectricalPower USER true)
    Q_PROPERTY(double maxThermalEnergy READ maxThermalEnergy WRITE setMaxThermalEnergy USER true)
    Q_PROPERTY(HouseType houseType READ houseType WRITE setHouseType USER true)
    Q_PROPERTY(double floorHeatingArea READ floorHeatingArea WRITE setFloorHeatingArea USER true)
    Q_PROPERTY(QUuid heatMeterThingId READ heatMeterThingId WRITE setHeatMeterThingId USER true)
    Q_PROPERTY(bool controllableLocalSystem READ controllableLocalSystem WRITE setControllableLocalSystem USER true)

public:
    HeatingConfiguration();

    enum HouseType {
        HouseTypePassive,
        HouseTypeLowEnergy,
        HouseTypeEnEV2016,
        HouseTypeBefore1949,
        HouseTypeSince1949,
        HouseTypeSince1969,
        HouseTypeSince1979,
        HouseTypeSince1984
    };

    Q_ENUM(HouseType)

    ThingId heatPumpThingId() const;
    void setHeatPumpThingId(const ThingId &heatPumpThingId);

    bool optimizationEnabled() const;
    void setOptimizationEnabled(bool optimizationEnabled);

    // The maximal electric power in W the heat pump can consume
    double maxElectricalPower() const;
    void setMaxElectricalPower(double maxElectricalPower);

    // The maximal thermal energy in kWh the heat pump can produce
    double maxThermalEnergy() const;
    void setMaxThermalEnergy(double maxThermalEnergy);

    HeatingConfiguration::HouseType houseType() const;
    void setHouseType(HouseType houseType);

    // Area of the floor heating in m^2
    double floorHeatingArea() const;
    void setFloorHeatingArea(double floorHeatingArea);

    // Optional extra information about heating energy produced
    ThingId heatMeterThingId() const;
    void setHeatMeterThingId(const ThingId &heatMeterThingId);

    bool isValid() const;

    bool controllableLocalSystem() const;
    void setControllableLocalSystem(bool controllableLocalSystem);

    bool operator==(const HeatingConfiguration &other) const;
    bool operator!=(const HeatingConfiguration &other) const;

private:
    ThingId m_heatPumpThingId;
    bool m_optimizationEnabled = false;
    double m_maxElectricalPower = 9;
    double m_maxThermalEnergy = 5;
    HouseType m_houseType = HouseTypeSince1984;
    double m_floorHeatingArea = 100;
    ThingId m_heatMeterThingId;
    bool m_controllableLocalSystem = false;
};

QDebug operator<<(QDebug debug, const HeatingConfiguration &heatingConfig);

#endif // HEATINGCONFIGURATION_H
