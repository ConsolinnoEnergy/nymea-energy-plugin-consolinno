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

    // Engine for interacting with the online Hems optimizer
    m_optimizerEngine = new HemsOptimizerEngine(m_energyManager, m_weatherDataProvider, m_networkManager, this);
    m_optimizerEngine->setHousholdPowerLimit(m_housholdPowerLimit);
    qCDebug(dcConsolinnoEnergy()) << "======> Consolinno energy engine initialized" << m_availableUseCases;

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
    qCWarning(dcConsolinnoEnergy()) << "setHeatingConfiguration" << heatingConfiguration;
    qCWarning(dcConsolinnoEnergy()) << "More indepth: " << m_heatingConfigurations.value(heatingConfiguration.heatPumpThingId());


    qCDebug(dcConsolinnoEnergy()) << "Set heating configuration called" << heatingConfiguration;
    if (!m_heatingConfigurations.contains(heatingConfiguration.heatPumpThingId())) {
        qCWarning(dcConsolinnoEnergy()) << "Could not set heating configuration. The given heat pump thing id does not exist." << heatingConfiguration;
        return HemsErrorInvalidThing;
    }

    // Verify the optional heat meter
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

        // Update the schedules for this heat pump since the configuration has changed
        if (heatingConfiguration.optimizationEnabled()) {
            updateSchedules();
        }
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
        updateSchedules();
    }

    return HemsErrorNoError;
}





QList<PvConfiguration> EnergyEngine::pvConfigurations() const
{

    return m_pvConfigurations.values();

}

EnergyEngine::HemsError EnergyEngine::setPvConfiguration(const PvConfiguration &pvConfiguration)
{
    qCWarning(dcConsolinnoEnergy()) << "setPvConfiguration" << pvConfiguration;
    qCWarning(dcConsolinnoEnergy()) << "More indepth: " << m_pvConfigurations.value(pvConfiguration.pvThingId());
    if (!m_pvConfigurations.contains(pvConfiguration.pvThingId())) {
        qCWarning(dcConsolinnoEnergy()) << "Could not set pv configuration. The given pv thing id does not exist." << pvConfiguration;
        return HemsErrorInvalidThing;
    }


     if (m_pvConfigurations.value(pvConfiguration.pvThingId()) != pvConfiguration) {

        m_pvConfigurations[pvConfiguration.pvThingId()] = pvConfiguration;
        qCDebug(dcConsolinnoEnergy()) << "Pv configuration changed" << pvConfiguration;
        savePvConfigurationToSettings(pvConfiguration);
        emit pvConfigurationChanged(pvConfiguration);

    }

    return HemsErrorNoError;
}





void EnergyEngine::monitorHeatPump(Thing *thing)
{
    qCDebug(dcConsolinnoEnergy()) << "Start monitoring heatpump" << thing;
    m_heatPumps.insert(thing->id(), thing);
    evaluateAvailableUseCases();
    loadHeatingConfiguration(thing->id());
}

void EnergyEngine::monitorInverter(Thing *thing)
{
    qCDebug(dcConsolinnoEnergy()) << "Start monitoring inverter" << thing;
    m_inverters.insert(thing->id(), thing);
    evaluateAvailableUseCases();
    loadPvConfiguration(thing->id());
}

void EnergyEngine::monitorEvCharger(Thing *thing)
{
    qCDebug(dcConsolinnoEnergy()) << "Start monitoring ev charger" << thing;
    m_evChargers.insert(thing->id(), thing);
    evaluateAvailableUseCases();
    loadChargingConfiguration(thing->id());
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
            qCDebug(dcConsolinnoEnergy()) << "Using open weather map service" << thing->name() << "as weather forecast provider.";

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

        if (m_pvConfigurations.contains(thingId)) {
            PvConfiguration pvConfig = m_pvConfigurations.take(thingId);
            removePvConfigurationFromSettings(thingId);
            emit pvConfigurationRemoved(thingId);
            qCDebug(dcConsolinnoEnergy()) << "Removed heating configuration" << pvConfig;
        }

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

    // Check if this was an assigned car and update the configuration
    foreach (const ChargingConfiguration &chargingConfig, m_chargingConfigurations.values()) {
        if (chargingConfig.carThingId() == thingId) {
            qCDebug(dcConsolinnoEnergy()) << "Removing assigned car from charging configuration";
            ChargingConfiguration config = chargingConfig;
            config.setCarThingId(ThingId());
            // Disable config since incomplete
            config.setOptimizationEnabled(false);
            setChargingConfiguration(config);
        }
    }

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
    // We need a root meter, otherwise no optimization or blackout protection can be done.
    if (!m_energyManager->rootMeter())
        return;

    qCDebug(dcConsolinnoEnergy()) << "============> Evaluate system:" << QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss");
    qCDebug(dcConsolinnoEnergy()) << "Root meter consumption changed" << m_energyManager->rootMeter()->stateValue("currentPower").toDouble() << "W";

    // Evaluate individual device types
    evaluateInverters();
    evaluateHeatPumps();
    evaluateEvChargers();

    // Blackout protection just incase something is still over the limit
    qCDebug(dcConsolinnoEnergy()) << "--> Evaluating blackout protection";
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
    if (m_heatPumps.isEmpty())
        return;

    qCDebug(dcConsolinnoEnergy()) << "--> Evaluating heat pumps";
    foreach (const HeatingConfiguration &heatingConfiguration, m_heatingConfigurations) {
        if (heatingConfiguration.optimizationEnabled()) {

        }
    }
}

void EnergyEngine::evaluateInverters()
{
    if (m_inverters.isEmpty())
        return;

    qCDebug(dcConsolinnoEnergy()) << "--> Evaluating inverters";
    //foreach (const PvConfiguration &pvConfiguration, m_pvConfigurations) {
    //}
}

void EnergyEngine::evaluateEvChargers()
{
    if (m_evChargers.isEmpty())
        return;

    qCDebug(dcConsolinnoEnergy()) << "--> Evaluating EV chargers";

    // Check if charging and update the SoC

}

void EnergyEngine::updateSchedules()
{   
    // Get all information and update the optimizer schedules
    HeatingConfiguration heatingConfig;
    HemsOptimizerEngine::ChargingSchedule chargingSchedule;

    foreach (const HeatingConfiguration &heatingConfiguration, m_heatingConfigurations) {
        if (heatingConfiguration.isValid() && heatingConfiguration.optimizationEnabled()) {
            heatingConfig = heatingConfiguration;
            // We support only one heating atm
            break;
        }
    }

    foreach (const ChargingConfiguration &chargingConfiguration, m_chargingConfigurations) {
        if (chargingConfiguration.isValid() && chargingConfiguration.optimizationEnabled()) {
            chargingSchedule.chargingConfiguration = chargingConfiguration;
        }
    }

    // Calculate schedule information
    if (chargingSchedule.chargingConfiguration.isValid()) {
        // Calculate the max power
        Thing *evChargerThing = m_evChargers.value(chargingSchedule.chargingConfiguration.evChargerThingId());
        Thing *carThing = m_thingManager->findConfiguredThing(chargingSchedule.chargingConfiguration.carThingId());
        if (!evChargerThing || !carThing) {
            qCWarning(dcConsolinnoEnergy()) << "Cannot get pv optimization for" << chargingSchedule.chargingConfiguration << "because the associated things could not be found.";
            chargingSchedule.chargingConfiguration = ChargingConfiguration();
        } else {
            // Fill out chargingSchedule
            uint maxChargingCurrent = evChargerThing->stateValue("maxChargingCurrent").toUInt(); // A
            uint minChargingCurrent = carThing->stateValue("minChargingCurrent").toUInt(); // A
            uint phaseCount = evChargerThing->stateValue("phaseCount").toUInt();
            uint stateOfChargePercentage = carThing->stateValue("batteryLevel").toInt(); // %
            uint totalCapacity = carThing->stateValue("capacity").toDouble() * 1000; // Wh

            qCDebug(dcConsolinnoEnergy()) << "State of charge:" << stateOfChargePercentage << "Capacity:" << totalCapacity << "Wh";
            chargingSchedule.maxPower = maxChargingCurrent * phaseCount * 230; // W
            chargingSchedule.minPower = minChargingCurrent * phaseCount * 230; // W
            chargingSchedule.energyNeeded = totalCapacity * stateOfChargePercentage / 100.0;
            qCDebug(dcConsolinnoEnergy()) << chargingSchedule;
       }
    }

    m_optimizerEngine->updatePvOptimizationSchedule(heatingConfig, chargingSchedule);
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

        // No root meter, we cannot do anything
        return;
    }

    // Heating
    if (m_energyManager->rootMeter() && !m_inverters.isEmpty() && !m_heatPumps.isEmpty()) {
        // We need at least a root meter and and inverter for having the heating use case
        availableUseCases = availableUseCases.setFlag(HemsUseCaseHeating);
    }

    // Charging
    if (m_energyManager->rootMeter() && !m_inverters.isEmpty() && !m_evChargers.isEmpty()) {
        // We need at least a root meter, an inverter and and ev charger for having the charging usecase
        availableUseCases = availableUseCases.setFlag(HemsUseCaseCharging);
    }

    if (m_energyManager->rootMeter() && !m_inverters.isEmpty()){
        //we need atleast a root meter and an inverter to read pv data
        availableUseCases = availableUseCases.setFlag(HemsUseCasePv);
    }

    if (m_availableUseCases != availableUseCases) {
        qCDebug(dcConsolinnoEnergy()) << "Available use cases changed from" << availableUseCases;
        m_availableUseCases = availableUseCases;
        emit availableUseCasesChanged(m_availableUseCases);
    }
}

void EnergyEngine::loadHeatingConfiguration(const ThingId &heatPumpThingId)
{
    QSettings settings(NymeaSettings::settingsPath() + "/consolinno.conf", QSettings::IniFormat);
    settings.beginGroup("HeatingConfigurations");
    if (settings.childGroups().contains(heatPumpThingId.toString())) {
        settings.beginGroup(heatPumpThingId.toString());

        HeatingConfiguration configuration;
        configuration.setHeatPumpThingId(heatPumpThingId);
        configuration.setOptimizationEnabled(settings.value("optimizationEnabled").toBool());
        configuration.setHouseType(static_cast<HemsOptimizerInterface::HouseType>(settings.value("houseType").toInt()));
        configuration.setFloorHeatingArea(settings.value("floorHeatingArea").toDouble());
        configuration.setMaxElectricalPower(settings.value("maxElectricalPower").toDouble());
        configuration.setMaxThermalEnergy(settings.value("maxThermalEnergy").toDouble());
        configuration.setHeatMeterThingId(ThingId(settings.value("heatMeterThingId").toUuid()));
        settings.endGroup(); // ThingId

        m_heatingConfigurations.insert(heatPumpThingId, configuration);
        emit heatingConfigurationAdded(configuration);

        qCDebug(dcConsolinnoEnergy()) << "Loaded" << configuration;
    } else {
        // Heating usecase is available and this heat pump has no configuration yet, lets add one
        HeatingConfiguration configuration;
        configuration.setHeatPumpThingId(heatPumpThingId);
        m_heatingConfigurations.insert(heatPumpThingId, configuration);
        emit heatingConfigurationAdded(configuration);
        qCDebug(dcConsolinnoEnergy()) << "Added new" << configuration;
        saveHeatingConfigurationToSettings(configuration);
    }
    settings.endGroup(); // HeatingConfigurations
}

void EnergyEngine::saveHeatingConfigurationToSettings(const HeatingConfiguration &heatingConfiguration)
{
    QSettings settings(NymeaSettings::settingsPath() + "/consolinno.conf", QSettings::IniFormat);
    settings.beginGroup("HeatingConfigurations");
    settings.beginGroup(heatingConfiguration.heatPumpThingId().toString());
    settings.setValue("optimizationEnabled", heatingConfiguration.optimizationEnabled());
    settings.setValue("houseType", heatingConfiguration.houseType());
    settings.setValue("floorHeatingArea", heatingConfiguration.floorHeatingArea());
    settings.setValue("maxElectricalPower", heatingConfiguration.maxElectricalPower());
    settings.setValue("maxThermalEnergy", heatingConfiguration.maxThermalEnergy());
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

void EnergyEngine::loadChargingConfiguration(const ThingId &evChargerThingId)
{
    QSettings settings(NymeaSettings::settingsPath() + "/consolinno.conf", QSettings::IniFormat);
    settings.beginGroup("ChargingConfigurations");
    if (settings.childGroups().contains(evChargerThingId.toString())) {
        settings.beginGroup(evChargerThingId.toString());

        ChargingConfiguration configuration;
        configuration.setEvChargerThingId(evChargerThingId);
        configuration.setOptimizationEnabled(settings.value("optimizationEnabled").toBool());
        configuration.setCarThingId(ThingId(settings.value("carThingId").toUuid()));
        configuration.setEndTime(settings.value("endTime").toTime());
        configuration.setTargetPercentage(settings.value("targetPercentage").toUInt());
        configuration.setZeroReturnPolicyEnabled(settings.value("zeroReturnPolicyEnabled").toBool());
        settings.endGroup();

        m_chargingConfigurations.insert(evChargerThingId, configuration);
        emit chargingConfigurationAdded(configuration);

        qCDebug(dcConsolinnoEnergy()) << "Loaded" << configuration;
    } else {
        // Charging usecase is available and this ev charger has no configuration yet, lets add one
        ChargingConfiguration configuration;
        configuration.setEvChargerThingId(evChargerThingId);
        m_chargingConfigurations.insert(evChargerThingId, configuration);
        emit chargingConfigurationAdded(configuration);

        qCDebug(dcConsolinnoEnergy()) << "Added new" << configuration;
        saveChargingConfigurationToSettings(configuration);
    }
    settings.endGroup(); // ChargingConfigurations
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

void EnergyEngine::savePvConfigurationToSettings(const PvConfiguration &pvConfiguration)
{

    QSettings settings(NymeaSettings::settingsPath() + "/consolinno.conf", QSettings::IniFormat);
    settings.beginGroup("PvConfigurations");
    settings.setValue("longitude", pvConfiguration.longitude());
    settings.setValue("latitude", pvConfiguration.latitude());
    settings.setValue("roofPitch", pvConfiguration.roofPitch());
    settings.setValue("alignment", pvConfiguration.alignment());
    settings.setValue("kwPeak", pvConfiguration.kwPeak());
    settings.endGroup();
    settings.endGroup();
}

void EnergyEngine::removePvConfigurationFromSettings(const ThingId &pvThingId)
{

    QSettings settings(NymeaSettings::settingsPath() + "/consolinno.conf", QSettings::IniFormat);
    settings.beginGroup("PvConfigurations");
    settings.beginGroup(pvThingId.toString());
    settings.remove("");
    settings.endGroup();
    settings.endGroup();
}

void EnergyEngine::loadPvConfiguration(const ThingId &pvThingId)
{

   QSettings settings(NymeaSettings::settingsPath() + "/consolinno.conf", QSettings::IniFormat);
    settings.beginGroup("PvConfigurations");
    if (settings.childGroups().contains(pvThingId.toString())) {
        settings.beginGroup(pvThingId.toString());

        PvConfiguration configuration;
        configuration.setPvThingId(pvThingId);
        configuration.setLongitude(settings.value("longitude").toDouble());
        configuration.setLatitude(settings.value("latitude").toDouble());
        configuration.setRoofPitch(settings.value("roofPitch").toInt());
        configuration.setAlignment(settings.value("alignment").toInt());
        configuration.setKwPeak(settings.value("kwPeak").toFloat());

        settings.endGroup(); // ThingId

        m_pvConfigurations.insert(pvThingId, configuration);

        emit pvConfigurationAdded(configuration);

        qCDebug(dcConsolinnoEnergy()) << "Loaded";
    }
    else {
        // Heating usecase is available and this heat pump has no configuration yet, lets add one
        PvConfiguration configuration;
        configuration.setPvThingId(pvThingId);
        m_pvConfigurations.insert(pvThingId, configuration);
        emit pvConfigurationAdded(configuration);
        qCDebug(dcConsolinnoEnergy()) << "Added new";
        savePvConfigurationToSettings(configuration);
    }
    settings.endGroup(); // PvConfigurations
}
