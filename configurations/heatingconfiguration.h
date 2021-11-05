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

#include <typeutils.h>

class HeatingConfiguration
{
    Q_GADGET
    Q_PROPERTY(ThingId heatPumpThingId READ heatPumpThingId WRITE setHeatPumpThingId)
    Q_PROPERTY(bool optimizationEnabled READ optimizationEnabled WRITE setOptimizationEnabled USER true)
    Q_PROPERTY(ThingId heatMeterThingId READ heatMeterThingId WRITE setHeatMeterThingId USER true)

public:
    HeatingConfiguration();

    ThingId heatPumpThingId() const;
    void setHeatPumpThingId(const ThingId &heatPumpThingId);

    bool optimizationEnabled() const;
    void setOptimizationEnabled(bool optimizationEnabled);

    ThingId heatMeterThingId() const;
    void setHeatMeterThingId(const ThingId &heatMeterThingId);

    bool operator==(const HeatingConfiguration &other) const;
    bool operator!=(const HeatingConfiguration &other) const;

private:
    ThingId m_heatPumpThingId;
    bool m_optimizationEnabled = false;
    ThingId m_heatMeterThingId;

};

#endif // HEATINGCONFIGURATION_H
