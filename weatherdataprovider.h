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

#ifndef WEATHERDATAPROVIDER_H
#define WEATHERDATAPROVIDER_H

#include <QObject>
#include <QNetworkAccessManager>

#include "weatherdataentry.h"

class WeatherDataProvider : public QObject
{
    Q_OBJECT
public:
    explicit WeatherDataProvider(QNetworkAccessManager *networkManager, QObject *parent = nullptr);

    QString locationId() const;
    void setLocationId(const QString &locationId);

    QString apiKey() const;
    void setApiKey(const QString &apiKey);

    bool available() const;
    bool updating() const;

    WeatherDataEntries weatherData() const;

    WeatherDataEntries getHistoryEntries(const QDateTime &currentDateTime, int hours);
    WeatherDataEntries getForecastEntries(const QDateTime &currentDateTime, int hours);


    // Returns true if we have weather data 24h into the past and 24h into the future
    bool updateRequired() const;

public slots:
    bool updateWeatherInformation();

signals:
    void updatingChanged(bool updating);
    void weatherDataUpdated();
    void weatherDataUpdateFailed();

private:
    QNetworkAccessManager *m_networkManager = nullptr;

    QUrl m_apiBaseUrl = QUrl("https://api.openweathermap.org");

    QString m_locationId;
    QString m_apiKey;

    bool m_updating = false;
    void setUpdating(bool updating);

    // Weather location information
    QString m_locationName;
    double m_longitude = 0;
    double m_latitude = 0;

    WeatherDataEntry m_currentWeatherEntry;
    QDateTime m_timestampSunrise;
    QDateTime m_timestampSunset;

    QNetworkReply *getWeather();
    QNetworkReply *getWeatherForecast();
    QNetworkReply *getWeatherHistory(int dayOffset);

    WeatherDataEntries m_history;
    WeatherDataEntries m_forecast;

    WeatherDataEntries m_entries;

    WeatherDataEntry parseHourlyEntry(const QVariantMap &hourlyMap);

    void updateEntries();
};

#endif // WEATHERDATAPROVIDER_H
