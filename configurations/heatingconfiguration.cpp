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

#include "heatingconfiguration.h"

HeatingConfiguration::HeatingConfiguration()
{

}

ThingId HeatingConfiguration::heatPumpThingId() const
{
    return m_heatPumpThingId;
}

void HeatingConfiguration::setHeatPumpThingId(const ThingId &heatPumpThingId)
{
    m_heatPumpThingId = heatPumpThingId;
}

bool HeatingConfiguration::optimizationEnabled() const
{
    return m_optimizationEnabled;
}

void HeatingConfiguration::setOptimizationEnabled(bool optimizationEnabled)
{
    m_optimizationEnabled = optimizationEnabled;
}

ThingId HeatingConfiguration::heatMeterThingId() const
{
    return m_heatMeterThingId;
}

void HeatingConfiguration::setHeatMeterThingId(const ThingId &heatMeterThingId)
{
    m_heatMeterThingId = heatMeterThingId;
}

bool HeatingConfiguration::operator==(const HeatingConfiguration &other) const
{
    return m_heatMeterThingId == other.heatPumpThingId() &&
            m_optimizationEnabled == other.optimizationEnabled() &&
            m_heatMeterThingId == other.heatMeterThingId();
}

bool HeatingConfiguration::operator!=(const HeatingConfiguration &other) const
{
    return !(*this == other);
}

QDebug operator<<(QDebug debug, const HeatingConfiguration &heatingConfig)
{
    debug.nospace() << "HeatingConfiguration(" << heatingConfig.heatPumpThingId().toString();
    debug.nospace() << ", " << (heatingConfig.optimizationEnabled() ? "enabled" : "disabled");
    if (!heatingConfig.heatMeterThingId().isNull()) {
        debug.nospace() << ", heat meter: " << heatingConfig.heatMeterThingId().toString();
    }
    debug.nospace() << ")";
    return debug.maybeSpace();
}
