#include "hemsoptimizerengine.h"

#include <QJsonDocument>
#include <QNetworkReply>

#include "loggingcategories.h"
Q_DECLARE_LOGGING_CATEGORY(dcConsolinnoEnergy)

HemsOptimizerEngine::HemsOptimizerEngine(EnergyManager *energyManager, QNetworkAccessManager *networkManager, QObject *parent) :
    QObject(parent),
    m_energyManager(energyManager),
    m_interface(new HemsOptimizerInterface(networkManager, this))
{

//    updatePvOptimizationSchedule();

//    //*********** Testing

//    QDateTime nowUtc = QDateTime::currentDateTimeUtc();
//    QVector<QDateTime> timestamps = generateScheduleTimeStamps(nowUtc, 15, m_scheduleWindowHours);

//    QVariantMap testData = getTemperatureForecast(nowUtc);
//    HemsOptimizerSchedules testSchedules;
//    int countIndex = 0;
//    foreach (const QString &timestampString, testData.keys()) {
//        HemsOptimizerSchedule schedule(QDateTime::fromString(timestampString, Qt::ISODate), countIndex);
//        qCDebug(dcConsolinnoEnergy()) << schedule;
//        testSchedules.append(schedule);
//        countIndex++;
//    }

//    testSchedules.sort();

//    HemsOptimizerSchedules interpolatedSchedules = interpolateValues(timestamps, testSchedules);
//    qCDebug(dcConsolinnoEnergy()) << "--------------- Interpolated result";
//    foreach (const HemsOptimizerSchedule &schedule, interpolatedSchedules) {
//        qCDebug(dcConsolinnoEnergy()) << schedule;
//    }

//    exit(0);


//    QNetworkReply *reply = m_interface->florHeatingPowerDemandStandardProfile(HemsOptimizerInterface::HouseTypeLowEnergy, 100, getTemperatureHistory(currentDateTime), getTemperatureForecast(currentDateTime));
//    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
//    connect(reply, &QNetworkReply::finished, reply, [=](){
//        if (reply->error() != QNetworkReply::NoError) {
//            qCWarning(dcConsolinnoEnergy()) << "HemsOptimizer: Failed to get electrical demand standard profile. The reply returned with error" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) << reply->errorString();
//            QByteArray responsedata = reply->readAll();
//            qCWarning(dcConsolinnoEnergy()) << qUtf8Printable(responsedata);
//            return;
//        }

//        QByteArray data = reply->readAll();
//        QJsonParseError error;
//        QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
//        if (error.error != QJsonParseError::NoError) {
//            qCWarning(dcConsolinnoEnergy()) << "HemsOptimizer: Failed to parse electrical demand standard profile data" << data << ":" << error.errorString();
//            return;
//        }

//        qCDebug(dcConsolinnoEnergy()) << "HemsOptimizer: Request electrical demand standard profile finished successfully";
//        //qCDebug(dcConsolinnoEnergy()) << "<--" << qUtf8Printable(jsonDoc.toJson(QJsonDocument::Indented));

//        HemsOptimizerSchedules schedules = m_interface->parseSchedules(jsonDoc.toVariant().toMap());
//        qCDebug(dcConsolinnoEnergy()) << "--------------- Result";
//        foreach (const HemsOptimizerSchedule &schedule, schedules) {
//            qCDebug(dcConsolinnoEnergy()) << schedule;
//        }

//        // TODO: perform interpolation
//        HemsOptimizerSchedules interpolatedSchedules = interpolateValues(timestamps, schedules);
//        qCDebug(dcConsolinnoEnergy()) << "--------------- Interpolated result";
//        foreach (const HemsOptimizerSchedule &schedule, interpolatedSchedules) {
//            qCDebug(dcConsolinnoEnergy()) << schedule;
//        }

//    });

}

HemsOptimizerInterface *HemsOptimizerEngine::interface() const
{
    return m_interface;
}

void HemsOptimizerEngine::updatePvOptimizationSchedule()
{
    qCDebug(dcConsolinnoEnergy()) << "--> Update schedules for the next" << m_scheduleWindowHours << "hours on 15 minutes resolution...";
    QDateTime nowUtc = QDateTime::currentDateTimeUtc();

    // Note: everything communicated from and to the server must be UTC, within th system, we work with local time

    // Create timestamps for the next 24 hours in 15 min timeslots
    QVector<QDateTime> timestamps = generateScheduleTimeStamps(nowUtc, 15, m_scheduleWindowHours);
    qCDebug(dcConsolinnoEnergy()) << "Created" << timestamps.count() <<  "schedule time slots from" << timestamps.first().toString("dd.MM.yyyy HH:mm:ss") << "until" << timestamps.last().toString("dd.MM.yyyy HH:mm:ss");

    // Get the consumption from the yesterday for this time window
    PowerBalanceLogEntries powerBalanceHistory = m_energyManager->logs()->powerBalanceLogs(EnergyLogs::SampleRate15Mins, timestamps.first().addDays(-1), timestamps.last().addDays(-1));
    if (powerBalanceHistory.count() < timestamps.count()) {
        qCWarning(dcConsolinnoEnergy()) << "Not enought historical data for optimization. Having" << powerBalanceHistory.count() << "historical samples but require at least" << timestamps.count() << "sampels for doing optimization.";
        return;
    }

    //    qCDebug(dcConsolinnoEnergy()) << "Logs form the past yesterday based on 15 min sample rate in this period";
    //    foreach (const PowerBalanceLogEntry &log, powerBalanceHistory) {
    //        qCDebug(dcConsolinnoEnergy()) << log.timestamp().toString("dd.MM.yyyy HH:mm:ss") << "Consumption" << log.consumption() << "W";
    //    }

    // Root meter
    QVariantMap ntpMap = m_interface->buildRootMeterInformation(timestamps, m_energyManager->rootMeter(), m_housholdPowerLimit, 0.3);

    // Inverter
    QVariantList productionForecast = getPvForecast(powerBalanceHistory);
    QVariantMap pvMap = m_interface->buildPhotovoltaicInformation(timestamps, productionForecast);

    // Electrical demand
    QVariantList consumptionForecast = getConsumptionForecast(powerBalanceHistory);
    QVariantMap electricalDemandMap = m_interface->buildElectricDemandInformation(timestamps, QUuid::createUuid(), consumptionForecast);


    QNetworkReply *reply = m_interface->florHeatingPowerDemandStandardProfile(HemsOptimizerInterface::HouseTypeLowEnergy, 100, getTemperatureHistory(nowUtc), getTemperatureForecast(nowUtc));
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
    connect(reply, &QNetworkReply::finished, reply, [=](){
        if (reply->error() != QNetworkReply::NoError) {
            qCWarning(dcConsolinnoEnergy()) << "HemsOptimizer: Failed to get electrical demand standard profile. The reply returned with error" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) << reply->errorString();
            QByteArray responsedata = reply->readAll();
            qCWarning(dcConsolinnoEnergy()) << qUtf8Printable(responsedata);
            return;
        }

        QByteArray data = reply->readAll();
        QJsonParseError error;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
        if (error.error != QJsonParseError::NoError) {
            qCWarning(dcConsolinnoEnergy()) << "HemsOptimizer: Failed to parse electrical demand standard profile data" << data << ":" << error.errorString();
            return;
        }

        qCDebug(dcConsolinnoEnergy()) << "HemsOptimizer: Request electrical demand standard profile finished successfully";
        qCDebug(dcConsolinnoEnergy()) << "<--" << qUtf8Printable(jsonDoc.toJson(QJsonDocument::Indented));

        // Read the schedules
        HemsOptimizerSchedules floorHeatingPowerForecast = m_interface->parseSchedules(jsonDoc.toVariant().toMap());
        foreach (const HemsOptimizerSchedule &schedule, floorHeatingPowerForecast) {
            qCDebug(dcConsolinnoEnergy()) << schedule;
        }

        // Heat pump
        QVariantList thermalDemandForcast = getThermalDemandForecast(timestamps, floorHeatingPowerForecast);
        QVariantList copForecast = getCopForecast(timestamps, 3.5);
        // Heatpump has a maximum of 9 kW
        double maxElectricalPower = 9;
        // We assume a maximal themral energy of 4kWh
        double maxThermalEnergy = 4;

        QVariantMap heatPumpMap = m_interface->buildHeatpumpInformation(timestamps, QUuid::createUuid(), maxThermalEnergy, 0.5, maxElectricalPower, thermalDemandForcast, copForecast, 0.1);
        QNetworkReply *reply = m_interface->pvOptimization(ntpMap, pvMap, electricalDemandMap, heatPumpMap);
        connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
        connect(reply, &QNetworkReply::finished, reply, [=](){
            if (reply->error() != QNetworkReply::NoError) {
                qCWarning(dcConsolinnoEnergy()) << "HemsOptimizer: Failed to get pv optimization. The reply returned with error" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) << reply->errorString();
                QByteArray responsedata = reply->readAll();
                qCWarning(dcConsolinnoEnergy()) << qUtf8Printable(responsedata);
                return;
            }

            QByteArray data = reply->readAll();
            QJsonParseError error;
            QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
            if (error.error != QJsonParseError::NoError) {
                qCWarning(dcConsolinnoEnergy()) << "HemsOptimizer: Failed to parse pv optimization data" << data << ":" << error.errorString();
                return;
            }

            qCDebug(dcConsolinnoEnergy()) << "HemsOptimizer: Request pv optimization finished successfully";
            qCDebug(dcConsolinnoEnergy()) << "<--" << qUtf8Printable(jsonDoc.toJson(QJsonDocument::Indented));



        });
    });

}

double HemsOptimizerEngine::housholdPowerLimit() const
{
    return m_housholdPowerLimit;
}

void HemsOptimizerEngine::setHousholdPowerLimit(double housholdLimit)
{
    m_housholdPowerLimit = housholdLimit;
}

QVector<QDateTime> HemsOptimizerEngine::generateScheduleTimeStamps(const QDateTime &nowUtc, uint resolutionMinutes, uint durationHours)
{
    qCDebug(dcConsolinnoEnergy()) << "Generate timestamps for schedule in" << resolutionMinutes << "min resolution for the next" << durationHours;
    // Note: forecasts and schedules will be used on 15 min base starting at a full hour
    QDate date = nowUtc.date();
    QTime time = nowUtc.time();
    uint minutesOffsetToNextSlot = (15 - time.minute() % 15);
    time = time.addSecs(minutesOffsetToNextSlot * 60);
    time.setHMS(time.hour(), time.minute(), 0);

    QDateTime scheduleStartDateTime = QDateTime(date, time);
    qCDebug(dcConsolinnoEnergy()) << "Minutes until next slot:" << minutesOffsetToNextSlot << scheduleStartDateTime.toString();

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
        thermalDemandForecast << interpolatedForecast.at(i).value() / 1000.0; // kW
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
    // TODO: fetch from weather plugin
    QDateTime currentDateTimeRounded = nowUtc;
    currentDateTimeRounded.setTime(QTime(nowUtc.time().hour(), 0, 0));

    QVariantMap history;
    for (int i = 0; i < 24; i++) {
        history.insert(currentDateTimeRounded.toUTC().addSecs(-(3600 * i)).toString(Qt::ISODate), 5);
    }

    return history;
}

QVariantMap HemsOptimizerEngine::getTemperatureForecast(const QDateTime &nowUtc)
{
    // Create a list of hourly 24h outdoor temperature history
    // TODO: fetch from weather plugin
    QDateTime currentDateTimeRounded = nowUtc;
    currentDateTimeRounded.setTime(QTime(nowUtc.time().hour(), 0, 0));

    QVariantMap forecast;
    for (int i = 0; i < 24; i++) {
        forecast.insert(currentDateTimeRounded.addSecs(3600 * i).toString(Qt::ISODate), 5);
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

        qCDebug(dcConsolinnoEnergy()) << "-------------------";
        qCDebug(dcConsolinnoEnergy()) << "Target from" << targetScheduleStart.toString("hh:mm.ss") << "to" << targetScheduleEnd.toString("hh:mm.ss");
        qCDebug(dcConsolinnoEnergy()) << sourceScheduleIndex << "Current schedule" << sourceSchedules.at(sourceScheduleIndex);
        outputSchedules << HemsOptimizerSchedule(targetScheduleStart, sourceSchedules.at(sourceScheduleIndex).value());
    }

    Q_ASSERT_X(outputSchedules.count() == desiredTimestamps.count(), "HemsOptimizerEngine", "The interpolated schedules count does not match the desired schedule count");
    return outputSchedules;
}
