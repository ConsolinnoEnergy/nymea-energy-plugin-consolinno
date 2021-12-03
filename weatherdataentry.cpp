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

#include "weatherdataentry.h"

WeatherDataEntry::WeatherDataEntry()
{

}

WeatherDataEntry::WeatherDataEntry(const QDateTime &timestamp, double temperature) :
    m_timestamp(timestamp),
    m_temperature(temperature)
{

}

QDateTime WeatherDataEntry::timestamp() const
{
    return m_timestamp;
}

void WeatherDataEntry::setTimeStamp(const QDateTime &timestamp)
{
    m_timestamp = timestamp;
}

double WeatherDataEntry::temperature() const
{
    return m_temperature;
}

void WeatherDataEntry::setTemperature(double temperature)
{
    m_temperature = temperature;
}

QDebug operator<<(QDebug debug, const WeatherDataEntry &weatherDataEntry)
{
    debug.nospace() << "WeatherDataEntry(" << weatherDataEntry.timestamp().toString();
    debug.nospace() << ", " << weatherDataEntry.temperature() << " °C";
    debug.nospace() << ")";
    return debug.maybeSpace();
}

QDebug operator<<(QDebug debug, const WeatherDataEntries &weatherDataEntries)
{
    debug.nospace() << "Weather data: count" << weatherDataEntries.count() << "\n";
    foreach (const WeatherDataEntry &entry, weatherDataEntries)
        debug << entry << "\n";

    return debug.maybeSpace();
}
