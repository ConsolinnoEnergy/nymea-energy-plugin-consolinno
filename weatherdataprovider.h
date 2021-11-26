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
    WeatherDataEntries m_forcast;

    WeatherDataEntries m_entries;

    WeatherDataEntry parseHourlyEntry(const QVariantMap &hourlyMap);

    void updateEntries();
};

#endif // WEATHERDATAPROVIDER_H
