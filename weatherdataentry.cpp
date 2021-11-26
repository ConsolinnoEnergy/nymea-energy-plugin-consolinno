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
