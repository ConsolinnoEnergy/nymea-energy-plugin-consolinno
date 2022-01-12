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

#ifndef HEMSOPTIMIZERENGINE_H
#define HEMSOPTIMIZERENGINE_H

#include <QObject>
#include <energymanager.h>

#include "weatherdataprovider.h"
#include "hemsoptimizerinterface.h"
#include "configurations/heatingconfiguration.h"
#include "configurations/chargingconfiguration.h"

#include <loggingcategories.h>
Q_DECLARE_LOGGING_CATEGORY(dcHemsOptimizer)
Q_DECLARE_LOGGING_CATEGORY(dcHemsOptimizerTraffic)

class HemsOptimizerEngine : public QObject
{
    Q_OBJECT
public:
    enum State {
        StateIdle,
        StateGetWeatherData,
        StateGetElectricDemandForecast,
        StateGetHeatingForecast,
        StateGetPvOptimization
    };
    Q_ENUM(State)

    typedef struct ChargingSchedule {
        ChargingConfiguration chargingConfiguration;
        double maxPower; // W
        double minPower; // W
        double energyNeeded; // Wh
    } ChargingSchedule;

    explicit HemsOptimizerEngine(EnergyManager *energyManager, WeatherDataProvider *weatherDataProvider, QNetworkAccessManager *networkManager, QObject *parent = nullptr);

    HemsOptimizerInterface *interface() const;

    State state() const;

    void updatePvOptimizationSchedule(const HeatingConfiguration &heatingConfiguration, const ChargingSchedule &chargingSchedule);

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
    HemsOptimizerSchedules m_electricDemandStandardProfileForecast;
    HeatingConfiguration m_heatingConfiguration;
    ChargingSchedule m_chargingSchedule;

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

    void getHeatingPowerForecast(HemsOptimizerInterface::HouseType houseType, double livingArea);
    void getElectricDemandStandardProfileForecast(double annualDemand);

    void getPvOptimizationSchedule();

};

QDebug operator<<(QDebug debug, const HemsOptimizerEngine::ChargingSchedule &schedule);


#endif // HEMSOPTIMIZERENGINE_H
