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
