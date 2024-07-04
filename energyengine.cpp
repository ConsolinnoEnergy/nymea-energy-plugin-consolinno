/* Copyright (C) Consolinno Energy GmbH - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include "energyengine.h"
#include "nymeasettings.h"
#include <integrations/integrationplugin.h>

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

    // Load UserConfig
    monitorUserConfig();

    settings.beginGroup("BlackoutProtection");
    m_housholdPhaseLimit = settings.value("housholdPhaseLimit", 25).toUInt();
    settings.endGroup();

    settings.beginGroup("HybridSimulation");
    m_hybridSimulationEnabled = settings.value("enabled", 0).toBool();
    m_hybridSimIgnoreSimulated = settings.value("ignoreSimulated", "true").toBool();
    m_hybridSimulationMap =  settings.value("mappings").toMap();
    settings.endGroup();

    m_housholdPowerLimit = m_housholdPhaseLimit * m_housholdPhaseCount * 230;
    qCDebug(dcConsolinnoEnergy()) << "Houshold phase limit" << m_housholdPhaseLimit << "[A] using" << m_housholdPhaseCount << "phases: max power" << m_housholdPowerLimit << "[W]";

    qCDebug(dcConsolinnoEnergy()) << "======> Consolinno energy engine initialized" << m_availableUseCases;

    if(m_hybridSimulationEnabled) {
        qCInfo(dcConsolinnoEnergy()) << "======> Hybrid simulation enabled";
        qCDebug(dcConsolinnoEnergy()) << "======> Hybrid simulation mappings" << m_hybridSimulationMap;
    } else {
        qCDebug(dcConsolinnoEnergy()) << "======> Hybrid simulation disabled";
    }

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

    QSettings settings(NymeaSettings::settingsPath() + "/consolinno.conf", QSettings::IniFormat);
    settings.beginGroup("BlackoutProtection");
    settings.setValue("housholdPhaseLimit", m_housholdPhaseLimit);
    settings.endGroup();

    return HemsErrorNoError;
}

ConEMSState EnergyEngine::ConemsState() const
{
    return m_conEMSState;
}

EnergyEngine::HemsError EnergyEngine::setConEMSState(const ConEMSState &conEMSState)
{
    if (m_conEMSState != conEMSState) {
        m_conEMSState = conEMSState;
        qCDebug(dcConsolinnoEnergy()) << "ConEMSState changed" << conEMSState;
        emit conEMSStateChanged(conEMSState);
    } else{
        qCDebug(dcConsolinnoEnergy()) << "ConEMSState did not change";

    }
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

QList<HeatingRodConfiguration> EnergyEngine::heatingRodConfigurations() const
{
    return m_heatingRodConfigurations.values();
}

EnergyEngine::HemsError EnergyEngine::setHeatingRodConfiguration(const HeatingRodConfiguration &heatingRodConfiguration)
{

    qCDebug(dcConsolinnoEnergy()) << "Set heating rod configuration called" << heatingRodConfiguration;
    if (!m_heatingRodConfigurations.contains(heatingRodConfiguration.heatingRodThingId())) {
        qCWarning(dcConsolinnoEnergy()) << "Could not set heating rod configuration. The given heat pump thing id does not exist." << heatingRodConfiguration;
        return HemsErrorInvalidThing;
    }


    if (m_heatingRodConfigurations.value(heatingRodConfiguration.heatingRodThingId()) != heatingRodConfiguration) {
        m_heatingRodConfigurations[heatingRodConfiguration.heatingRodThingId()] = heatingRodConfiguration;
        qCDebug(dcConsolinnoEnergy()) << "Heating rod configuration changed" << heatingRodConfiguration;
        saveHeatingRodConfigurationToSettings(heatingRodConfiguration);
        emit heatingRodConfigurationChanged(heatingRodConfiguration);
    }

    return HemsErrorNoError;
}

QList<DynamicElectricPricingConfiguration> EnergyEngine::dynamicElectricPricingConfigurations() const
{
    return m_dynamicElectricPricingConfigurations.values();
}

EnergyEngine::HemsError EnergyEngine::setDynamicElectricPricingConfiguration(const DynamicElectricPricingConfiguration &dynamicElectricPricingConfiguration)
{
    qCDebug(dcConsolinnoEnergy()) << "Set dynamic electric pricing configuration called" << dynamicElectricPricingConfiguration;
    if (!m_dynamicElectricPricingConfigurations.contains(dynamicElectricPricingConfiguration.dynamicElectricPricingThingId())) {
        qCWarning(dcConsolinnoEnergy()) << "Could not set dynamic electric pricing configuration. The given dynamic electric pricing thing ID does not exist." << dynamicElectricPricingConfiguration;
        return HemsErrorInvalidThing;
    }

    if (m_dynamicElectricPricingConfigurations.value(dynamicElectricPricingConfiguration.dynamicElectricPricingThingId()) != dynamicElectricPricingConfiguration) {
        m_dynamicElectricPricingConfigurations[dynamicElectricPricingConfiguration.dynamicElectricPricingThingId()] = dynamicElectricPricingConfiguration;
        qCDebug(dcConsolinnoEnergy()) << "Dynamic electric pricing configuration changed" << dynamicElectricPricingConfiguration;
        saveDynamicElectricPricingConfigurationToSettings(dynamicElectricPricingConfiguration);
        emit dynamicElectricPricingConfigurationChanged(dynamicElectricPricingConfiguration);
    }

    return HemsErrorNoError;
}

QList<WashingMachineConfiguration> EnergyEngine::washingMachineConfigurations() const
{
    return m_washingMachineConfigurations.values();
}

EnergyEngine::HemsError EnergyEngine::setWashingMachineConfiguration(const WashingMachineConfiguration &washingMachineConfiguration)
{

    qCDebug(dcConsolinnoEnergy()) << "Set washing machine configuration called" << washingMachineConfiguration;
    if (!m_washingMachineConfigurations.contains(washingMachineConfiguration.washingMachineThingId())) {
        qCWarning(dcConsolinnoEnergy()) << "Could not set washing machine configuration. The given washing machine thing id does not exist." << washingMachineConfiguration;
        return HemsErrorInvalidThing;
    }


    if (m_washingMachineConfigurations.value(washingMachineConfiguration.washingMachineThingId()) != washingMachineConfiguration) {
        m_washingMachineConfigurations[washingMachineConfiguration.washingMachineThingId()] = washingMachineConfiguration;
        qCDebug(dcConsolinnoEnergy()) << "Washing machine configuration changed" << washingMachineConfiguration;
        saveWashingMachineConfigurationToSettings(washingMachineConfiguration);
        emit washingMachineConfigurationChanged(washingMachineConfiguration);
    }

    return HemsErrorNoError;
}


QList<ChargingConfiguration> EnergyEngine::chargingConfigurations() const
{
    return m_chargingConfigurations.values();
}

EnergyEngine::HemsError EnergyEngine::setChargingConfiguration(const ChargingConfiguration &chargingConfiguration)
{
    //qCDebug(dcConsolinnoEnergy()) << "Set charging configuration called" << chargingConfiguration;
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

    }

    return HemsErrorNoError;
}


QList<ChargingOptimizationConfiguration> EnergyEngine::chargingOptimizationConfigurations() const
{
    return m_chargingOptimizationConfigurations.values();
}

EnergyEngine::HemsError EnergyEngine::setChargingOptimizationConfiguration(const ChargingOptimizationConfiguration &chargingOptimizationConfiguration)
{
    qCDebug(dcConsolinnoEnergy()) << "Set charging Optimization configuration called" << chargingOptimizationConfiguration;
    if (!m_chargingOptimizationConfigurations.contains(chargingOptimizationConfiguration.evChargerThingId())) {
        qCWarning(dcConsolinnoEnergy()) << "Could not set charging configuration. The given ev charger thing id does not exist." << chargingOptimizationConfiguration;
        return HemsErrorInvalidThing;
    }

    // Update the configuraton
    if (m_chargingOptimizationConfigurations.value(chargingOptimizationConfiguration.evChargerThingId()) != chargingOptimizationConfiguration) {

        m_chargingOptimizationConfigurations[chargingOptimizationConfiguration.evChargerThingId()] = chargingOptimizationConfiguration;
        qCDebug(dcConsolinnoEnergy()) << "Charging configuration changed" << chargingOptimizationConfiguration;
        saveChargingOptimizationConfigurationToSettings(chargingOptimizationConfiguration);
        emit chargingOptimizationConfigurationChanged(chargingOptimizationConfiguration);

    }

    return HemsErrorNoError;
}



QList<BatteryConfiguration> EnergyEngine::batteryConfigurations() const
{
    return m_batteryConfigurations.values();
}

EnergyEngine::HemsError EnergyEngine::setBatteryConfiguration(const BatteryConfiguration &batteryConfiguration)
{

    if (!m_batteryConfigurations.contains(batteryConfiguration.batteryThingId())) {
        qCWarning(dcConsolinnoEnergy()) << "Could not set battery configuration. The given battery thing id does not exist." << batteryConfiguration;
        return HemsErrorInvalidThing;
    }


     if (m_batteryConfigurations.value(batteryConfiguration.batteryThingId()) != batteryConfiguration) {

        m_batteryConfigurations[batteryConfiguration.batteryThingId()] = batteryConfiguration;
        qCDebug(dcConsolinnoEnergy()) << "Battery configuration changed" << batteryConfiguration;
        saveBatteryConfigurationToSettings(batteryConfiguration);
        emit batteryConfigurationChanged(batteryConfiguration);

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

        //qCDebug(dcConsolinnoEnergy()) << "ChargingSession configuration changed" << chargingSessionConfiguration;
        // save changes in ChargingSessionConfig
        saveChargingSessionConfigurationToSettings(chargingSessionConfiguration);
        // send Signal that the chargingSessionConfig has changed
        emit chargingSessionConfigurationChanged(chargingSessionConfiguration);

    }

    return HemsErrorNoError;
}

QList<UserConfiguration> EnergyEngine::userConfigurations() const
{
    return m_userConfigurations.values();
}

EnergyEngine::HemsError EnergyEngine::setUserConfiguration(const UserConfiguration &userConfiguration)
{

    if (!m_userConfigurations.contains(userConfiguration.userConfigID())) {
        qCWarning(dcConsolinnoEnergy()) << "Could not set user configuration. The given user QUUid does not exist." << userConfiguration;
        return HemsErrorInvalidThing;
    }

    qCDebug(dcConsolinnoEnergy()) << "setUser configuration: " << userConfiguration;


     if (m_userConfigurations.value(userConfiguration.userConfigID()) != userConfiguration) {

        m_userConfigurations[userConfiguration.userConfigID()] = userConfiguration;
        qCDebug(dcConsolinnoEnergy()) << "User configuration changed" << userConfiguration;
        saveUserConfigurationToSettings(userConfiguration);
        emit userConfigurationChanged(userConfiguration);

    }

    return HemsErrorNoError;
}

// monitor Things
void EnergyEngine::monitorUserConfig()
{
    qCDebug(dcConsolinnoEnergy()) << "Start monitoring UserConfig";
    loadUserConfiguration();
}

void EnergyEngine::monitorBattery(Thing *thing)
{
    qCDebug(dcConsolinnoEnergy()) << "Start monitoring Battery" << thing;
    m_batteries.insert(thing->id(),thing);
    evaluateAvailableUseCases();
    loadBatteryConfiguration(thing->id());
}

void EnergyEngine::monitorHeatPump(Thing *thing)
{
    qCDebug(dcConsolinnoEnergy()) << "Start monitoring heatpump" << thing;
    m_heatPumps.insert(thing->id(), thing);
    evaluateAvailableUseCases();
    loadHeatingConfiguration(thing->id());
}

void EnergyEngine::monitorHeatingRod(Thing *thing)
{
    qCDebug(dcConsolinnoEnergy()) << "Start monitoring heating rod" << thing;
    m_heatingRods.insert(thing->id(), thing);
    evaluateAvailableUseCases();
    loadHeatingRodConfiguration(thing->id());
}

void EnergyEngine::monitorDynamicElectricPricing(Thing *thing)
{
    qCDebug(dcConsolinnoEnergy()) << "Start monitoring dynamic electric pricing" << thing;
    m_dynamicElectricPricings.insert(thing->id(), thing);
    evaluateAvailableUseCases();
    loadDynamicElectricPricingConfiguration(thing->id());
}

void EnergyEngine::monitorWashingMachine(Thing *thing)
{
    qCDebug(dcConsolinnoEnergy()) << "Start monitoring washing machine" << thing;
    m_washingMachines.insert(thing->id(), thing);
    evaluateAvailableUseCases();
    loadWashingMachineConfiguration(thing->id());
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
    loadChargingOptimizationConfiguration(thing->id());

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
        
        if (stateType.name() == "currentPower"){
            updateHybridSimulation(thing);
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

    if (thing->thingClass().interfaces().contains("smartheatingrod")) {
        monitorHeatingRod(thing);
    }

    if (thing->thingClass().interfaces().contains("dynamicelectricitypricing")) {
        monitorDynamicElectricPricing(thing);
    }

    if (thing->thingClass().interfaces().contains("smartwashingmachine")) {
        monitorWashingMachine(thing);
    }

    if (thing->thingClass().interfaces().contains("evcharger")) {
        
        monitorEvCharger(thing);
        monitorChargingSession(thing);

        // Handle Hybrid Simulation
        // Add a linked simulated ev charger if the evcharger is not simulated itself
        if (m_hybridSimulationEnabled)  {
            if (thing->thingClass().id().toString() != "{21a48e6d-6152-407a-a303-3b46e29bbb94}" || !m_hybridSimIgnoreSimulated) { 
                // This means thing is not a simulated evcharger; TODO: Better way than using uuids? 
                qCDebug(dcConsolinnoEnergy()) << "Adding generic simulated consumer for " << thing;
                // Define information for adding generic consumer
                ThingClassId thingClassId("3e13b1aa-4ecd-4b48-80be-0dfcc0e5cbe4"); 
                QString thingName = "Bridge (" + thing->name() + ")";
                ParamList thingParams = ParamList();
                ThingSetupInfo *info;
                info = m_thingManager->addConfiguredThing(thingClassId, thingParams, thingName);
                // Disable updating total energy consumption for linked simulated ev charger
                info->thing()->setSettingValue("updateTotalEnergy", false);
                // We keep track of the mapping using a QMap which is also persisted in consolinno.conf
                m_hybridSimulationMap.insert(thing->id().toString(), info->thing()->id().toString());
                qCDebug(dcConsolinnoEnergy()) << "Hybrid simulation map: " << m_hybridSimulationMap;
                QSettings settings(NymeaSettings::settingsPath() + "/consolinno.conf", QSettings::IniFormat);
                settings.beginGroup("HybridSimulation");
                settings.setValue("mappings", QVariant(m_hybridSimulationMap));
                settings.endGroup();
            }
        }
    }

    if (thing->thingClass().interfaces().contains("energystorage")) {
        monitorBattery(thing);
    }

}

void EnergyEngine::onThingRemoved(const ThingId &thingId)
{

    // Battery
    if (m_batteries.contains(thingId)) {
        m_batteries.remove(thingId);
        qCDebug(dcConsolinnoEnergy()) << "Removed battery from energy manager"  << thingId.toString();

        if (m_batteryConfigurations.contains(thingId)) {
            BatteryConfiguration batteryConfig = m_batteryConfigurations.take(thingId);
            removeBatteryConfigurationFromSettings(thingId);
            emit batteryConfigurationRemoved(thingId);
            qCDebug(dcConsolinnoEnergy()) << "Removed battery configuration" << batteryConfig;
        }

    }

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

    // Heating rod
    if (m_heatingRods.contains(thingId)) {
        m_heatingRods.remove(thingId);
        qCDebug(dcConsolinnoEnergy()) << "Removed heating rod from energy manager" << thingId.toString();

        if (m_heatingRodConfigurations.contains(thingId)) {
            HeatingRodConfiguration heatingRodConfig = m_heatingRodConfigurations.take(thingId);
            removeHeatingRodConfigurationFromSettings(thingId);
            emit heatingRodConfigurationRemoved(thingId);
            qCDebug(dcConsolinnoEnergy()) << "Removed heating rod configuration" << heatingRodConfig;
        }
    }

    // Dynamic Electric Pricing
    if (m_dynamicElectricPricings.contains(thingId)) {
        m_dynamicElectricPricings.remove(thingId);
        qCDebug(dcConsolinnoEnergy()) << "Removed dynamic electric pricing from energy manager" << thingId.toString();

        if (m_dynamicElectricPricingConfigurations.contains(thingId)) {
            DynamicElectricPricingConfiguration dynamicElectricPricingConfig = m_dynamicElectricPricingConfigurations.take(thingId);
            removeDynamicElectricPricingConfigurationFromSettings(thingId);
            emit dynamicElectricPricingConfigurationRemoved(thingId);
            qCDebug(dcConsolinnoEnergy()) << "Removed dynamic electric pricing configuration" << dynamicElectricPricingConfig;
        }
    }

    // Washing machine
    if (m_washingMachines.contains(thingId)) {
        m_washingMachines.remove(thingId);
        qCDebug(dcConsolinnoEnergy()) << "Removed washing machine from energy manager" << thingId.toString();

        if (m_washingMachineConfigurations.contains(thingId)) {
            WashingMachineConfiguration washingMachineConfig = m_washingMachineConfigurations.take(thingId);
            removeWashingMachineConfigurationFromSettings(thingId);
            emit washingMachineConfigurationRemoved(thingId);
            qCDebug(dcConsolinnoEnergy()) << "Removed washing Machine configuration" << washingMachineConfig;
        }
    }

    // Ev charger
    if (m_evChargers.contains(thingId)) {
        m_evChargers.remove(thingId);
        qCDebug(dcConsolinnoEnergy()) << "Removed evcharger from energy manager" << thingId.toString();
        // Chargeing
        if (m_chargingConfigurations.contains(thingId)) {
            ChargingConfiguration chargingConfig = m_chargingConfigurations.take(thingId);
            removeChargingConfigurationFromSettings(thingId);
            emit chargingConfigurationRemoved(thingId);
            qCDebug(dcConsolinnoEnergy()) << "Removed charging configuration" << chargingConfig;
        }

        // ChargeingOptimization
        if (m_chargingOptimizationConfigurations.contains(thingId)) {
            ChargingOptimizationConfiguration chargingConfig = m_chargingOptimizationConfigurations.take(thingId);
            removeChargingOptimizationConfigurationFromSettings(thingId);
            emit chargingOptimizationConfigurationRemoved(thingId);
            qCDebug(dcConsolinnoEnergy()) << "Removed charging Optimization configuration" << chargingConfig;
        }


        // Charging Session
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

    if (m_hybridSimulationEnabled) {
        if (m_hybridSimulationMap.contains(thingId.toString()))  {
            ThingId linkedThingId = m_hybridSimulationMap.value(thingId.toString()).toUuid();
            m_hybridSimulationMap.remove(thingId.toString());
            QSettings settings(NymeaSettings::settingsPath() + "/consolinno.conf", QSettings::IniFormat);
            settings.beginGroup("HybridSimulation");
            settings.setValue("mappings", QVariant(m_hybridSimulationMap));
            settings.endGroup();
            m_thingManager->removeConfiguredThing(linkedThingId);
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


void EnergyEngine::updateHybridSimulation(Thing *thing)
{
    if (!m_hybridSimulationEnabled) {
        return;
    }

    // We need  root meter for this
    if (!m_energyManager->rootMeter())
        return;
    // Only continue if root meter is simulated 
    // TODO: Get the id by looking for the simulation plugin and get the smartMeter thingClassId
    // Hardcoding the thingClassId is quicker for now but not robust
    if (m_energyManager->rootMeter()->thingClass().id().toString() != "{d96c77e3-dbf1-4875-95a4-7ca85aa3ef8e}") {
        qCWarning(dcConsolinnoEnergy()) << "Root meter is not simulated. Hybrid simulation is not available.";
        return;
    }
    qCDebug(dcConsolinnoEnergy()) << "Updating hybrid simulation for thing" << thing->name();

    if (!thing->thingClass().interfaces().contains("smartmeterconsumer")) {
       qCWarning(dcConsolinnoEnergy()) << "Thing" << thing->name() << "is not a smartmeter consumer. Hybrid simulation is not available.";
       return;
    }
    // This omits all things created by "nymea" vendor. 
    // This is a workaround for the fact that the energy simulation already evaluates these things elsewhere (e.g. simulated ev charger)
    // I couldn't find a better way to filter out these things yet.
    if (thing->thingClass().vendorId().toString() == "{2062d64d-3232-433c-88bc-0d33c0ba2ba6}" && m_hybridSimIgnoreSimulated) {
        qCDebug(dcConsolinnoEnergy()) << "Omitting thing " << thing->name() << " for hybrid simulation because it is a simulated device";
        return;
    }

    ThingId linkedThingId = m_hybridSimulationMap.value(thing->id().toString()).toUuid();
    qCDebug(dcConsolinnoEnergy()) << linkedThingId;
    qCDebug(dcConsolinnoEnergy()) << m_hybridSimulationMap;
    Thing* linkedSimulatedThing = m_thingManager->findConfiguredThing(linkedThingId);
    if (!linkedSimulatedThing) {
        qCWarning(dcConsolinnoEnergy()) << "Could not find linked simulated thing for" << thing->name();
        return;
    }
    qCDebug(dcConsolinnoEnergy()) << "Updating linked simulated thing " << linkedSimulatedThing->name();
    linkedSimulatedThing->setSettingValue("maxPower", thing->stateValue("currentPower"));
    linkedSimulatedThing->setStateValue("power", thing->stateValue("power"));
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
    double marginPower = 230 * m_housholdPhaseLimit;
    double currMax = 0;
    double overshotCurrent = 0;
    double absMax = 0;
    double absMin = 0;
    qCDebug(dcConsolinnoEnergy()) << "Houshold phase limit" << m_housholdPhaseLimit << "[A] =" << phasePowerLimit << "[W] at 230V";
    foreach (const QString &phase, currentPhaseConsumption.keys()) {
        if (currentPhaseConsumption.value(phase) > phasePowerLimit) {
            qCInfo(dcConsolinnoEnergy()) << "Blackout protection: Phase" << phase << "exceeding limit:" << currentPhaseConsumption.value(phase) << "W. Maximum allowance:" << phasePowerLimit << "W";
            limitExceeded = true;
            double phaseOvershotPower = currentPhaseConsumption.value(phase) - phasePowerLimit;
            if (phaseOvershotPower > overshotPower) {
                overshotPower = phaseOvershotPower;
            }
        } else {
            double phaseMarginPower = phasePowerLimit - currentPhaseConsumption.value(phase);
            if (phaseMarginPower < marginPower) {
                marginPower = phaseMarginPower;
            }
        }
    }
        qCDebug(dcConsolinnoEnergy()) << "Blackout protection: Maximum available power: " << marginPower << "W";

        foreach (Thing *thing, m_evChargers) {
            qCDebug(dcConsolinnoEnergy()) << "Blackout protection: Checking EV charger thing " << thing->name();
            absMax = thing->thingClass().stateTypes().findByName("maxChargingCurrent").maxValue().toFloat();
            absMin = thing->thingClass().stateTypes().findByName("maxChargingCurrent").minValue().toFloat();
            qCDebug(dcConsolinnoEnergy()) << "Blackout protection: Absolute limits are min. " << absMin << "A and max. " << absMax << "A.";
            currMax = thing->state("maxChargingCurrent").maxValue().toFloat();
            overshotCurrent = qRound(overshotPower / 230);
            if (limitExceeded) 
            {
                qCInfo(dcConsolinnoEnergy()) << "Blackout protection: Using at least" << overshotPower  << "W to much. Adjusting the evChargers...";
                thing->setStateMaxValue(thing->state("maxChargingCurrent").stateTypeId(), std::max(absMin, currMax - overshotCurrent - 1));
                qCInfo(dcConsolinnoEnergy()) << "Blackout protection: Ajdusted limit of charging current down to" <<  thing->state("maxChargingCurrent").maxValue().toInt() << "A";
            }else{
                if(currMax != absMax && marginPower > 250) {
                    thing->setStateMaxValue(thing->state("maxChargingCurrent").stateTypeId(), std::min(absMax, currMax + 1));
                    qCInfo(dcConsolinnoEnergy()) << "Blackout protection: Ajdusted limit of charging current up to" <<  thing->state("maxChargingCurrent").maxValue().toInt() << "A";
                }
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

    // Heating rod
    if (m_energyManager->rootMeter() && !m_inverters.isEmpty() && !m_heatingRods.isEmpty()) {
        // We need at least a root meter and and inverter for having the heating rod use case
        availableUseCases = availableUseCases.setFlag(HemsUseCaseHeatingRod);
    }

    // Dynamic electric pricing
    if (m_energyManager->rootMeter() && !m_dynamicElectricPricings.isEmpty()) {
        // We need at least a root meter for having the dynamic electric pricing use case
        availableUseCases = availableUseCases.setFlag(HemsUseCaseDynamicEPricing);
    }
    
    // Washing machine
    if (m_energyManager->rootMeter() && !m_inverters.isEmpty() && !m_washingMachines.isEmpty()) {
        // We need at least a root meter and and inverter for having the washing machine use case
        availableUseCases = availableUseCases.setFlag(HemsUseCaseWashingMachine);
    }

    // Charging
    if (m_energyManager->rootMeter() && !m_inverters.isEmpty() && !m_evChargers.isEmpty()) {
        // We need at least a root meter, an inverter and and ev charger for having the charging usecase
        availableUseCases = availableUseCases.setFlag(HemsUseCaseCharging);
    }

    // PV
    if (m_energyManager->rootMeter() && !m_inverters.isEmpty()){
        //we need atleast a root meter and an inverter to read pv data
        availableUseCases = availableUseCases.setFlag(HemsUseCasePv);
    }

    // Battery
    if (m_energyManager->rootMeter() && !m_batteries.isEmpty()){
        //we need atleast a root meter and an inverter to read pv data
        availableUseCases = availableUseCases.setFlag(HemsUseCaseBattery);
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
    // Disable optimization when car is unplugged for all modes except NO_OPTIMIZATION and SIMPLE_PV_EXCESS this is necessary to 
    // continoue charging when the car is plugged in again (simple charging)
    if (!(configuration.optimizationModeBase() == NO_OPTIMIZATION || configuration.optimizationModeBase() == SIMPLE_PV_EXCESS) 
            || configuration.optimizationModeBase() == DYN_PRICING) {
        configuration.setOptimizationEnabled(false);
        qCDebug(dcConsolinnoEnergy()) << "Setting OptimizationEnabled to false";
    }
    setChargingConfiguration(configuration);
    saveChargingConfigurationToSettings(configuration);
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
        configuration.setControllableLocalSystem(settings.value("controllableLocalSystem").toBool());
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
    settings.setValue("controllableLocalSystem", heatingConfiguration.controllableLocalSystem());
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

void EnergyEngine::loadHeatingRodConfiguration(const ThingId &heatingRodThingId)
{
    QSettings settings(NymeaSettings::settingsPath() + "/consolinno.conf", QSettings::IniFormat);
    settings.beginGroup("HeatingRodConfigurations");
    if (settings.childGroups().contains(heatingRodThingId.toString())) {
        settings.beginGroup(heatingRodThingId.toString());

        HeatingRodConfiguration configuration;
        configuration.setHeatingRodThingId(heatingRodThingId);
        configuration.setOptimizationEnabled(settings.value("optimizationEnabled").toBool());
        configuration.setMaxElectricalPower(settings.value("maxElectricalPower").toDouble());
        configuration.setControllableLocalSystem(settings.value("controllableLocalSystem").toBool());
        settings.endGroup(); // ThingId

        m_heatingRodConfigurations.insert(heatingRodThingId, configuration);
        emit heatingRodConfigurationAdded(configuration);

        qCDebug(dcConsolinnoEnergy()) << "Loaded" << configuration;
    } else {
        // HeatingRod usecase is available and this heat pump has no configuration yet, lets add one
        HeatingRodConfiguration configuration;
        configuration.setHeatingRodThingId(heatingRodThingId);
        m_heatingRodConfigurations.insert(heatingRodThingId, configuration);
        emit heatingRodConfigurationAdded(configuration);
        qCDebug(dcConsolinnoEnergy()) << "Added new" << configuration;
        saveHeatingRodConfigurationToSettings(configuration);
    }
    settings.endGroup(); // HeatingRodConfigurations
}


void EnergyEngine::saveHeatingRodConfigurationToSettings(const HeatingRodConfiguration &heatingRodConfiguration)
{
    QSettings settings(NymeaSettings::settingsPath() + "/consolinno.conf", QSettings::IniFormat);
    settings.beginGroup("HeatingRodConfigurations");
    settings.beginGroup(heatingRodConfiguration.heatingRodThingId().toString());
    settings.setValue("optimizationEnabled", heatingRodConfiguration.optimizationEnabled());
    settings.setValue("maxElectricalPower", heatingRodConfiguration.maxElectricalPower());
    settings.setValue("controllableLocalSystem", heatingRodConfiguration.controllableLocalSystem());
    settings.endGroup();
    settings.endGroup();
}

void EnergyEngine::removeHeatingRodConfigurationFromSettings(const ThingId &heatingRodThingId)
{
    QSettings settings(NymeaSettings::settingsPath() + "/consolinno.conf", QSettings::IniFormat);
    settings.beginGroup("HeatingRodConfigurations");
    settings.beginGroup(heatingRodThingId.toString());
    settings.remove("");
    settings.endGroup();
    settings.endGroup();
}

void EnergyEngine::loadDynamicElectricPricingConfiguration(const ThingId &dynamicElectricPricingThingId)
{
    QSettings settings(NymeaSettings::settingsPath() + "/consolinno.conf", QSettings::IniFormat);
    settings.beginGroup("DynamicElectricPricingConfigurations");
    if (settings.childGroups().contains(dynamicElectricPricingThingId.toString())) {
        settings.beginGroup(dynamicElectricPricingThingId.toString());

        DynamicElectricPricingConfiguration configuration;
        configuration.setDynamicElectricPricingThingId(dynamicElectricPricingThingId);
        configuration.setOptimizationEnabled(settings.value("optimizationEnabled").toBool());
        configuration.setMaxElectricalPower(settings.value("maxElectricalPower").toDouble());
        settings.endGroup(); // ThingId

        m_dynamicElectricPricingConfigurations.insert(dynamicElectricPricingThingId, configuration);
        emit dynamicElectricPricingConfigurationAdded(configuration);

        qCDebug(dcConsolinnoEnergy()) << "Loaded" << configuration;
    } else {
        // Dynamic Electric Pricing usecase is available and this heat pump has no configuration yet, lets add one
        DynamicElectricPricingConfiguration configuration;
        configuration.setDynamicElectricPricingThingId(dynamicElectricPricingThingId);
        m_dynamicElectricPricingConfigurations.insert(dynamicElectricPricingThingId, configuration);
        emit dynamicElectricPricingConfigurationAdded(configuration);
        qCDebug(dcConsolinnoEnergy()) << "Added new" << configuration;
        saveDynamicElectricPricingConfigurationToSettings(configuration);
    }
    settings.endGroup(); // DynamicElectricPricingConfigurations
}

void EnergyEngine::saveDynamicElectricPricingConfigurationToSettings(const DynamicElectricPricingConfiguration &dynamicElectricPricingConfiguration)
{
    QSettings settings(NymeaSettings::settingsPath() + "/consolinno.conf", QSettings::IniFormat);
    settings.beginGroup("DynamicElectricPricingConfigurations");
    settings.beginGroup(dynamicElectricPricingConfiguration.dynamicElectricPricingThingId().toString());
    settings.setValue("optimizationEnabled", dynamicElectricPricingConfiguration.optimizationEnabled());
    settings.setValue("maxElectricalPower", dynamicElectricPricingConfiguration.maxElectricalPower());
    settings.endGroup();
    settings.endGroup();
}

void EnergyEngine::removeDynamicElectricPricingConfigurationFromSettings(const ThingId &dynamicElectricPricingThingId)
{
    QSettings settings(NymeaSettings::settingsPath() + "/consolinno.conf", QSettings::IniFormat);
    settings.beginGroup("DynamicElectricPricingConfigurations");
    settings.beginGroup(dynamicElectricPricingThingId.toString());
    settings.remove("");
    settings.endGroup();
    settings.endGroup();
}

void EnergyEngine::loadWashingMachineConfiguration(const ThingId &washingMachineThingId)
{
    QSettings settings(NymeaSettings::settingsPath() + "/consolinno.conf", QSettings::IniFormat);
    settings.beginGroup("WashingMachineConfigurations");
    if (settings.childGroups().contains(washingMachineThingId.toString())) {
        settings.beginGroup(washingMachineThingId.toString());

        WashingMachineConfiguration configuration;
        configuration.setWashingMachineThingId(washingMachineThingId);
        configuration.setOptimizationEnabled(settings.value("optimizationEnabled").toBool());
        configuration.setMaxElectricalPower(settings.value("maxElectricalPower").toDouble());
        settings.endGroup(); // ThingId

        m_washingMachineConfigurations.insert(washingMachineThingId, configuration);
        emit washingMachineConfigurationAdded(configuration);

        qCDebug(dcConsolinnoEnergy()) << "Loaded" << configuration;
    } else {
        // WashingMachine usecase is available and this heat pump has no configuration yet, lets add one
        WashingMachineConfiguration configuration;
        configuration.setWashingMachineThingId(washingMachineThingId);
        m_washingMachineConfigurations.insert(washingMachineThingId, configuration);
        emit washingMachineConfigurationAdded(configuration);
        qCDebug(dcConsolinnoEnergy()) << "Added new" << configuration;
        saveWashingMachineConfigurationToSettings(configuration);
    }
    settings.endGroup(); // WashingMachineConfigurations
}

void EnergyEngine::saveWashingMachineConfigurationToSettings(const WashingMachineConfiguration &washingMachineConfiguration)
{
    QSettings settings(NymeaSettings::settingsPath() + "/consolinno.conf", QSettings::IniFormat);
    settings.beginGroup("WashingMachineConfigurations");
    settings.beginGroup(washingMachineConfiguration.washingMachineThingId().toString());
    settings.setValue("optimizationEnabled", washingMachineConfiguration.optimizationEnabled());
    settings.setValue("maxElectricalPower", washingMachineConfiguration.maxElectricalPower());
    settings.endGroup();
    settings.endGroup();
}

void EnergyEngine::removeWashingMachineConfigurationFromSettings(const ThingId &washingMachineThingId)
{
    QSettings settings(NymeaSettings::settingsPath() + "/consolinno.conf", QSettings::IniFormat);
    settings.beginGroup("WashingMachineConfigurations");
    settings.beginGroup(washingMachineThingId.toString());
    settings.remove("");
    settings.endGroup();
    settings.endGroup();
}

void EnergyEngine::loadUserConfiguration()
{
    QUuid userConfigID = "528b3820-1b6d-4f37-aea7-a99d21d42e72";
    QSettings settings(NymeaSettings::settingsPath() + "/consolinno.conf", QSettings::IniFormat);
    settings.beginGroup("UserConfigurations");
    if (settings.childGroups().contains(userConfigID.toString())) {
        settings.beginGroup(userConfigID.toString());

        UserConfiguration configuration;
        configuration.setLastSelectedCar(settings.value("lastSelectedCar").toUuid());
        configuration.setDefaultChargingMode(settings.value("defaultChargingMode").toInt());
        configuration.setInstallerName(settings.value("installerName").toString());
        configuration.setInstallerEmail(settings.value("installerEmail").toString());
        configuration.setInstallerPhoneNr(settings.value("installerPhoneNr").toString());
        configuration.setInstallerWorkplace(settings.value("installerWorkplace").toString());

        settings.endGroup(); // ThingId

        m_userConfigurations.insert(userConfigID, configuration);
        emit userConfigurationAdded(configuration);

        qCDebug(dcConsolinnoEnergy()) << "Loaded" << configuration;
    } else {
        // UserConfig does not exist yet, so add one
        UserConfiguration configuration;
        m_userConfigurations.insert(userConfigID, configuration);
        emit userConfigurationAdded(configuration);
        qCDebug(dcConsolinnoEnergy()) << "Added new" << configuration;
        saveUserConfigurationToSettings(configuration);
    }
    settings.endGroup(); // HeatingConfigurations
}

void EnergyEngine::saveUserConfigurationToSettings(const UserConfiguration &userConfiguration)
{
    qCDebug(dcConsolinnoEnergy()) << "saveUserConfiguration" << userConfiguration;
    QSettings settings(NymeaSettings::settingsPath() + "/consolinno.conf", QSettings::IniFormat);
    settings.beginGroup("UserConfigurations");
    settings.beginGroup(userConfiguration.userConfigID().toString());
    settings.setValue("lastSelectedCar", userConfiguration.lastSelectedCar());
    settings.setValue("defaultChargingMode", userConfiguration.defaultChargingMode());
    settings.setValue("installerName", userConfiguration.installerName());
    settings.setValue("installerEmail", userConfiguration.installerEmail());
    settings.setValue("installerPhoneNr", userConfiguration.installerPhoneNr());
    settings.setValue("installerWorkplace", userConfiguration.installerWorkplace());

    settings.endGroup();
    settings.endGroup();
}

void EnergyEngine::removeUserConfigurationFromSettings()
{
    QUuid userConfigID = "528b3820-1b6d-4f37-aea7-a99d21d42e72";
    QSettings settings(NymeaSettings::settingsPath() + "/consolinno.conf", QSettings::IniFormat);
    settings.beginGroup("UserConfigurations");
    settings.beginGroup(userConfigID.toString());
    settings.remove("");
    settings.endGroup();
    settings.endGroup();
}


void EnergyEngine::loadBatteryConfiguration(const ThingId &batteryThingId)
{

    QSettings settings(NymeaSettings::settingsPath() + "/consolinno.conf", QSettings::IniFormat);
    settings.beginGroup("BatteryConfigurations");
    if (settings.childGroups().contains(batteryThingId.toString())) {
        settings.beginGroup(batteryThingId.toString());

        BatteryConfiguration configuration;
        configuration.setBatteryThingId(batteryThingId);
        configuration.setOptimizationEnabled(settings.value("optimizationEnabled").toBool());
        configuration.setControllableLocalSystem(settings.value("controllableLocalSystem").toBool());
        settings.endGroup();

        m_batteryConfigurations.insert(batteryThingId, configuration);
        emit batteryConfigurationAdded(configuration);

        qCDebug(dcConsolinnoEnergy()) << "Loaded" << configuration;
    } else {
        // Battery usecase is available and this battery has no configuration yet, lets add one
        BatteryConfiguration configuration;
        configuration.setBatteryThingId(batteryThingId);
        m_batteryConfigurations.insert(batteryThingId, configuration);
        emit batteryConfigurationAdded(configuration);

        qCDebug(dcConsolinnoEnergy()) << "Added new" << configuration;
        saveBatteryConfigurationToSettings(configuration);
    }
    settings.endGroup(); // BatteryConfigurations
}

void EnergyEngine::saveBatteryConfigurationToSettings(const BatteryConfiguration &batteryConfiguration)
{
    QSettings settings(NymeaSettings::settingsPath() + "/consolinno.conf", QSettings::IniFormat);
    settings.beginGroup("BatteryConfigurations");
    settings.beginGroup(batteryConfiguration.batteryThingId().toString());
    settings.setValue("optimizationEnabled", batteryConfiguration.optimizationEnabled());
    settings.setValue("controllableLocalSystem", batteryConfiguration.controllableLocalSystem());
    settings.endGroup();
    settings.endGroup();
}

void EnergyEngine::removeBatteryConfigurationFromSettings(const ThingId &batteryThingId)
{
    QSettings settings(NymeaSettings::settingsPath() + "/consolinno.conf", QSettings::IniFormat);
    settings.beginGroup("BatteryConfigurations");
    settings.beginGroup(batteryThingId.toString());
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
        configuration.setControllableLocalSystem(settings.value("controllableLocalSystem").toBool());
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
    settings.setValue("controllableLocalSystem", chargingConfiguration.controllableLocalSystem());
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


void EnergyEngine::loadChargingOptimizationConfiguration(const ThingId &evChargerThingId)
{

    QSettings settings(NymeaSettings::settingsPath() + "/consolinno.conf", QSettings::IniFormat);
    settings.beginGroup("ChargingOptimizationConfigurations");
    if (settings.childGroups().contains(evChargerThingId.toString())) {
        settings.beginGroup(evChargerThingId.toString());

        ChargingOptimizationConfiguration configuration;
        configuration.setEvChargerThingId(evChargerThingId);
        configuration.setReenableChargepoint(settings.value("reenableChargepoint").toBool());
        configuration.setP_value(settings.value("p_value").toFloat());
        configuration.setI_value(settings.value("i_value").toFloat());
        configuration.setD_value(settings.value("d_value").toFloat());
        configuration.setSetpoint(settings.value("setpoint").toFloat());

        settings.endGroup();

        m_chargingOptimizationConfigurations.insert(evChargerThingId, configuration);
        emit chargingOptimizationConfigurationAdded(configuration);

        qCDebug(dcConsolinnoEnergy()) << "Loaded" << configuration;
    } else {
        // Charging usecase is available and this ev charger has no configuration yet, lets add one
        ChargingOptimizationConfiguration configuration;
        configuration.setEvChargerThingId(evChargerThingId);
        m_chargingOptimizationConfigurations.insert(evChargerThingId, configuration);
        emit chargingOptimizationConfigurationAdded(configuration);

        qCDebug(dcConsolinnoEnergy()) << "Added new" << configuration;
        saveChargingOptimizationConfigurationToSettings(configuration);
    }
    settings.endGroup(); // ChargingOptimizationConfigurations
}

void EnergyEngine::saveChargingOptimizationConfigurationToSettings(const ChargingOptimizationConfiguration &chargingOptimizationConfiguration)
{
    QSettings settings(NymeaSettings::settingsPath() + "/consolinno.conf", QSettings::IniFormat);
    settings.beginGroup("ChargingOptimizationConfigurations");
    settings.beginGroup(chargingOptimizationConfiguration.evChargerThingId().toString());
    settings.setValue("reenableChargepoint", chargingOptimizationConfiguration.reenableChargepoint());
    settings.setValue("p_value", chargingOptimizationConfiguration.p_value());
    settings.setValue("i_value", chargingOptimizationConfiguration.i_value());
    settings.setValue("d_value", chargingOptimizationConfiguration.d_value());
    settings.setValue("setpoint", chargingOptimizationConfiguration.setpoint());

    settings.endGroup();
    settings.endGroup();
}

void EnergyEngine::removeChargingOptimizationConfigurationFromSettings(const ThingId &evChargerThingId)
{
    QSettings settings(NymeaSettings::settingsPath() + "/consolinno.conf", QSettings::IniFormat);
    settings.beginGroup("ChargingOptimizationConfigurations");
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
    //qCDebug(dcConsolinnoEnergy()) << "Charging Session settings: " << settings.childGroups();
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

        //qCDebug(dcConsolinnoEnergy()) << "Loaded" << configuration;
    } else {
        // Charging usecase is available and this ev charger has no configuration yet, lets add one
        ChargingSessionConfiguration configuration;
        configuration.setEvChargerThingId(evChargerThingId);
        m_chargingSessionConfigurations.insert(evChargerThingId, configuration);
        emit chargingSessionConfigurationAdded(configuration);

        //qCDebug(dcConsolinnoEnergy()) << "Added new" << configuration;
        saveChargingSessionConfigurationToSettings(configuration);
    }
    settings.endGroup(); // ChargingSessionConfigurations
}

void EnergyEngine::saveChargingSessionConfigurationToSettings(const ChargingSessionConfiguration &chargingSessionConfiguration)
{
    //qCDebug(dcConsolinnoEnergy() ) << " saving ChargingSessionConfiguration" ;
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


