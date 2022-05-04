/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2021, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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

    bool operator==(const HeatingConfiguration &other) const;
    bool operator!=(const HeatingConfiguration &other) const;

private:
    ThingId m_heatPumpThingId;
    bool m_optimizationEnabled = false;
    double m_maxElectricalPower = 0;
    double m_maxThermalEnergy = 0;
    HouseType m_houseType = HouseTypeSince1984;
    double m_floorHeatingArea = 0;
    ThingId m_heatMeterThingId;

};

QDebug operator<<(QDebug debug, const HeatingConfiguration &heatingConfig);

#endif // HEATINGCONFIGURATION_H
