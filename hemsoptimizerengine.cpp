#include "hemsoptimizerengine.h"

#include <QJsonDocument>
#include <QNetworkReply>

#include "loggingcategories.h"
Q_DECLARE_LOGGING_CATEGORY(dcConsolinnoEnergy)

HemsOptimizerEngine::HemsOptimizerEngine(EnergyManager *energyManager, WeatherDataProvider *weatherDataProvider, QNetworkAccessManager *networkManager, QObject *parent) :
    QObject(parent),
    m_energyManager(energyManager),
    m_weatherDataProvider(weatherDataProvider),
    m_interface(new HemsOptimizerInterface(networkManager, this))
{

    connect(m_weatherDataProvider, &WeatherDataProvider::weatherDataUpdated, this, [=](){
        if (m_state == StateGetWeatherData) {
            qCDebug(dcConsolinnoEnergy()) << "HemsOptimizer: The weather data have been fetched successfully.";
            setState(StateGetHeatingForecast);
        }
    });

    connect(m_weatherDataProvider, &WeatherDataProvider::weatherDataUpdateFailed, this, [=](){
        if (m_state == StateGetWeatherData) {
            qCWarning(dcConsolinnoEnergy()) << "HemsOptimizer: Failed to fetch weather data. Cannot continue to fetch optimization schedules.";
            setState(StateIdle);
        }
    });
}

HemsOptimizerInterface *HemsOptimizerEngine::interface() const
{
    return m_interface;
}

HemsOptimizerEngine::State HemsOptimizerEngine::state() const
{
    return m_state;
}

void HemsOptimizerEngine::updatePvOptimizationSchedule()
{
    if (m_state != StateIdle) {
        qCWarning(dcConsolinnoEnergy()) << "HemsOptimizer: Could not update PV optimization schedule. The optimizer engine is currently busy.";
        return;
    }

    // We use this time as refference for the entire optimization data snapshot
    qCDebug(dcConsolinnoEnergy()) << "HemsOptimizer: --> Update schedules for the next" << m_scheduleWindowHours << "hours on 15 minutes resolution...";
    m_currentDateTimeUtc = QDateTime::currentDateTimeUtc();

    // Note: everything communicated from and to the server must be UTC, within the system, we work with local time

    // Create timestamps for the next 24 hours in 15 min timeslots
    m_timestamps = generateScheduleTimeStamps(m_currentDateTimeUtc, 15, m_scheduleWindowHours);
    qCDebug(dcConsolinnoEnergy()) << "HemsOptimizer: Created" << m_timestamps.count() <<  "schedule time slots from" << m_timestamps.first().toString("dd.MM.yyyy HH:mm:ss") << "until" << m_timestamps.last().toString("dd.MM.yyyy HH:mm:ss");

    // Get the consumption from the yesterday for this time window
    m_powerBalanceHistory = m_energyManager->logs()->powerBalanceLogs(EnergyLogs::SampleRate15Mins, m_timestamps.first().addDays(-1), m_timestamps.last().addDays(-1));
    if (m_powerBalanceHistory.count() < m_timestamps.count()) {
        qCWarning(dcConsolinnoEnergy()) << "HemsOptimizer: Not enought historical data for optimization. Having" << m_powerBalanceHistory.count() << "historical samples but require at least" << m_timestamps.count() << "sampels for doing optimization.";
        setState(StateIdle);
        return;
    }

    Q_ASSERT_X(m_powerBalanceHistory.count() == m_timestamps.count(), "HemsOptimizer", "Power balance history count does not match timestamp count.");
    //    qCDebug(dcConsolinnoEnergy()) << "Logs form the past yesterday based on 15 min sample rate in this period";
    //    foreach (const PowerBalanceLogEntry &log, powerBalanceHistory) {
    //        qCDebug(dcConsolinnoEnergy()) << log.timestamp().toString("dd.MM.yyyy HH:mm:ss") << "Consumption" << log.consumption() << "W";
    //    }

    // Preparation done, start the state machine for fetching PV schedules
    setState(StateGetWeatherData);
}

double HemsOptimizerEngine::housholdPowerLimit() const
{
    return m_housholdPowerLimit;
}

void HemsOptimizerEngine::setHousholdPowerLimit(double housholdLimit)
{
    m_housholdPowerLimit = housholdLimit;
}

void HemsOptimizerEngine::setState(State state)
{
    if (m_state == state)
        return;

    qCDebug(dcConsolinnoEnergy()) << "HemsOptimizer: The state changed" << state;
    m_state = state;
    emit stateChanged(m_state);

    switch (m_state) {
    case StateIdle:
        // Cleanup to free memory
        m_timestamps.clear();
        m_powerBalanceHistory.clear();
        m_floorHeatingPowerForecast.clear();
        break;
    case StateGetWeatherData:
        if (!m_weatherDataProvider->available()) {
            qCWarning(dcConsolinnoEnergy()) << "HemsOptimizer: The weather data provider is not available. Cannot continue.";
            setState(StateIdle);
            return;
        }

        // Check if we even have to fetch new weather data
        if (m_weatherDataProvider->updateRequired()) {
            m_weatherDataProvider->updateWeatherInformation();
            return;
        }

        // We already have weather data, let's continue with the heating power forecast
        setState(StateGetHeatingForecast);
        break;
    case StateGetHeatingForecast:
        getHeatingPowerForecast();
        break;
    case StateGetPvOptimization:
        getPvOptimizationSchedule();
        break;

    }

}

QVector<QDateTime> HemsOptimizerEngine::generateScheduleTimeStamps(const QDateTime &nowUtc, uint resolutionMinutes, uint durationHours)
{
    qCDebug(dcConsolinnoEnergy()) << "HemsOptimizer: Generate timestamps for schedule in" << resolutionMinutes << "min resolution for the next" << durationHours;
    // Note: forecasts and schedules will be used on 15 min base starting at a full hour
    QDate date = nowUtc.date();
    QTime time = nowUtc.time();
    uint minutesOffsetToNextSlot = (15 - time.minute() % 15);
    time = time.addSecs(minutesOffsetToNextSlot * 60);
    time.setHMS(time.hour(), time.minute(), 0);

    QDateTime scheduleStartDateTime = QDateTime(date, time);
    qCDebug(dcConsolinnoEnergy()) << "HemsOptimizer: Minutes until next slot:" << minutesOffsetToNextSlot << scheduleStartDateTime.toString();

    // get minutes to the next 15 min schedule

    QVector<QDateTime> timestamps;
    // Example: resolution of 10 minutes for the next 12 hours = 72 timestamps
    uint timestampsCount = (durationHours * 60) / resolutionMinutes;
    for (uint i = 0; i < timestampsCount; i++) {
        timestamps << scheduleStartDateTime.addSecs(i * 60 * resolutionMinutes);
    }
    return timestamps;
}

QVariantList HemsOptimizerEngine::getConsumptionForecast(const PowerBalanceLogEntries &powerBalanceHistory)
{
    // Get the consumption from the yesterday for this time window
    // TODO: for now use the values from yesterday during this window as forecast
    QVariantList forecast;
    for (int i = 0; i < powerBalanceHistory.count(); i++) {
        forecast << powerBalanceHistory.at(i).consumption() / 1000.0; // kW
    }
    return forecast;
}

QVariantList HemsOptimizerEngine::getPvForecast(const PowerBalanceLogEntries &powerBalanceHistory)
{
    // Get the consumption from the yesterday for this time window
    // Forcast values must be positive from the optimizer perspective
    // TODO: for now use the values from yesterday during this window as forecast
    QVariantList forecast;
    for (int i = 0; i < powerBalanceHistory.count(); i++) {
        forecast << abs(powerBalanceHistory.at(i).production() / 1000.0); // kW
    }
    return forecast;
}

QVariantList HemsOptimizerEngine::getThermalDemandForecast(const QVector<QDateTime> &timestamps, const HemsOptimizerSchedules &floorHeatingPowerForecast)
{
    HemsOptimizerSchedules interpolatedForecast = interpolateValues(timestamps, floorHeatingPowerForecast);
    QVariantList thermalDemandForecast;
    for (int i = 0; i < interpolatedForecast.count(); i++) {
        thermalDemandForecast << interpolatedForecast.at(i).value(); // kW
    }
    return thermalDemandForecast;
}

QVariantList HemsOptimizerEngine::getCopForecast(const QVector<QDateTime> &timestamps, double staticCopValue)
{
    QVariantList copForecast;
    for (int i = 0; i < timestamps.count(); i++) {
        copForecast << staticCopValue;
    }
    return copForecast;
}

QVariantMap HemsOptimizerEngine::getTemperatureHistory(const QDateTime &nowUtc)
{
    // Create a list of hourly 24h outdoor temperature history
    QDateTime currentDateTimeRounded = nowUtc;
    currentDateTimeRounded.setTime(QTime(nowUtc.time().hour(), 0, 0));

    WeatherDataEntries entries = m_weatherDataProvider->getHistoryEntries(currentDateTimeRounded, 24);
    entries.sort();
    QVariantMap history;
    for (int i = 0; i < entries.count(); i++) {
        history.insert(entries.at(i).timestamp().toUTC().toString(Qt::ISODate), entries.at(i).temperature());
    }

    return history;
}

QVariantMap HemsOptimizerEngine::getTemperatureForecast(const QDateTime &nowUtc)
{
    // Create a list of hourly 24h outdoor temperature history
    // TODO: fetch from weather plugin
    QDateTime currentDateTimeRounded = nowUtc;
    currentDateTimeRounded.setTime(QTime(nowUtc.time().hour(), 0, 0));

    WeatherDataEntries entries = m_weatherDataProvider->getForecastEntries(currentDateTimeRounded, 24);
    entries.sort();

    QVariantMap forecast;
    for (int i = 0; i < entries.count(); i++) {
        forecast.insert(entries.at(i).timestamp().toUTC().toString(Qt::ISODate), entries.at(i).temperature());
    }

    return forecast;
}

HemsOptimizerSchedules HemsOptimizerEngine::interpolateValues(const QVector<QDateTime> &desiredTimestamps, const HemsOptimizerSchedules &sourceSchedules)
{
    int sourceScheduleIndex = 0;
    QDateTime targetScheduleStart;
    QDateTime targetScheduleEnd;

    HemsOptimizerSchedules outputSchedules;
    for (int i = 0; i < desiredTimestamps.count(); i++) {
        // Get the next time window
        targetScheduleStart = desiredTimestamps.at(i);
        if (i < desiredTimestamps.count() - 1) {
            targetScheduleEnd = desiredTimestamps.at(i + 1);
        } else {
            // Last value, we are done
            targetScheduleEnd = desiredTimestamps.at(i);
        }

        // Get the matching source index
        for (int j = sourceScheduleIndex; j < sourceSchedules.count(); j++) {
            // Check if the current source is still in range
            if (targetScheduleStart >= sourceSchedules.at(j).timestamp()) {
                sourceScheduleIndex++;

                // In case we run out of souce values for the next schedule, use the last one
                if (sourceScheduleIndex >= sourceSchedules.count()) {
                    sourceScheduleIndex = sourceSchedules.count() - 1;
                }
            }
        }

        //        qCDebug(dcConsolinnoEnergy()) << "HemsOptimizer: -------------------";
        //        qCDebug(dcConsolinnoEnergy()) << "HemsOptimizer: Target from" << targetScheduleStart.toString("hh:mm.ss") << "to" << targetScheduleEnd.toString("hh:mm.ss");
        //        qCDebug(dcConsolinnoEnergy()) << "HemsOptimizer: " << sourceScheduleIndex << "Current schedule" << sourceSchedules.at(sourceScheduleIndex);
        outputSchedules << HemsOptimizerSchedule(targetScheduleStart, sourceSchedules.at(sourceScheduleIndex).value());
    }

    Q_ASSERT_X(outputSchedules.count() == desiredTimestamps.count(), "HemsOptimizer", "The interpolated schedules count does not match the desired schedule count");
    return outputSchedules;
}

void HemsOptimizerEngine::getHeatingPowerForecast()
{
    QNetworkReply *reply = m_interface->florHeatingPowerDemandStandardProfile(HemsOptimizerInterface::HouseTypeLowEnergy, 100, getTemperatureHistory(m_currentDateTimeUtc), getTemperatureForecast(m_currentDateTimeUtc));
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
    connect(reply, &QNetworkReply::finished, reply, [=](){
        if (reply->error() != QNetworkReply::NoError) {
            qCWarning(dcConsolinnoEnergy()) << "HemsOptimizer: Failed to get electrical demand standard profile. The reply returned with error" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) << reply->errorString();
            QByteArray responsedata = reply->readAll();
            qCWarning(dcConsolinnoEnergy()) << qUtf8Printable(responsedata);
            setState(StateIdle);
            return;
        }

        QByteArray data = reply->readAll();
        QJsonParseError error;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
        if (error.error != QJsonParseError::NoError) {
            qCWarning(dcConsolinnoEnergy()) << "HemsOptimizer: Failed to parse electrical demand standard profile data" << data << ":" << error.errorString();
            setState(StateIdle);
            return;
        }

        qCDebug(dcConsolinnoEnergy()) << "HemsOptimizer: Request electrical demand standard profile finished successfully";
        qCDebug(dcConsolinnoEnergy()) << "<--" << qUtf8Printable(jsonDoc.toJson(QJsonDocument::Indented));

        // Read the schedules
        m_floorHeatingPowerForecast = m_interface->parseSchedules(jsonDoc.toVariant().toMap());
        //        foreach (const HemsOptimizerSchedule &schedule, m_floorHeatingPowerForecast) {
        //            qCDebug(dcConsolinnoEnergy()) << schedule;
        //        }

        // We have the heating power forecast, let's continue with the heat pump optimization schedule update
        setState(StateGetPvOptimization);
    });

}

void HemsOptimizerEngine::getPvOptimizationSchedule()
{
    // Root meter
    QVariantMap ntpMap = m_interface->buildRootMeterInformation(m_timestamps, m_energyManager->rootMeter(), m_housholdPowerLimit, 0.3);

    // Inverter
    QVariantList productionForecast = getPvForecast(m_powerBalanceHistory);
    QVariantMap pvMap = m_interface->buildPhotovoltaicInformation(m_timestamps, productionForecast);

    // Electrical demand
    QVariantList consumptionForecast = getConsumptionForecast(m_powerBalanceHistory);
    QVariantMap electricalDemandMap = m_interface->buildElectricDemandInformation(m_timestamps, QUuid::createUuid(), consumptionForecast);

    // Heat pump
    QVariantList thermalDemandForcast = getThermalDemandForecast(m_timestamps, m_floorHeatingPowerForecast);
    QVariantList copForecast = getCopForecast(m_timestamps, 3.5);

    // Heatpump has a maximum of 9 kW
    double maxElectricalPower = 9;
    // We assume a maximal themral energy of 4kWh
    double maxThermalEnergy = 4;

    QVariantMap heatPumpMap = m_interface->buildHeatpumpInformation(m_timestamps, QUuid::createUuid(), maxThermalEnergy, 0.5, maxElectricalPower, thermalDemandForcast, copForecast, 0.1);

    QNetworkReply *reply = m_interface->pvOptimization(ntpMap, pvMap, electricalDemandMap, heatPumpMap);
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
    connect(reply, &QNetworkReply::finished, reply, [=](){
        if (reply->error() != QNetworkReply::NoError) {
            qCWarning(dcConsolinnoEnergy()) << "HemsOptimizer: Failed to get pv optimization. The reply returned with error" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) << reply->errorString();
            QByteArray responsedata = reply->readAll();
            qCWarning(dcConsolinnoEnergy()) << qUtf8Printable(responsedata);
            setState(StateIdle);
            return;
        }

        QByteArray data = reply->readAll();
        QJsonParseError error;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
        if (error.error != QJsonParseError::NoError) {
            qCWarning(dcConsolinnoEnergy()) << "HemsOptimizer: Failed to parse pv optimization data" << data << ":" << error.errorString();
            setState(StateIdle);
            return;
        }

        qCDebug(dcConsolinnoEnergy()) << "HemsOptimizer: Request pv optimization finished successfully";
        qCDebug(dcConsolinnoEnergy()) << "<--" << qUtf8Printable(jsonDoc.toJson(QJsonDocument::Indented));


    });
}
