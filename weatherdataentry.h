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

#ifndef WEATHERDATAENTRY_H
#define WEATHERDATAENTRY_H

#include <QDebug>
#include <QObject>
#include <QDateTime>

class WeatherDataEntry
{
public:
    explicit WeatherDataEntry();
    WeatherDataEntry(const QDateTime &timestamp, double temperature);

    QDateTime timestamp() const;
    void setTimeStamp(const QDateTime &timestamp);

    double temperature() const;
    void setTemperature(double temperature);

private:
    QDateTime m_timestamp;
    double m_temperature = 0;

};

class WeatherDataEntries: public QList<WeatherDataEntry>
{
public:
    WeatherDataEntries() = default;

    inline void sort() {
        std::sort(begin(), end(), [](const WeatherDataEntry &a, const WeatherDataEntry &b) {
            return a.timestamp() < b.timestamp();
        });
    }
};

QDebug operator<<(QDebug debug, const WeatherDataEntry &weatherDataEntry);
QDebug operator<<(QDebug debug, const WeatherDataEntries &weatherDataEntries);


#endif // WEATHERDATAENTRY_H
