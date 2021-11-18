#ifndef HEMSOPTIMIZERENGINE_H
#define HEMSOPTIMIZERENGINE_H

#include <QObject>
#include <energymanager.h>

#include "hemsoptimizerinterface.h"

class HemsOptimizerEngine : public QObject
{
    Q_OBJECT
public:
    explicit HemsOptimizerEngine(EnergyManager *energyManager, QObject *parent = nullptr);

    HemsOptimizerInterface *interface() const;

    void updatePvOptimizationSchedule();

    double housholdPowerLimit() const;
    void setHousholdPowerLimit(double housholdLimit);

signals:

private:
    EnergyManager *m_energyManager = nullptr;
    HemsOptimizerInterface *m_interface = nullptr;
    uint m_scheduleWindowHours = 24;

    double m_housholdPowerLimit;

    // Optimizer information
    QVector<QDateTime> generateScheduleTimeStamps(const QDateTime &currentDateTime, uint resolutionMinutes, uint durationHours);

    QVariantList getConsumptionForecast(const PowerBalanceLogEntries &powerBalanceHistory);
    QVariantList getPvForecast(const PowerBalanceLogEntries &powerBalanceHistory);
    QVariantList getThermalDemandForecast(const QVector<QDateTime> &timestamps, const HemsOptimizerSchedules &floorHeatingPowerForecast);
    QVariantList getCopForecast(const QVector<QDateTime> &timestamps, double staticCopValue = 3.5);

    QVariantMap getTemperatureHistory(const QDateTime &now);
    QVariantMap getTemperatureForecast(const QDateTime &now);

    HemsOptimizerSchedules interpolateValues(const QVector<QDateTime> &desiredTimestamps, const HemsOptimizerSchedules &sourceSchedules);

};

#endif // HEMSOPTIMIZERENGINE_H
