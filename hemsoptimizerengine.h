#ifndef HEMSOPTIMIZERENGINE_H
#define HEMSOPTIMIZERENGINE_H

#include <QObject>
#include <energymanager.h>

#include "weatherdataprovider.h"
#include "hemsoptimizerinterface.h"

class HemsOptimizerEngine : public QObject
{
    Q_OBJECT
public:
    enum State {
        StateIdle,
        StateGetWeatherData,
        StateGetHeatingForecast,
        StateGetPvOptimization
    };
    Q_ENUM(State)

    explicit HemsOptimizerEngine(EnergyManager *energyManager, WeatherDataProvider *weatherDataProvider, QNetworkAccessManager *networkManager, QObject *parent = nullptr);

    HemsOptimizerInterface *interface() const;

    State state() const;

    void updatePvOptimizationSchedule();

    double housholdPowerLimit() const;
    void setHousholdPowerLimit(double housholdLimit);

signals:
    void stateChanged(HemsOptimizerEngine::State state);

private:
    EnergyManager *m_energyManager = nullptr;
    WeatherDataProvider *m_weatherDataProvider = nullptr;
    HemsOptimizerInterface *m_interface = nullptr;

    // Private members while the engine is working
    State m_state = StateIdle;
    QDateTime m_currentDateTimeUtc;
    QVector<QDateTime> m_timestamps;
    PowerBalanceLogEntries m_powerBalanceHistory;
    HemsOptimizerSchedules m_floorHeatingPowerForecast;

    // Settings
    uint m_scheduleWindowHours = 24;
    double m_housholdPowerLimit;

    void setState(State state);

    // Optimizer information
    QVector<QDateTime> generateScheduleTimeStamps(const QDateTime &nowUtc, uint resolutionMinutes, uint durationHours);

    QVariantList getConsumptionForecast(const PowerBalanceLogEntries &powerBalanceHistory);
    QVariantList getPvForecast(const PowerBalanceLogEntries &powerBalanceHistory);
    QVariantList getThermalDemandForecast(const QVector<QDateTime> &timestamps, const HemsOptimizerSchedules &floorHeatingPowerForecast);
    QVariantList getCopForecast(const QVector<QDateTime> &timestamps, double staticCopValue = 3.5);

    QVariantMap getTemperatureHistory(const QDateTime &nowUtc);
    QVariantMap getTemperatureForecast(const QDateTime &nowUtc);

    HemsOptimizerSchedules interpolateValues(const QVector<QDateTime> &desiredTimestamps, const HemsOptimizerSchedules &sourceSchedules);

    void getHeatingPowerForecast();
    void getPvOptimizationSchedule();

};

#endif // HEMSOPTIMIZERENGINE_H
