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

#include "energyengine.h"
#include "nymeasettings.h"
#include "hemsoptimizerengine.h"
#include "weatherdataprovider.h"

#include <QJsonDocument>
#include <QNetworkReply>

Q_DECLARE_LOGGING_CATEGORY(dcConsolinnoEnergy)

EnergyEngine::EnergyEngine(ThingManager *thingManager, EnergyManager *energyManager, QObject *parent):
    QObject(parent),
    m_thingManager(thingManager),
    m_energyManager(energyManager),
    m_networkManager(new QNetworkAccessManager(this))
{
    qCDebug(dcConsolinnoEnergy()) << "======> Initializing consolinno energy engine...";

    // Weather data provider
    // Note: initialize before the things get added, so the weather information get provided automatically
    m_weatherDataProvider = new WeatherDataProvider(m_networkManager, this);
    connect(m_weatherDataProvider, &WeatherDataProvider::weatherDataUpdated, this, [=](){
        qCDebug(dcConsolinnoEnergy()) << "Weather data updated";
    });

    // Energy engine
    connect(m_energyManager, &EnergyManager::rootMeterChanged, this, &EnergyEngine::onRootMeterChanged);
    onRootMeterChanged();

    // Thing manager
    foreach (Thing *thing, m_thingManager->configuredThings()) {
        onThingAdded(thing);
    }

    connect(thingManager, &ThingManager::thingAdded, this, &EnergyEngine::onThingAdded);
    connect(thingManager, &ThingManager::thingRemoved, this, &EnergyEngine::onThingRemoved);

    // Load configurations
    QSettings settings(NymeaSettings::settingsPath() + "/consolinno.conf", QSettings::IniFormat);

    settings.beginGroup("BlackoutProtection");
    m_housholdPhaseLimit = settings.value("housholdPhaseLimit", 25).toUInt();
    settings.endGroup();

    m_housholdPowerLimit = m_housholdPhaseLimit * m_housholdPhaseCount * 230;
    qCDebug(dcConsolinnoEnergy()) << "Houshold phase limit" << m_housholdPhaseLimit << "[A] using" << m_housholdPhaseCount << "phases: max power" << m_housholdPowerLimit << "[W]";

    settings.beginGroup("HeatingConfigurations");
    foreach (const QString &heatPumpUuidString, settings.childGroups()) {
        settings.beginGroup(heatPumpUuidString);
        ThingId heatPumpThingId(heatPumpUuidString);

        HeatingConfiguration configuration;
        configuration.setHeatPumpThingId(heatPumpThingId);
        configuration.setOptimizationEnabled(settings.value("optimizationEnabled").toBool());
        configuration.setHeatMeterThingId(ThingId(settings.value("heatMeterThingId").toUuid()));
        settings.endGroup();

        m_heatingConfigurations.insert(heatPumpThingId, configuration);
        qCDebug(dcConsolinnoEnergy()) << "Loaded" << configuration;
    }
    settings.endGroup();

    settings.beginGroup("ChargingConfigurations");
    foreach (const QString &evChargerUuidString, settings.childGroups()) {
        settings.beginGroup(evChargerUuidString);
        ThingId evChargerThingId(evChargerUuidString);

        ChargingConfiguration configuration;
        configuration.setEvChargerThingId(evChargerThingId);
        configuration.setOptimizationEnabled(settings.value("optimizationEnabled").toBool());
        configuration.setCarThingId(ThingId(settings.value("carThingId").toUuid()));
        configuration.setEndTime(settings.value("endTime").toTime());
        configuration.setTargetPercentage(settings.value("targetPercentage").toUInt());
        configuration.setZeroReturnPolicyEnabled(settings.value("zeroReturnPolicyEnabled").toBool());
        settings.endGroup();

        m_chargingConfigurations.insert(evChargerThingId, configuration);
        qCDebug(dcConsolinnoEnergy()) << "Loaded" << configuration;
    }
    settings.endGroup();

    // Engine for interacting with the online Hems optimizer
    m_optimizerEngine = new HemsOptimizerEngine(m_energyManager, m_weatherDataProvider, m_networkManager, this);
    m_optimizerEngine->setHousholdPowerLimit(m_housholdPowerLimit);
    qCDebug(dcConsolinnoEnergy()) << "======> Consolinno energy engine initialized" << m_availableUseCases;

//    m_optimizerEngine->updatePvOptimizationSchedule();
}

EnergyEngine::HemsUseCases EnergyEngine::availableUseCases() const
{
    return m_availableUseCases;
}

uint EnergyEngine::housholdPhaseLimit() const
{
    return m_housholdPhaseLimit;
}

EnergyEngine::HemsError EnergyEngine::setHousholdPhaseLimit(uint housholdPhaseLimit)
{
    if (m_housholdPhaseLimit == housholdPhaseLimit)
        return HemsErrorNoError;

    if (housholdPhaseLimit == 0)
        return HemsErrorInvalidPhaseLimit;


    m_housholdPhaseLimit = housholdPhaseLimit;
    emit housholdPhaseLimitChanged(m_housholdPhaseLimit);

    m_housholdPowerLimit = m_housholdPhaseLimit * m_housholdPhaseCount * 230;
    qCDebug(dcConsolinnoEnergy()) << "Houshold phase limit changed to" << m_housholdPhaseLimit << "[A] using" << m_housholdPhaseCount << "phases: max power" << m_housholdPowerLimit << "[W]";
    m_optimizerEngine->setHousholdPowerLimit(m_housholdPowerLimit);

    QSettings settings(NymeaSettings::settingsPath() + "/consolinno.conf", QSettings::IniFormat);
    settings.beginGroup("BlackoutProtection");
    settings.setValue("housholdPhaseLimit", m_housholdPhaseLimit);
    settings.endGroup();

    return HemsErrorNoError;
}

QList<HeatingConfiguration> EnergyEngine::heatingConfigurations() const
{
    return m_heatingConfigurations.values();
}

EnergyEngine::HemsError EnergyEngine::setHeatingConfiguration(const HeatingConfiguration &heatingConfiguration)
{
    qCDebug(dcConsolinnoEnergy()) << "Set heating configuration called" << heatingConfiguration;
    if (!m_heatingConfigurations.contains(heatingConfiguration.heatPumpThingId())) {
        qCWarning(dcConsolinnoEnergy()) << "Could not set heating configuration. The given heat pump thing id does not exist." << heatingConfiguration;
        return HemsErrorInvalidThing;
    }

    if (!heatingConfiguration.heatMeterThingId().isNull()) {
        Thing *heatMeterThing = m_thingManager->findConfiguredThing(heatingConfiguration.heatMeterThingId());
        if (!heatMeterThing) {
            qCWarning(dcConsolinnoEnergy()) << "Could not set heating configuration. The given heat meter thing does not exist." << heatingConfiguration;
            return HemsErrorThingNotFound;
        }

        if (!heatMeterThing->thingClass().interfaces().contains("heatmeter")) {
            qCWarning(dcConsolinnoEnergy()) << "Could not set heating configuration. The given heat meter thing does not implement the heatmeter interface." << heatMeterThing;
            return HemsErrorInvalidParameter;
        }
    }

    if (m_heatingConfigurations.value(heatingConfiguration.heatPumpThingId()) != heatingConfiguration) {
        m_heatingConfigurations[heatingConfiguration.heatPumpThingId()] = heatingConfiguration;
        qCDebug(dcConsolinnoEnergy()) << "Heating configuration changed" << heatingConfiguration;
        saveHeatingConfigurationToSettings(heatingConfiguration);
        emit heatingConfigurationChanged(heatingConfiguration);
        evaluate();
    }

    return HemsErrorNoError;
}

QList<ChargingConfiguration> EnergyEngine::chargingConfigurations() const
{
    return m_chargingConfigurations.values();
}

EnergyEngine::HemsError EnergyEngine::setChargingConfiguration(const ChargingConfiguration &chargingConfiguration)
{
    qCDebug(dcConsolinnoEnergy()) << "Set charging configuration called" << chargingConfiguration;
    if (!m_chargingConfigurations.contains(chargingConfiguration.evChargerThingId())) {
        qCWarning(dcConsolinnoEnergy()) << "Could not set charging configuration. The given ev charger thing id does not exist." << chargingConfiguration;
        return HemsErrorInvalidThing;
    }

    // Make sure the configuration is valid if enabled
    if (chargingConfiguration.optimizationEnabled()) {
        // Make sure we have an assigned car, otherwise we cannot enable the optimization
        if (chargingConfiguration.carThingId().isNull()) {
            qCWarning(dcConsolinnoEnergy()) << "Could not set charging configuration. The configuration is enabled but there is no assigned car." << chargingConfiguration;
            return HemsErrorInvalidThing;
        } else {
            // Verify the car thing exists
            Thing *carThing = m_thingManager->findConfiguredThing(chargingConfiguration.carThingId());
            if (!carThing) {
                qCWarning(dcConsolinnoEnergy()) << "Could not set charging configuration. The configuration is enabled but the given car thing does not exist in the system." << chargingConfiguration;
                return HemsErrorThingNotFound;
            }

            // Verify the cas implements the correct interface
            if (!carThing->thingClass().interfaces().contains("electricvehicle")) {
                qCWarning(dcConsolinnoEnergy()) << "Could not set heating configuration. The given car thing does not implement the electricvehicle interface." << carThing;
                return HemsErrorInvalidThing;
            }
        }
    }

    // Update the configuraton
    if (m_chargingConfigurations.value(chargingConfiguration.evChargerThingId()) != chargingConfiguration) {
        m_chargingConfigurations[chargingConfiguration.evChargerThingId()] = chargingConfiguration;
        qCDebug(dcConsolinnoEnergy()) << "Charging configuration changed" << chargingConfiguration;
        saveChargingConfigurationToSettings(chargingConfiguration);
        emit chargingConfigurationChanged(chargingConfiguration);
        evaluate();
    }

    return HemsErrorNoError;
}

void EnergyEngine::monitorHeatPump(Thing *thing)
{
    qCDebug(dcConsolinnoEnergy()) << "Start monitoring heatpump" << thing;
    m_heatPumps.insert(thing->id(), thing);

    evaluateAvailableUseCases();

    if (!m_heatingConfigurations.contains(thing->id()) && m_availableUseCases.testFlag(HemsUseCaseHeating)) {
        // Heating usecase is available and this heat pump has no configuration yet, lets add one
        HeatingConfiguration configuration;
        configuration.setHeatPumpThingId(thing->id());
        m_heatingConfigurations.insert(thing->id(), configuration);
        emit heatingConfigurationAdded(configuration);
        qCDebug(dcConsolinnoEnergy()) << "Added new" << configuration;
        saveHeatingConfigurationToSettings(configuration);
    }
}

void EnergyEngine::monitorInverter(Thing *thing)
{
    qCDebug(dcConsolinnoEnergy()) << "Start monitoring inverter" << thing;
    m_inverters.insert(thing->id(), thing);

    evaluateAvailableUseCases();
}

void EnergyEngine::monitorEvCharger(Thing *thing)
{
    qCDebug(dcConsolinnoEnergy()) << "Start monitoring ev charger" << thing;
    m_evChargers.insert(thing->id(), thing);

    evaluateAvailableUseCases();

    if (!m_chargingConfigurations.contains(thing->id()) && m_availableUseCases.testFlag(HemsUseCaseCharging)) {
        // Charging usecase is available and this ev charger has no configuration yet, lets add one
        ChargingConfiguration configuration;
        configuration.setEvChargerThingId(thing->id());
        m_chargingConfigurations.insert(thing->id(), configuration);
        emit chargingConfigurationAdded(configuration);
        qCDebug(dcConsolinnoEnergy()) << "Added new" << configuration;
        saveChargingConfigurationToSettings(configuration);
    }
}

void EnergyEngine::onThingAdded(Thing *thing)
{
    if (thing->thingClass().interfaces().contains("solarinverter")) {
        monitorInverter(thing);
    }

    if (thing->thingClass().interfaces().contains("heatpump")) {
        monitorHeatPump(thing);
    }

    if (thing->thingClass().interfaces().contains("evcharger")) {
        monitorEvCharger(thing);
    }

    if (thing->thingClass().interfaces().contains("weather")) {
        // For now we support only the open weather map api, get the thing params
        if (thing->thingClassId() == ThingClassId("985195aa-17ad-4530-88a4-cdd753d747d7")) {
            m_weatherThing = thing;
            qCDebug(dcConsolinnoEnergy()) << "Using open weather map service" << thing->name() << "as weather forcast provider.";

            // Take the weather location from the openweathermap thing params
            QString locationId = m_weatherThing->paramValue("id").toString();
            QString apiKey = "c1b9d5584bb740804871583f6c62744f"; // Note: community key!
            qCDebug(dcConsolinnoEnergy()) << "Setting up weather data provider using location" << locationId;
            m_weatherDataProvider->setLocationId(locationId);
            m_weatherDataProvider->setApiKey(apiKey);
            m_weatherDataProvider->updateWeatherInformation();
        }
    }
}

void EnergyEngine::onThingRemoved(const ThingId &thingId)
{
    // Inverter
    if (m_inverters.contains(thingId)) {
        m_inverters.remove(thingId);
        qCDebug(dcConsolinnoEnergy()) << "Removed inverter from energy manager"  << thingId.toString();
    }

    // Heat pump
    if (m_heatPumps.contains(thingId)) {
        m_heatPumps.remove(thingId);
        qCDebug(dcConsolinnoEnergy()) << "Removed heat pump from energy manager" << thingId.toString();

        if (m_heatingConfigurations.contains(thingId)) {
            HeatingConfiguration heatingConfig = m_heatingConfigurations.take(thingId);
            removeHeatingConfigurationFromSettings(thingId);
            emit heatingConfigurationRemoved(thingId);
            qCDebug(dcConsolinnoEnergy()) << "Removed heating configuration" << heatingConfig;
        }
    }

    // Ev charger
    if (m_evChargers.contains(thingId)) {
        m_evChargers.remove(thingId);
        qCDebug(dcConsolinnoEnergy()) << "Removed evcharger from energy manager" << thingId.toString();

        if (m_chargingConfigurations.contains(thingId)) {
            ChargingConfiguration chargingConfig = m_chargingConfigurations.take(thingId);
            removeChargingConfigurationFromSettings(thingId);
            emit chargingConfigurationRemoved(thingId);
            qCDebug(dcConsolinnoEnergy()) << "Removed charging configuration" << chargingConfig;
        }
    }

    // Weather
    if (m_weatherThing && m_weatherThing->id() == thingId) {
        qCDebug(dcConsolinnoEnergy()) << "Weather service has been removed.";
        m_weatherThing = nullptr;
        m_weatherDataProvider->setLocationId(QString());
        qCDebug(dcConsolinnoEnergy()) << "The weather data provider is not available any more.";

        // TODO: Check if there is any other weather location
    }

    // FIXME: update configuration if associated car thing or heat meter thing has been removed

    evaluateAvailableUseCases();
}

void EnergyEngine::onRootMeterChanged()
{
    if (m_energyManager->rootMeter()) {
        qCDebug(dcConsolinnoEnergy()) << "Using root meter" << m_energyManager->rootMeter();
        connect(m_energyManager->rootMeter(), &Thing::stateValueChanged, this, [=](const StateTypeId &stateTypeId, const QVariant &value){
            Q_UNUSED(value)
            StateType stateType = m_energyManager->rootMeter()->thingClass().getStateType(stateTypeId);
            if (stateType.name() == "currentPower") {
                evaluate();
            }
        });
    } else {
        qCWarning(dcConsolinnoEnergy()) << "There is no root meter configured. Optimization will not be available until a root meter has been declared in the energy experience.";
    }

    evaluateAvailableUseCases();
}

void EnergyEngine::evaluate()
{
    // We need a root meter, otherwise no optimization can be done.
    if (!m_energyManager->rootMeter())
        return;

    qCDebug(dcConsolinnoEnergy()) << "Root meter consumption changed" << m_energyManager->rootMeter()->stateValue("currentPower").toDouble() << "W";

    // Evaluate individual device types
    evaluateInverters();
    evaluateHeatPumps();
    evaluateEvChargers();

    //updateSchedules();

    // Blackout protection just incase something is still over the limit
    qCDebug(dcConsolinnoEnergy()) << "============> Evaluating blackout protection";
    QHash<QString, double> currentPhaseConsumption = {
        {"A", m_energyManager->rootMeter()->stateValue("currentPowerPhaseA").toDouble()},
        {"B", m_energyManager->rootMeter()->stateValue("currentPowerPhaseB").toDouble()},
        {"C", m_energyManager->rootMeter()->stateValue("currentPowerPhaseC").toDouble()},
    };
    bool limitExceeded = false;
    double phasePowerLimit = 230 * m_housholdPhaseLimit;
    double overshotPower = 0;
    qCDebug(dcConsolinnoEnergy()) << "Houshold phase limit" << m_housholdPhaseLimit << "[A] =" << phasePowerLimit << "[W] at 230V";
    foreach (const QString &phase, currentPhaseConsumption.keys()) {
        if (currentPhaseConsumption.value(phase) > phasePowerLimit) {
            qCInfo(dcConsolinnoEnergy()) << "Phase" << phase << "exceeding limit:" << currentPhaseConsumption.value(phase) << "W. Maximum allowance:" << phasePowerLimit << "W";
            limitExceeded = true;
            double phaseOvershotPower = currentPhaseConsumption.value(phase) - phasePowerLimit;
            if (phaseOvershotPower > overshotPower) {
                overshotPower = phaseOvershotPower;
            }
        } else {
            //qCInfo(dcConsolinnoEnergy()) << "= Phase" << phase << "at" << currentPhaseConsumption.value(phase) << "W from maximal" << phasePowerLimit << "W ->" << currentPhaseConsumption.value(phase) * 100.0 / phasePowerLimit << "%";
        }
    }

    // TODO: limit the consuption depending on a hirarchy and check calculate the amout of energy we can actually adjust down * 1.2 or something

    if (limitExceeded) {
        qCInfo(dcConsolinnoEnergy()) << "Using at least" << overshotPower  << "W to much. Start adjusting the evChargers...";
        // Note: iterate all chargers, not just the one we are optimizing
        foreach (Thing *thing, m_evChargers) {
            State maxChargingCurrentState = thing->state("maxChargingCurrent");
            Action action(maxChargingCurrentState.stateTypeId(), thing->id(), Action::TriggeredByRule);
            action.setParams(ParamList() << Param(maxChargingCurrentState.stateTypeId(), maxChargingCurrentState.minValue()));
            qCInfo(dcConsolinnoEnergy()) << "Adjusting charging on" << thing->name() << "to minimum of" << maxChargingCurrentState.minValue() << "A";
            m_thingManager->executeAction(action);
        }
    }
}

void EnergyEngine::evaluateHeatPumps()
{

}

void EnergyEngine::evaluateInverters()
{

}

void EnergyEngine::evaluateEvChargers()
{

}

void EnergyEngine::updateSchedules()
{   
    //qCDebug(dcConsolinnoEnergy()) << "--> Update schedules for the next" << m_scheduleWindowHours << "hours on 15 minutes resolution...";
    //QDateTime currentDateTime = QDateTime::currentDateTime();
//    // Create timestamps for the next 12 hours in 10 min slots
//    QVector<QDateTime> timestamps = generateScheduleTimeStamps(15, m_scheduleWindowHours);
//    qCDebug(dcConsolinnoEnergy()) << "Created" << timestamps.count() <<  "schedule time slots from" << timestamps.first().toString("dd.MM.yyyy HH:mm:ss") << "until" << timestamps.last().toString("dd.MM.yyyy HH:mm:ss");

//    // Get the consumption from the yesterday for this time window
//    PowerBalanceLogEntries powerBalanceHistory = m_energyManager->logs()->powerBalanceLogs(EnergyLogs::SampleRate15Mins, timestamps.first().addDays(-1), timestamps.last().addDays(-1));
//    if (powerBalanceHistory.count() < timestamps.count()) {
//        qCWarning(dcConsolinnoEnergy()) << "Not enought historical data for optimization. Having" << powerBalanceHistory.count() << "historical samples but require at least" << timestamps.count() << "sampels for doing optimization.";
//        return;
//    }

//    qCDebug(dcConsolinnoEnergy()) << "Logs form the past yesterday based on 15 min sample rate in this period";
//    foreach (const PowerBalanceLogEntry &log, powerBalanceHistory) {
//        qCDebug(dcConsolinnoEnergy()) << log.timestamp().toString("dd.MM.yyyy HH:mm:ss") << "Consumption" << log.consumption() << "W";
//    }

//    // Root meter
//    QVariantMap ntpMap = m_optimizer->buildRootMeterInformation(timestamps, m_energyManager->rootMeter(), m_housholdPowerLimit, 0.3);

//    // Inverter
//    QVariantList productionForecast = getPvForecast(powerBalanceHistory);
//    QVariantMap pvMap = m_optimizer->buildPhotovoltaicInformation(timestamps, productionForecast);

//    // Electrical demand
//    QVariantList consumptionForecast = getConsumptionForecast(powerBalanceHistory);
//    QVariantMap electricalDemandMap = m_optimizer->buildElectricDemandInformation(timestamps, QUuid::createUuid(), consumptionForecast);






    //    // Heat pump
    //    QVariantList thermalDemandForcast = getThermalDemandForecast(timestamps, m_heatPump);
    //    QVariantList copForecast;
    //    for (int i = 0; i < timestamps.count(); i++) {
    //        // Default to 3 for now
    //        copForecast << 3;
    //    }
    //    // Heatpump has a maximum of 9 kW
    //    double maxElectricalPower = 9;
    //    // We assume a maximal themral energy of 4kWh
    //    double maxThermalEnergy = 4;

    //    QVariantMap heatPumpMap = m_optimizer->buildHeatpumpInformation(timestamps, m_heatPump, maxThermalEnergy, -1, maxElectricalPower, thermalDemandForcast, copForecast, 0.1);

    //    QNetworkReply *reply = m_optimizer->pvOptimization(ntpMap, pvMap, electricalDemandMap, heatPumpMap);
    //    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
    //    connect(reply, &QNetworkReply::finished, reply, [=](){
    //        if (reply->error() != QNetworkReply::NoError) {
    //            qCWarning(dcConsolinnoEnergy()) << "HemsOptimizer: Failed to get pv optimization. The reply returned with error" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) << reply->errorString();
    //            QByteArray responsedata = reply->readAll();
    //            qCWarning(dcConsolinnoEnergy()) << qUtf8Printable(responsedata);
    //            return;
    //        }

    //        QByteArray data = reply->readAll();
    //        QJsonParseError error;
    //        QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
    //        if (error.error != QJsonParseError::NoError) {
    //            qCWarning(dcConsolinnoEnergy()) << "HemsOptimizer: Failed to parse pv optimization data" << data << ":" << error.errorString();
    //            return;
    //        }

    //        qCDebug(dcConsolinnoEnergy()) << "HemsOptimizer: Request pv optimization finished successfully";
    //        qCDebug(dcConsolinnoEnergy()) << "<--" << qUtf8Printable(jsonDoc.toJson(QJsonDocument::Indented));
    //    });
}

void EnergyEngine::evaluateAvailableUseCases()
{
    HemsUseCases availableUseCases;
    if (m_energyManager->rootMeter()) {
        // We need a root meter for the blackout protection
        // TODO: probably we need also a heat pump and or a charger for beeing able to protect, let's keep this condition for now.
        availableUseCases = availableUseCases.setFlag(HemsUseCaseBlackoutProtection);
    } else {
        // None of the optimizations is available, we always need a configured root meter.
        if (m_availableUseCases != availableUseCases) {
            qCDebug(dcConsolinnoEnergy()) << "No use cases available because there is no root meter defined.";
            m_availableUseCases = availableUseCases;
            emit availableUseCasesChanged(m_availableUseCases);
        }
        return;
    }

    if (m_energyManager->rootMeter() && !m_inverters.isEmpty() && !m_heatPumps.isEmpty()) {
        // We need a root meter, an inverter and and ev charging for having the charging usecase
        availableUseCases = availableUseCases.setFlag(HemsUseCaseHeating);
    }

    if (m_energyManager->rootMeter() && !m_inverters.isEmpty() && !m_evChargers.isEmpty()) {
        // We need a root meter, an inverter and and ev charging for having the charging usecase
        availableUseCases = availableUseCases.setFlag(HemsUseCaseCharging);
    }

    if (m_availableUseCases != availableUseCases) {
        qCDebug(dcConsolinnoEnergy()) << "Available use cases changed from" << availableUseCases;
        m_availableUseCases = availableUseCases;
        emit availableUseCasesChanged(m_availableUseCases);
    }
}

void EnergyEngine::saveHeatingConfigurationToSettings(const HeatingConfiguration &heatingConfiguration)
{
    QSettings settings(NymeaSettings::settingsPath() + "/consolinno.conf", QSettings::IniFormat);
    settings.beginGroup("HeatingConfigurations");
    settings.beginGroup(heatingConfiguration.heatPumpThingId().toString());
    settings.setValue("optimizationEnabled", heatingConfiguration.optimizationEnabled());
    settings.setValue("heatMeterThingId", heatingConfiguration.heatMeterThingId());
    settings.endGroup();
    settings.endGroup();
}

void EnergyEngine::removeHeatingConfigurationFromSettings(const ThingId &heatPumpThingId)
{
    QSettings settings(NymeaSettings::settingsPath() + "/consolinno.conf", QSettings::IniFormat);
    settings.beginGroup("HeatingConfigurations");
    settings.beginGroup(heatPumpThingId.toString());
    settings.remove("");
    settings.endGroup();
    settings.endGroup();
}

void EnergyEngine::saveChargingConfigurationToSettings(const ChargingConfiguration &chargingConfiguration)
{
    QSettings settings(NymeaSettings::settingsPath() + "/consolinno.conf", QSettings::IniFormat);
    settings.beginGroup("ChargingConfigurations");
    settings.beginGroup(chargingConfiguration.evChargerThingId().toString());
    settings.setValue("optimizationEnabled", chargingConfiguration.optimizationEnabled());
    settings.setValue("carThingId", chargingConfiguration.carThingId());
    settings.setValue("endTime", chargingConfiguration.endTime());
    settings.setValue("targetPercentage", chargingConfiguration.targetPercentage());
    settings.setValue("zeroReturnPolicyEnabled", chargingConfiguration.zeroReturnPolicyEnabled());
    settings.endGroup();
    settings.endGroup();
}

void EnergyEngine::removeChargingConfigurationFromSettings(const ThingId &evChargerThingId)
{
    QSettings settings(NymeaSettings::settingsPath() + "/consolinno.conf", QSettings::IniFormat);
    settings.beginGroup("ChargingConfigurations");
    settings.beginGroup(evChargerThingId.toString());
    settings.remove("");
    settings.endGroup();
    settings.endGroup();
}

