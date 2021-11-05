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

#include "chargingconfiguration.h"

ChargingConfiguration::ChargingConfiguration()
{

}

ThingId ChargingConfiguration::evChargerThingId() const
{
    return m_evChargerThingId;
}

void ChargingConfiguration::setEvChargerThingId(const ThingId &evChargerThingId)
{
    m_evChargerThingId = evChargerThingId;
}

bool ChargingConfiguration::optimizationEnabled() const
{
    return m_optimizationEnabled;
}

void ChargingConfiguration::setOptimizationEnabled(bool optimizationEnabled)
{
    m_optimizationEnabled = optimizationEnabled;
}

ThingId ChargingConfiguration::carThingId() const
{
    return m_carThingId;
}

void ChargingConfiguration::setCarThingId(const ThingId &carThingId)
{
    m_carThingId = carThingId;
}

QTime ChargingConfiguration::endTime() const
{
    return m_endTime;
}

void ChargingConfiguration::setEndTime(const QTime &endTime)
{
    m_endTime = endTime;
}

uint ChargingConfiguration::targetPercentage() const
{
    return m_targetPercentage;
}

void ChargingConfiguration::setTargetPercentage(uint targetPercentage)
{
    m_targetPercentage = targetPercentage;
}

bool ChargingConfiguration::zeroReturnPolicyEnabled() const
{
    return m_zeroReturnPolicyEnabled;
}

void ChargingConfiguration::setZeroReturnPolicyEnabled(bool zeroReturnPolicyEnabled)
{
    m_zeroReturnPolicyEnabled = zeroReturnPolicyEnabled;
}

bool ChargingConfiguration::operator==(const ChargingConfiguration &other) const
{
    return m_evChargerThingId == other.evChargerThingId() &&
            m_optimizationEnabled == other.optimizationEnabled() &&
            m_carThingId == other.carThingId() &&
            m_endTime == other.endTime() &&
            m_targetPercentage == other.targetPercentage() &&
            m_zeroReturnPolicyEnabled == other.zeroReturnPolicyEnabled();
}

bool ChargingConfiguration::operator!=(const ChargingConfiguration &other) const
{
    return !(*this == other);
}