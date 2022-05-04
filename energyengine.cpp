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
//#include "hemsoptimizerengine.h"
//#include "weatherdataprovider.h"

#include <QJsonDocument>
#include <QNetworkReply>

Q_DECLARE_LOGGING_CATEGORY(dcConsolinnoEnergy)

EnergyEngine::EnergyEngine(ThingManager *thingManager, EnergyManager *energyManager, QObject *parent):
    QObject(parent),
    m_thingManager(thingManager),
    m_energyManager(energyManager)
{
    qCDebug(dcConsolinnoEnergy()) << "======> Initializing consolinno energy engine...";

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
    //m_optimizerEngine->setHousholdPowerLimit(m_housholdPowerLimit);

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

            // Verify the car implements the correct interface
            if (!carThing->thingClass().interfaces().contains("electricvehicle")) {
                qCWarning(dcConsolinnoEnergy()) << "Could not set pv configuration. The given car thing does not implement the electricvehicle interface." << carThing;
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
        //updateSchedules();

    }

    return HemsErrorNoError;
}



QList<PvConfiguration> EnergyEngine::pvConfigurations() const
{
    return m_pvConfigurations.values();
}

EnergyEngine::HemsError EnergyEngine::setPvConfiguration(const PvConfiguration &pvConfiguration)
{

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


QList<ChargingSessionConfiguration> EnergyEngine::chargingSessionConfigurations() const
{
    return m_chargingSessionConfigurations.values();
}

EnergyEngine::HemsError EnergyEngine::setChargingSessionConfiguration(const ChargingSessionConfiguration &chargingSessionConfiguration)
{

    if (!m_chargingSessionConfigurations.contains(chargingSessionConfiguration.evChargerThingId())) {
        qCWarning(dcConsolinnoEnergy()) << "Could not set charging session configuration. The given evCharger id does not exist." << chargingSessionConfiguration;
        return HemsErrorInvalidThing;
    }


     if (m_chargingSessionConfigurations.value(chargingSessionConfiguration.evChargerThingId()) != chargingSessionConfiguration) {


        m_chargingSessionConfigurations[chargingSessionConfiguration.evChargerThingId()] = chargingSessionConfiguration;

        qCDebug(dcConsolinnoEnergy()) << "ChargingSession configuration changed" << chargingSessionConfiguration;
        // save changes in ChargingSessionConfig
        saveChargingSessionConfigurationToSettings(chargingSessionConfiguration);
        // send Signal that the chargingSessionConfig has changed
        emit chargingSessionConfigurationChanged(chargingSessionConfiguration);

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

    // This signal tells us, which state has changed (can also tell us to which value)
    connect(thing, &Thing::stateValueChanged, this, [=](const StateTypeId &stateTypeId){
        StateType stateType = m_evChargers.value(thing->id())->thingClass().getStateType(stateTypeId);
        // use case: EvCharger gets unplugged, while an optimization is happening
        if (stateType.name() == "pluggedIn"){
            qCDebug(dcConsolinnoEnergy()) << "EvCharger pluggedin value changed ";

            if (m_evChargers.value(thing->id())->state(stateTypeId).value() == false){
                qCDebug(dcConsolinnoEnergy()) << "the pluggedIn value changed to false";
                pluggedInEventHandling(thing);
            }
        }else{
            qCDebug(dcConsolinnoEnergy()) << "The state: " << stateType.name()  << " changed";
        }


            });
}

void EnergyEngine::monitorChargingSession(Thing *thing)
{
    qCDebug(dcConsolinnoEnergy()) << "Start monitoring ev chargers chargingSessions" << thing;
    //m_evChargers.insert(thing->id(), thing);
    evaluateAvailableUseCases();
    loadChargingSessionConfiguration(thing->id());

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
        monitorChargingSession(thing);
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
            qCDebug(dcConsolinnoEnergy()) << "Removed pv configuration" << pvConfig;
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
        // Charger
        if (m_chargingConfigurations.contains(thingId)) {
            ChargingConfiguration chargingConfig = m_chargingConfigurations.take(thingId);
            removeChargingConfigurationFromSettings(thingId);
            emit chargingConfigurationRemoved(thingId);
            qCDebug(dcConsolinnoEnergy()) << "Removed charging configuration" << chargingConfig;
        }
        // charging Session
        // Remove ChargingSession Note: Maybe we need to change this

        foreach(const ChargingSessionConfiguration &chargingSession, m_chargingSessionConfigurations){
                if(chargingSession.evChargerThingId() == thingId){
                    ChargingSessionConfiguration debugMessage = m_chargingSessionConfigurations.take(chargingSession.evChargerThingId());
                    removeChargingSessionConfigurationFromSettings(chargingSession.evChargerThingId());
                    emit chargingSessionConfigurationRemoved(chargingSession.evChargerThingId());
                    qCDebug(dcConsolinnoEnergy()) << "Removed chargingsession configuration" << debugMessage;
                    break;
                }

        }

        /*
        if (m_chargingSessionConfigurations.contains(thingId)  ) {
            ChargingConfiguration chargingConfig = m_chargingConfigurations.take(thingId);
            removeChargingConfigurationFromSettings(thingId);
            emit chargingConfigurationRemoved(thingId);
            qCDebug(dcConsolinnoEnergy()) << "Removed charging configuration" << chargingConfig;
        }
        */

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
            qCDebug(dcConsolinnoEnergy()) << "Phase" << phase << "exceeding limit:" << currentPhaseConsumption.value(phase) << "W. Maximum allowance:" << phasePowerLimit << "W";
            limitExceeded = true;
            double phaseOvershotPower = currentPhaseConsumption.value(phase) - phasePowerLimit;
            if (phaseOvershotPower > overshotPower) {
                overshotPower = phaseOvershotPower;
            }
        } else {
            //qCInfo(dcConsolinnoEnergy()) << "= Phase" << phase << "at" << currentPhaseConsumption.value(phase) << "W from maximal" << phasePowerLimit << "W ->" << currentPhaseConsumption.value(phase) * 100.0 / phasePowerLimit << "%";
        }
    }

    // TODO: limit the consumption depending on a hirarchy and check calculate the amout of energy we can actually adjust down * 1.2 or something

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


// check whether e.g charging is possible, by checking if the necessary things are available (charger, car and rootMeter)
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

// EventHandling functions
// These are functions, which are used to solve specific events
void EnergyEngine::pluggedInEventHandling(Thing *thing)
{
    qCDebug(dcConsolinnoEnergy()) << "pluggedIn Changed from true to false";
    ChargingConfiguration configuration = m_chargingConfigurations.value(thing->id());
    configuration.setOptimizationEnabled(false);
    saveChargingConfigurationToSettings(configuration);
    emit chargingConfigurationChanged(configuration);

}



// every configuration needs to be loaded, saved and removed at some point
void EnergyEngine::loadHeatingConfiguration(const ThingId &heatPumpThingId)
{
    QSettings settings(NymeaSettings::settingsPath() + "/consolinno.conf", QSettings::IniFormat);
    settings.beginGroup("HeatingConfigurations");
    if (settings.childGroups().contains(heatPumpThingId.toString())) {
        settings.beginGroup(heatPumpThingId.toString());

        HeatingConfiguration configuration;
        configuration.setHeatPumpThingId(heatPumpThingId);
        configuration.setOptimizationEnabled(settings.value("optimizationEnabled").toBool());
        configuration.setHouseType(static_cast<HeatingConfiguration::HouseType>(settings.value("houseType").toInt()));
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
        configuration.setOptimizationMode(settings.value("optimizationMode").toInt());
        configuration.setOptimizationEnabled(settings.value("optimizationEnabled").toBool());
        configuration.setCarThingId(ThingId(settings.value("carThingId").toUuid()));
        configuration.setEndTime(settings.value("endTime").toString());
        configuration.setTargetPercentage(settings.value("targetPercentage").toUInt());
        configuration.setUniqueIdentifier(settings.value("uniqueIdentifier").toUuid());
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
    settings.setValue("optimizationMode", chargingConfiguration.optimizationMode());
    settings.setValue("optimizationEnabled", chargingConfiguration.optimizationEnabled());
    settings.setValue("carThingId", chargingConfiguration.carThingId());
    settings.setValue("endTime", chargingConfiguration.endTime());
    settings.setValue("targetPercentage", chargingConfiguration.targetPercentage());
    settings.setValue("uniqueIdentifier", chargingConfiguration.uniqueIdentifier());
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
    settings.beginGroup(pvConfiguration.pvThingId().toString());
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
    qCDebug(dcConsolinnoEnergy()) << "Pv settings: " << settings.childGroups();
    if (settings.childGroups().contains(pvThingId.toString())) {
        settings.beginGroup(pvThingId.toString());

        PvConfiguration configuration;
        configuration.setPvThingId(pvThingId);
        configuration.setLongitude(settings.value("longitude").toFloat());
        configuration.setLatitude(settings.value("latitude").toFloat());
        configuration.setRoofPitch(settings.value("roofPitch").toInt());
        configuration.setAlignment(settings.value("alignment").toInt());
        configuration.setKwPeak(settings.value("kwPeak").toFloat());

        settings.endGroup(); // ThingId

        m_pvConfigurations.insert(pvThingId, configuration);

        emit pvConfigurationAdded(configuration);

        qCDebug(dcConsolinnoEnergy()) << "Loaded";
    }
    else {
        // Pv usecase is available and this inverter has no configuration yet, lets add one
        PvConfiguration configuration;
        configuration.setPvThingId(pvThingId);
        m_pvConfigurations.insert(pvThingId, configuration);
        emit pvConfigurationAdded(configuration);
        qCDebug(dcConsolinnoEnergy()) << "Added new";
        savePvConfigurationToSettings(configuration);
    }
    settings.endGroup(); // PvConfigurations
}


void EnergyEngine::loadChargingSessionConfiguration(const ThingId &evChargerThingId)
{
    QSettings settings(NymeaSettings::settingsPath() + "/consolinno.conf", QSettings::IniFormat);
    settings.beginGroup("ChargingSessionConfigurations");
    qCDebug(dcConsolinnoEnergy()) << "Charging Session settings: " << settings.childGroups();
    if (settings.childGroups().contains(evChargerThingId.toString())) {
        settings.beginGroup(evChargerThingId.toString());

        ChargingSessionConfiguration configuration;

        configuration.setEvChargerThingId(evChargerThingId);
        configuration.setCarThingId(settings.value("carThingId").toUuid());

        configuration.setStartedAt(settings.value("startedAt").toString());
        configuration.setFinishedAt(settings.value("finishedAt").toString());

        configuration.setInitialBatteryEnergy(settings.value("initialBatteryEnergy").toFloat());
        configuration.setDuration(settings.value("duration").toInt());
        configuration.setEnergyCharged(settings.value("energyCharged").toFloat());
        configuration.setEnergyBattery(settings.value("energyBattery").toFloat());
        configuration.setBatteryLevel(settings.value("batteryLevel").toInt());
        configuration.setSessionId(settings.value("sessionId").toUuid());
        configuration.setState(settings.value("state").toInt());
        configuration.setTimestamp(settings.value("timestamp").toInt());


        settings.endGroup();

        m_chargingSessionConfigurations.insert(evChargerThingId, configuration);
        emit chargingSessionConfigurationAdded(configuration);

        qCDebug(dcConsolinnoEnergy()) << "Loaded" << configuration;
    } else {
        // Charging usecase is available and this ev charger has no configuration yet, lets add one
        ChargingSessionConfiguration configuration;
        configuration.setEvChargerThingId(evChargerThingId);
        m_chargingSessionConfigurations.insert(evChargerThingId, configuration);
        emit chargingSessionConfigurationAdded(configuration);

        qCDebug(dcConsolinnoEnergy()) << "Added new" << configuration;
        saveChargingSessionConfigurationToSettings(configuration);
    }
    settings.endGroup(); // ChargingConfigurations
}

void EnergyEngine::saveChargingSessionConfigurationToSettings(const ChargingSessionConfiguration &chargingSessionConfiguration)
{
    qCDebug(dcConsolinnoEnergy() ) << " saving ChargingSessionConfiguration" ;
    QSettings settings(NymeaSettings::settingsPath() + "/consolinno.conf", QSettings::IniFormat);
    settings.beginGroup("ChargingSessionConfigurations");
    settings.beginGroup(chargingSessionConfiguration.evChargerThingId().toString());
    settings.setValue("carThingId", chargingSessionConfiguration.carThingId());

    settings.setValue("startedAt" , chargingSessionConfiguration.startedAt() );
    settings.setValue("finishedAt", chargingSessionConfiguration.finishedAt() );

    settings.setValue("initialBatteryEnergy", chargingSessionConfiguration.initialBatteryEnergy());
    settings.setValue("duration", chargingSessionConfiguration.duration() );
    settings.setValue("energyCharged", chargingSessionConfiguration.energyCharged());
    settings.setValue("energyBattery", chargingSessionConfiguration.energyBattery());
    settings.setValue("batteryLevel", chargingSessionConfiguration.batteryLevel());
    settings.setValue("sessionId", chargingSessionConfiguration.sessionId());
    settings.setValue("state", chargingSessionConfiguration.state());
    settings.setValue("timestamp", chargingSessionConfiguration.timestamp());
    settings.endGroup();
    settings.endGroup();

}

void EnergyEngine::removeChargingSessionConfigurationFromSettings(const ThingId &evChargerThingId)
{
    QSettings settings(NymeaSettings::settingsPath() + "/consolinno.conf", QSettings::IniFormat);
    settings.beginGroup("ChargingSessionConfigurations");
    settings.beginGroup(evChargerThingId.toString());
    settings.remove("");
    settings.endGroup();
    settings.endGroup();
}


