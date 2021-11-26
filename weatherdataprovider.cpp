#include "weatherdataprovider.h"

#include <QUrlQuery>
#include <QNetworkReply>
#include <QJsonDocument>

#include "loggingcategories.h"
Q_DECLARE_LOGGING_CATEGORY(dcConsolinnoEnergy)

WeatherDataProvider::WeatherDataProvider(QNetworkAccessManager *networkManager, QObject *parent) :
    QObject(parent),
    m_networkManager(networkManager)
{

}

QString WeatherDataProvider::locationId() const
{
    return m_locationId;
}

void WeatherDataProvider::setLocationId(const QString &locationId)
{
    m_locationId = locationId;
}

QString WeatherDataProvider::apiKey() const
{
    return m_apiKey;
}

void WeatherDataProvider::setApiKey(const QString &apiKey)
{
    m_apiKey = apiKey;
}

bool WeatherDataProvider::available() const
{
    return !m_locationId.isEmpty() && !m_apiKey.isEmpty();
}

WeatherDataEntries WeatherDataProvider::weatherData() const
{
    return m_entries;
}

WeatherDataEntries WeatherDataProvider::getHistoryEntries(const QDateTime &currentDateTime, int hours)
{
    WeatherDataEntries result;

    // Get the index of the current date time
    int currentIndex = 0;
    for (int i = 0; i < m_entries.count(); i++) {
        if (m_entries.at(i).timestamp().toUTC() == currentDateTime.toUTC()) {
            currentIndex = i;
            break;
        }
    }

    // Make sure there are 24 hours available
    if (currentIndex < hours) {
        qCWarning(dcConsolinnoEnergy()) << "Not enought historical data available for timestamp" << currentDateTime.toString() << "hours:" << hours << "Current index" << currentIndex;
        return result;
    }

    for (int i = currentIndex; i > currentIndex - hours; i--) {
        result.prepend(m_entries.at(i));
    }

    Q_ASSERT_X(result.count() == hours, "WeatherDataProvider", QString("Requested data for %1 hours but the result list has %2 entries.").arg(hours).arg(result.count()).toUtf8());

    return result;
}

WeatherDataEntries WeatherDataProvider::getForecastEntries(const QDateTime &currentDateTime, int hours)
{
    WeatherDataEntries result;

    // Get the index of the current date time
    int currentIndex = 0;
    for (int i = 0; i < m_entries.count(); i++) {
        if (m_entries.at(i).timestamp().toUTC() == currentDateTime.toUTC()) {
            currentIndex = i;
            break;
        }
    }

    // Make sure there are 24 hours available
    if (m_entries.count() - currentIndex < hours) {
        qCWarning(dcConsolinnoEnergy()) << "Not enought forecast data available for timestamp" << currentDateTime.toString() << "hours:" << hours << "Current index" << currentIndex;
        return result;
    }

    for (int i = currentIndex; i < currentIndex + hours; i++) {
        result.prepend(m_entries.at(i));
    }

    Q_ASSERT_X(result.count() == hours, "WeatherDataProvider", QString("Requested data for %1 hours but the result list has %2 entries.").arg(hours).arg(result.count()).toUtf8());

    return result;
}

bool WeatherDataProvider::updateRequired() const
{
    if (m_entries.count() < 2)
        return true;

    QDateTime currentDateTimeUtc = QDateTime::currentDateTimeUtc();
    return (m_entries.first().timestamp() < currentDateTimeUtc.addDays(-1) && m_entries.last().timestamp() >= currentDateTimeUtc.addDays(1));
}


bool WeatherDataProvider::updateWeatherInformation()
{
    // If we already have enougth data, we don't need to fetch data
    if (!updateRequired())
        return true;

    if (!available()) {
        emit weatherDataUpdateFailed();
        return false;
    }

    if (m_updating)
        return true;

    setUpdating(true);

    qCDebug(dcConsolinnoEnergy()) << "Updating weather data...";
    QNetworkReply *weatherReply = getWeather();
    connect(weatherReply, &QNetworkReply::finished, weatherReply, &QNetworkReply::deleteLater);
    connect(weatherReply, &QNetworkReply::finished, this, [=](){
        if (weatherReply->error() != QNetworkReply::NoError) {
            qCWarning(dcConsolinnoEnergy()) << "WeatherDataProvider: Failed to get weather data. The reply returned with error" << weatherReply->attribute(QNetworkRequest::HttpStatusCodeAttribute) << weatherReply->errorString();
            emit weatherDataUpdateFailed();
            setUpdating(false);
            return;
        }

        QByteArray data = weatherReply->readAll();
        QJsonParseError error;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
        if (error.error != QJsonParseError::NoError) {
            qCWarning(dcConsolinnoEnergy()) << "WeatherDataProvider: Failed to parse weather data" << data << ":" << error.errorString();
            emit weatherDataUpdateFailed();
            setUpdating(false);
            return;
        }

        qCDebug(dcConsolinnoEnergy()) << "WeatherDataProvider: Request weather data finished successfully";
        //qCDebug(dcConsolinnoEnergy()) << "<--" << qUtf8Printable(jsonDoc.toJson(QJsonDocument::Indented));

        QVariantMap dataMap = jsonDoc.toVariant().toMap();
        if (!dataMap.contains("coord")) {
            qCWarning(dcConsolinnoEnergy()) << "WeatherDataProvider: Weather data does not provide coordinats for location" << m_locationId << ". Cannot continue because the forecast requires a coordinates.";
            emit weatherDataUpdateFailed();
            setUpdating(false);
            return;
        }

        // Parse current weather location information
        m_locationName = dataMap.value("name").toString();
        m_longitude = dataMap.value("coord").toMap().value("lon").toDouble();
        m_latitude = dataMap.value("coord").toMap().value("lat").toDouble();
        m_timestampSunrise = QDateTime::fromMSecsSinceEpoch(dataMap.value("sys").toMap().value("sunrise").toLongLong() * 1000);
        m_timestampSunset = QDateTime::fromMSecsSinceEpoch(dataMap.value("sys").toMap().value("sunset").toLongLong() * 1000);

        qCDebug(dcConsolinnoEnergy()) << "WeatherDataProvider: Location:" << m_locationName << m_longitude << "," << m_latitude;
        qCDebug(dcConsolinnoEnergy()) << "WeatherDataProvider: Sunrise:" << m_timestampSunrise.toString();
        qCDebug(dcConsolinnoEnergy()) << "WeatherDataProvider: Sunset:" << m_timestampSunset.toString();

        // Get yesterdays history
        QNetworkReply *historyReply = getWeatherHistory(-1);
        connect(historyReply, &QNetworkReply::finished, historyReply, &QNetworkReply::deleteLater);
        connect(historyReply, &QNetworkReply::finished, this, [=](){
            if (historyReply->error() != QNetworkReply::NoError) {
                qCWarning(dcConsolinnoEnergy()) << "WeatherDataProvider: Failed to get weather history. The reply returned with error" << historyReply->attribute(QNetworkRequest::HttpStatusCodeAttribute) << historyReply->errorString();
                emit weatherDataUpdateFailed();
                setUpdating(false);
                return;
            }

            QByteArray data = historyReply->readAll();
            QJsonParseError error;
            QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
            if (error.error != QJsonParseError::NoError) {
                qCWarning(dcConsolinnoEnergy()) << "WeatherDataProvider: Failed to parse weather history data" << data << ":" << error.errorString();
                emit weatherDataUpdateFailed();
                setUpdating(false);
                return;
            }

            qCDebug(dcConsolinnoEnergy()) << "WeatherDataProvider: Request weather history finished successfully";
            //qCDebug(dcConsolinnoEnergy()) << "<--" << qUtf8Printable(jsonDoc.toJson(QJsonDocument::Indented));

            QVariantMap dataMap = jsonDoc.toVariant().toMap();

            // Valid request, let's wipe the current data
            m_history.clear();

            // Hourly forecast
            QVariantList hourlyForecastList = dataMap.value("hourly").toList();
            foreach (const QVariant &hourlyForecastVariant, hourlyForecastList)
                m_history.append(parseHourlyEntry(hourlyForecastVariant.toMap()));

            m_history.sort();

            // Get todays history
            QNetworkReply *historyReply = getWeatherHistory(0);
            connect(historyReply, &QNetworkReply::finished, historyReply, &QNetworkReply::deleteLater);
            connect(historyReply, &QNetworkReply::finished, this, [=](){
                if (historyReply->error() != QNetworkReply::NoError) {
                    qCWarning(dcConsolinnoEnergy()) << "WeatherDataProvider: Failed to get weather history. The reply returned with error" << historyReply->attribute(QNetworkRequest::HttpStatusCodeAttribute) << historyReply->errorString();
                    emit weatherDataUpdateFailed();
                    setUpdating(false);
                    return;
                }

                QByteArray data = historyReply->readAll();
                QJsonParseError error;
                QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
                if (error.error != QJsonParseError::NoError) {
                    qCWarning(dcConsolinnoEnergy()) << "WeatherDataProvider: Failed to parse weather history data" << data << ":" << error.errorString();
                    emit weatherDataUpdateFailed();
                    setUpdating(false);
                    return;
                }

                qCDebug(dcConsolinnoEnergy()) << "WeatherDataProvider: Request weather history finished successfully";

                QVariantMap dataMap = jsonDoc.toVariant().toMap();
                //qCDebug(dcConsolinnoEnergy()) << "<--" << qUtf8Printable(jsonDoc.toJson(QJsonDocument::Indented));

                // Hourly forecast
                QVariantList hourlyForecastList = dataMap.value("hourly").toList();
                foreach (const QVariant &hourlyForecastVariant, hourlyForecastList)
                    m_history.append(parseHourlyEntry(hourlyForecastVariant.toMap()));

                m_history.sort();

                QNetworkReply *forecastReply = getWeatherForecast();
                connect(forecastReply, &QNetworkReply::finished, forecastReply, &QNetworkReply::deleteLater);
                connect(forecastReply, &QNetworkReply::finished, this, [=](){
                    if (forecastReply->error() != QNetworkReply::NoError) {
                        qCWarning(dcConsolinnoEnergy()) << "WeatherDataProvider: Failed to get weather forecast. The reply returned with error" << forecastReply->attribute(QNetworkRequest::HttpStatusCodeAttribute) << forecastReply->errorString();
                        emit weatherDataUpdateFailed();
                        setUpdating(false);
                        return;
                    }

                    QByteArray data = forecastReply->readAll();
                    QJsonParseError error;
                    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
                    if (error.error != QJsonParseError::NoError) {
                        qCWarning(dcConsolinnoEnergy()) << "WeatherDataProvider: Failed to parse weather forecast data" << data << ":" << error.errorString();
                        emit weatherDataUpdateFailed();
                        setUpdating(false);
                        return;
                    }

                    qCDebug(dcConsolinnoEnergy()) << "WeatherDataProvider: Request weather forecast finished successfully";
                    //qCDebug(dcConsolinnoEnergy()) << "<--" << qUtf8Printable(jsonDoc.toJson(QJsonDocument::Indented));

                    QVariantMap dataMap = jsonDoc.toVariant().toMap();

                    // Valid request, let's wipe the current data
                    m_forcast.clear();

                    // Update current weather value
                    QVariantMap currentMap = dataMap.value("current").toMap();
                    m_currentWeatherEntry.setTimeStamp(QDateTime::fromMSecsSinceEpoch(currentMap.value("dt").toLongLong() * 1000));
                    m_currentWeatherEntry.setTemperature(currentMap.value("temp").toDouble());
                    qCDebug(dcConsolinnoEnergy()) << "WeatherDataProvider: current" << m_currentWeatherEntry;

                    // Hourly forecast
                    QVariantList hourlyForecastList = dataMap.value("hourly").toList();
                    foreach (const QVariant &hourlyForecastVariant, hourlyForecastList)
                        m_forcast.append(parseHourlyEntry(hourlyForecastVariant.toMap()));

                    m_forcast.sort();

                    // Update the entries
                    updateEntries();

                    setUpdating(false);
                });
            });
        });
    });

    return true;
}

void WeatherDataProvider::setUpdating(bool updating)
{
    if (m_updating == updating)
        return;

    m_updating = updating;
    emit updatingChanged(m_updating);
}

QNetworkReply *WeatherDataProvider::getWeather()
{
    // https://openweathermap.org/current#one
    qCDebug(dcConsolinnoEnergy()) << "WeatherDataProvider: Updating current weather data";
    QUrlQuery query;
    query.addQueryItem("id", m_locationId);
    query.addQueryItem("mode", "json");
    query.addQueryItem("units", "metric");
    query.addQueryItem("appid", m_apiKey);

    QUrl requestUrl = m_apiBaseUrl;
    requestUrl.setPath("/data/2.5/weather");
    requestUrl.setQuery(query);

    qCDebug(dcConsolinnoEnergy()) << "WeatherDataProvider:" << requestUrl.toString();
    return m_networkManager->get(QNetworkRequest(requestUrl));
}

QNetworkReply *WeatherDataProvider::getWeatherForecast()
{
    qCDebug(dcConsolinnoEnergy()) << "WeatherDataProvider: Updating weather forecast data";

    QUrlQuery query;
    query.addQueryItem("mode", "json");
    query.addQueryItem("units", "metric");
    query.addQueryItem("appid", m_apiKey);
    query.addQueryItem("lat", QString::number(m_latitude));
    query.addQueryItem("lon", QString::number(m_longitude));

    QUrl requestUrl = m_apiBaseUrl;
    requestUrl.setPath("/data/2.5/onecall");
    requestUrl.setQuery(query);

    return m_networkManager->get(QNetworkRequest(requestUrl));
}

QNetworkReply *WeatherDataProvider::getWeatherHistory(int dayOffset)
{
    // https://openweathermap.org/api/one-call-api#history
    QDateTime dateTimeOfHistoryDay = QDateTime::currentDateTime().addDays(dayOffset);
    qCDebug(dcConsolinnoEnergy()) << "WeatherDataProvider: Updating weather history data from" << dateTimeOfHistoryDay.date().toString();

    QUrlQuery query;
    query.addQueryItem("dt", QString::number(static_cast<qint64>(QDateTime::currentDateTime().addDays(dayOffset).toMSecsSinceEpoch() / 1000)));
    query.addQueryItem("lat", QString::number(m_latitude));
    query.addQueryItem("lon", QString::number(m_longitude));
    query.addQueryItem("units", "metric");
    query.addQueryItem("appid", m_apiKey);

    QUrl requestUrl = m_apiBaseUrl;
    requestUrl.setPath("/data/2.5/onecall/timemachine");
    requestUrl.setQuery(query);

    return m_networkManager->get(QNetworkRequest(requestUrl));
}

WeatherDataEntry WeatherDataProvider::parseHourlyEntry(const QVariantMap &hourlyMap)
{
    WeatherDataEntry entry;
    entry.setTimeStamp(QDateTime::fromMSecsSinceEpoch(hourlyMap.value("dt").toLongLong() * 1000));
    entry.setTemperature(hourlyMap.value("temp").toDouble());
    return entry;
}

void WeatherDataProvider::updateEntries()
{
    m_entries.clear();

    // Add the history data
    qCDebug(dcConsolinnoEnergy()) << "WeatherDataProvider: Process history data...";
    for (int i = 0; i < m_history.count(); i++) {
        if (i == 0) {
            m_entries.append(m_history.at(i));
        } else {
            if (m_entries.last().timestamp() != m_history.at(i).timestamp()) {
                Q_ASSERT_X(m_entries.last().timestamp() == m_history.at(i).timestamp().addSecs(-3600), "WeatherData", "The previous weater data entry is not exactly one hour ago.");
                m_entries.append(m_history.at(i));
            }
        }
    }

    // Add the forecast
    qCDebug(dcConsolinnoEnergy()) << "WeatherDataProvider: Process forecast data...";
    for (int i = 0; i < m_forcast.count(); i++) {
        if (m_entries.last().timestamp() != m_forcast.at(i).timestamp()) {
            Q_ASSERT_X(m_entries.last().timestamp() == m_forcast.at(i).timestamp().addSecs(-3600), "WeatherData", "The previous weater data entry is not exactly one hour ago.");
            m_entries.append(m_forcast.at(i));
        }
    }

    qCDebug(dcConsolinnoEnergy()) << "WeatherDataProvider: Weather data updated and available from" << m_entries.first() << m_entries.last();
    emit weatherDataUpdated();
}
