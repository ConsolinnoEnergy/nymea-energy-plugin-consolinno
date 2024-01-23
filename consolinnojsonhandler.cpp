/* Copyright (C) Consolinno Energy GmbH - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include "consolinnojsonhandler.h"
#include "configurations/conemsstate.h"
#include "energyengine.h"
#include "energypluginconsolinno.h"
#include <QJsonDocument>
#include <QJsonParseError>

Q_DECLARE_LOGGING_CATEGORY(dcConsolinnoEnergy)

ConsolinnoJsonHandler::ConsolinnoJsonHandler(EnergyEngine *energyEngine, HEMSVersionInfo versionInfo, QObject *parent) :
    JsonHandler(parent),
    m_energyEngine(energyEngine),
    m_versionInfo(versionInfo)
{
    // Enums
    registerEnum<EnergyEngine::HemsError>();
    registerEnum<HeatingConfiguration::HouseType>();

    // Flags
    registerFlag<EnergyEngine::HemsUseCase, EnergyEngine::HemsUseCases>();

    // Types
    registerObject<HeatingConfiguration>();
    registerObject<HeatingRodConfiguration>();
    registerObject<WashingMachineConfiguration>();
    registerObject<ChargingConfiguration>();
    registerObject<ChargingOptimizationConfiguration>();
    registerObject<ChargingSessionConfiguration>();
    registerObject<PvConfiguration>();
    registerObject<ConEMSState>();
    registerObject<UserConfiguration>();
    registerObject<BatteryConfiguration>();
    registerObject<ConEMSState>();

    QVariantMap params, returns;
    QString description;




    // Methods
    //
    params.clear(); returns.clear();
    description = "Get the version of the HEMS system";
    registerMethod("GetHEMSVersion", description, params, returns);

    params.clear(); returns.clear();
    description = "Get the current available optimization UseCases based on the thing setup available in the system.";
    returns.insert("availableUseCases", flagRef<EnergyEngine::HemsUseCases>());
    registerMethod("GetAvailableUseCases", description, params, returns);

    params.clear(); returns.clear();
    description = "Get the houshold phase limit in amperes. This value is gonna be used by the blackout protection use case for limiting the phase current.";
    returns.insert("housholdPhaseLimit", enumValueName(Uint));
    registerMethod("GetHousholdPhaseLimit", description, params, returns);

    params.clear(); returns.clear();
    description = "Set the houshold phase limit in amperes. This value is gonna be used by the blackout protection use case for limiting the phase current.";
    params.insert("housholdPhaseLimit", enumValueName(Uint));
    returns.insert("hemsError", enumRef<EnergyEngine::HemsError>());
    registerMethod("SetHousholdPhaseLimit", description, params, returns);

    // UserConfig
    params.clear(); returns.clear();
    description = "Get the list of available heating configurations from the energy engine.";
    returns.insert("userConfigurations", QVariantList() << objectRef<UserConfiguration>());
    registerMethod("GetUserConfigurations", description, params, returns);

    params.clear(); returns.clear();
    description = "Update a heating configuration to the given heating configuration. The heat pump thing ID will be used as an identifier.";
    params.insert("userConfiguration", objectRef<UserConfiguration>());
    returns.insert("hemsError", enumRef<EnergyEngine::HemsError>());
    registerMethod("SetUserConfiguration", description, params, returns);

    // Heating
    params.clear(); returns.clear();
    description = "Get the list of available heating configurations from the energy engine.";
    returns.insert("heatingConfigurations", QVariantList() << objectRef<HeatingConfiguration>());
    registerMethod("GetHeatingConfigurations", description, params, returns);

    params.clear(); returns.clear();
    description = "Update a heating configuration to the given heating configuration. The heat pump thing ID will be used as an identifier.";
    params.insert("heatingConfiguration", objectRef<HeatingConfiguration>());
    returns.insert("hemsError", enumRef<EnergyEngine::HemsError>());
    registerMethod("SetHeatingConfiguration", description, params, returns);

    // Heating rod
    params.clear(); returns.clear();
    description = "Get the list of available heating rod configurations from the energy engine.";
    returns.insert("heatingRodConfigurations", QVariantList() << objectRef<HeatingRodConfiguration>());
    registerMethod("GetHeatingRodConfigurations", description, params, returns);

    params.clear(); returns.clear();
    description = "Update a heating rod configuration to the given heating rod configuration. The heating rod thing ID will be used as an identifier.";
    params.insert("heatingRodConfiguration", objectRef<HeatingRodConfiguration>());
    returns.insert("hemsError", enumRef<EnergyEngine::HemsError>());
    registerMethod("SetHeatingRodConfiguration", description, params, returns);

    // Washing machine
    params.clear(); returns.clear();
    description = "Get the list of available washing machine configurations from the energy engine.";
    returns.insert("washingMachineConfigurations", QVariantList() << objectRef<WashingMachineConfiguration>());
    registerMethod("GetWashingMachineConfigurations", description, params, returns);

    params.clear(); returns.clear();
    description = "Update a washing machine configuration to the given washing machine configuration. The washing machine thing ID will be used as an identifier.";
    params.insert("washingMachineConfiguration", objectRef<WashingMachineConfiguration>());
    returns.insert("hemsError", enumRef<EnergyEngine::HemsError>());
    registerMethod("SetWashingMachineConfiguration", description, params, returns);

    // ConEMS
    params.clear(); returns.clear();
    description = "Get the list of available ConEMSState from the energy engine.";
    returns.insert("conEMSState", QVariantList() << objectRef<ConEMSState>());
    registerMethod("GetConEMSState", description, params, returns);

    params.clear(); returns.clear();
    description = "Update the ConEMSState";
    params.insert("conEMSState", objectRef<ConEMSState>());
    returns.insert("hemsError", enumRef<EnergyEngine::HemsError>());
    registerMethod("SetConEMSState", description, params, returns);

    // PV
    params.clear(); returns.clear();
    description = "Get the list of available pv configurations from the energy engine.";
    returns.insert("pvConfigurations", QVariantList() << objectRef<PvConfiguration>());
    registerMethod("GetPvConfigurations", description, params, returns);

    params.clear(); returns.clear();
    description = "Update a pv configuration to the given pv configuration. The pv thing ID will be used as an identifier.";
    params.insert("pvConfiguration", objectRef<PvConfiguration>());
    returns.insert("hemsError", enumRef<EnergyEngine::HemsError>());
    registerMethod("SetPvConfiguration", description, params, returns);

    // chargingSession
    params.clear(); returns.clear();
    description = "Get the list of available chargingSession configurations from the energy engine.";
    returns.insert("chargingSessionConfigurations", QVariantList() << objectRef<ChargingSessionConfiguration>());
    registerMethod("GetChargingSessionConfigurations", description, params, returns);


    params.clear(); returns.clear();
    description = "Update a chargingSession configuration to the given chargingSession configuration. The chargingSession thing ID will be used as an identifier.";
    params.insert("chargingSessionConfiguration", objectRef<ChargingSessionConfiguration>());
    returns.insert("hemsError", enumRef<EnergyEngine::HemsError>());
    registerMethod("SetChargingSessionConfiguration", description, params, returns);

    // charging
    params.clear(); returns.clear();
    description = "Get the list of available charging optimizaton configurations from the energy engine.";
    returns.insert("chargingConfigurations", QVariantList() << objectRef<ChargingConfiguration>());
    registerMethod("GetChargingConfigurations", description, params, returns);

    params.clear(); returns.clear();
    description = "Update a charging configuration to the given charging optimization configuration. The ev charger thing ID will be used as an identifier.";
    params.insert("chargingConfiguration", objectRef<ChargingConfiguration>());
    returns.insert("hemsError", enumRef<EnergyEngine::HemsError>());
    registerMethod("SetChargingConfiguration", description, params, returns);


    // charging optimization
    params.clear(); returns.clear();
    description = "Get the list of available charging configurations from the energy engine.";
    returns.insert("chargingOptimizationConfigurations", QVariantList() << objectRef<ChargingOptimizationConfiguration>());
    registerMethod("GetChargingOptimizationConfigurations", description, params, returns);

    params.clear(); returns.clear();
    description = "Update a charging configuration to the given charging configuration. The ev charger thing ID will be used as an identifier.";
    params.insert("chargingOptimizationConfiguration", objectRef<ChargingOptimizationConfiguration>());
    returns.insert("hemsError", enumRef<EnergyEngine::HemsError>());
    registerMethod("SetChargingOptimizationConfiguration", description, params, returns);


    // battery
    params.clear(); returns.clear();
    description = "Get the list of available battery configurations from the energy engine.";
    returns.insert("batteryConfigurations", QVariantList() << objectRef<BatteryConfiguration>());
    registerMethod("GetBatteryConfigurations", description, params, returns);

    params.clear(); returns.clear();
    description = "Update a battery configuration to the given battery configuration. The battery thing ID will be used as an identifier.";
    params.insert("batteryConfiguration", objectRef<BatteryConfiguration>());
    returns.insert("hemsError", enumRef<EnergyEngine::HemsError>());
    registerMethod("SetBatteryConfiguration", description, params, returns);


    // Notifications
    params.clear();
    description = "Emitted whenever the available energy uses cases in the energy engine have changed depending on the thing constelation.";
    params.insert("availableUseCases", flagRef<EnergyEngine::HemsUseCases>());
    registerNotification("AvailableUseCasesChanged", description, params);


    params.clear();
    description = "Emitted whenever the houshold phase limit in amperes has changed.";
    params.insert("housholdPhaseLimit", enumValueName(Uint));
    registerNotification("HousholdPhaseLimitChanged", description, params);

    // UserConfig
    params.clear();
    description = "Emitted whenever a new user configuration has been added to the energy engine.";
    params.insert("userConfiguration", objectRef<UserConfiguration>());
    registerNotification("UserConfigurationAdded", description, params);

    params.clear();
    description = "Emitted whenever a user configuration has been removed from the energy engine with the given heat pump thing ID.";
    params.insert("userConfigID", enumValueName(Uuid));
    registerNotification("UserConfigurationRemoved", description, params);

    params.clear();
    description = "Emitted whenever a user configuration has changed in the energy engine.";
    params.insert("userConfiguration", objectRef<UserConfiguration>());
    registerNotification("UserConfigurationChanged", description, params);


    // Heating
    params.clear();
    description = "Emitted whenever a new heating configuration has been added to the energy engine.";
    params.insert("heatingConfiguration", objectRef<HeatingConfiguration>());
    registerNotification("HeatingConfigurationAdded", description, params);

    params.clear();
    description = "Emitted whenever a heating configuration has been removed from the energy engine with the given heat pump thing ID.";
    params.insert("heatPumpThingId", enumValueName(Uuid));
    registerNotification("HeatingConfigurationRemoved", description, params);

    params.clear();
    description = "Emitted whenever a heating configuration has changed in the energy engine.";
    params.insert("heatingConfiguration", objectRef<HeatingConfiguration>());
    registerNotification("HeatingConfigurationChanged", description, params);

    // Heating rod
    params.clear();
    description = "Emitted whenever a new heating rod configuration has been added to the energy engine.";
    params.insert("heatingRodConfiguration", objectRef<HeatingRodConfiguration>());
    registerNotification("HeatingRodConfigurationAdded", description, params);

    params.clear();
    description = "Emitted whenever a heating rod configuration has been removed from the energy engine with the given heat pump thing ID.";
    params.insert("heatingRodThingId", enumValueName(Uuid));
    registerNotification("HeatingRodConfigurationRemoved", description, params);

    params.clear();
    description = "Emitted whenever a heating rod configuration has changed in the energy engine.";
    params.insert("heatingRodConfiguration", objectRef<HeatingRodConfiguration>());
    registerNotification("HeatingRodConfigurationChanged", description, params);

    // Washing machine
    params.clear();
    description = "Emitted whenever a new washing machine configuration has been added to the energy engine.";
    params.insert("washingMachineonfiguration", objectRef<WashingMachineConfiguration>());
    registerNotification("WashingMachineConfigurationAdded", description, params);

    params.clear();
    description = "Emitted whenever a washing machine configuration has been removed from the energy engine with the given washing machine thing ID.";
    params.insert("heatingRodThingId", enumValueName(Uuid));
    registerNotification("WashingMachineConfigurationRemoved", description, params);

    params.clear();
    description = "Emitted whenever a washing machine configuration has changed in the energy engine.";
    params.insert("washingMachineConfiguration", objectRef<WashingMachineConfiguration>());
    registerNotification("WashingMachineConfigurationChanged", description, params);

    // ConEMS
    params.clear();
    description = "Emitted whenever a new ConEMSState has been added to the energy engine.";
    params.insert("conEMSState", objectRef<ConEMSState>());
    registerNotification("ConEMSStateAdded", description, params);

    params.clear();
    description = "Emitted whenever the ConEMSState has been removed from the energy engine";
    params.insert("conEMSStateID", enumValueName(Uuid));
    registerNotification("ConEMSStateRemoved", description, params);

    params.clear();
    description = "Emitted whenever a heating configuration has changed in the energy engine.";
    params.insert("conEMSState", objectRef<ConEMSState>());
    registerNotification("ConEMSStateChanged", description, params);

    // PV
    params.clear();
    description = "Emitted whenever a new pv configuration has been added to the energy engine.";
    params.insert("pvConfiguration", objectRef<PvConfiguration>());
    registerNotification("PvConfigurationAdded", description, params);

    params.clear();
    description = "Emitted whenever a pv configuration has been removed from the energy engine with the given pv thing ID.";
    params.insert("pvThingId", enumValueName(Uuid));
    registerNotification("PvConfigurationRemoved", description, params);

    params.clear();
    description = "Emitted whenever a pv configuration has changed in the energy engine.";
    params.insert("pvConfiguration", objectRef<PvConfiguration>());
    registerNotification("PvConfigurationChanged", description, params);

    // chargingSessionConfiguration
    params.clear();
    description = "Emitted whenever a new chargingsession configuration has been added to the energy engine.";
    params.insert("chargingSessionConfiguration", objectRef<ChargingSessionConfiguration>());
    registerNotification("ChargingSessionConfigurationAdded", description, params);

    params.clear();
    description = "Emitted whenever a chargingsession configuration has been removed from the energy engine with the given pv thing ID.";
    params.insert("evChargerThingId", enumValueName(Uuid));
    registerNotification("ChargingSessionConfigurationRemoved", description, params);

    params.clear();
    description = "Emitted whenever a chargingsession configuration has changed in the energy engine.";
    params.insert("chargingSessionConfiguration", objectRef<ChargingSessionConfiguration>());
    registerNotification("ChargingSessionConfigurationChanged", description, params);

    // Charging
    params.clear();
    description = "Emitted whenever a new charging configuration has been added to the energy engine.";
    params.insert("chargingConfiguration", objectRef<ChargingConfiguration>());
    registerNotification("ChargingConfigurationAdded", description, params);

    params.clear();
    description = "Emitted whenever a charging configuration has been removed from the energy engine with the given ev charger thing ID.";
    params.insert("evChargerThingId", enumValueName(Uuid));
    registerNotification("ChargingConfigurationRemoved", description, params);

    params.clear();
    description = "Emitted whenever a charging configuration has changed in the energy engine.";
    params.insert("chargingConfiguration", objectRef<ChargingConfiguration>());
    registerNotification("ChargingConfigurationChanged", description, params);

    // Charging optimization
    params.clear();
    description = "Emitted whenever a new charging Optimization configuration has been added to the energy engine.";
    params.insert("chargingOptimizationConfiguration", objectRef<ChargingOptimizationConfiguration>());
    registerNotification("ChargingOptimizationConfigurationAdded", description, params);

    params.clear();
    description = "Emitted whenever a charging Optimization configuration has been removed from the energy engine with the given ev charger thing ID.";
    params.insert("evChargerThingId", enumValueName(Uuid));
    registerNotification("ChargingOptimizationConfigurationRemoved", description, params);

    params.clear();
    description = "Emitted whenever a charging Optimization configuration has changed in the energy engine.";
    params.insert("chargingOptimizationConfiguration", objectRef<ChargingOptimizationConfiguration>());
    registerNotification("ChargingOptimizationConfigurationChanged", description, params);



    // Battery
    params.clear();
    description = "Emitted whenever a new battery configuration has been added to the energy engine.";
    params.insert("batteryConfiguration", objectRef<BatteryConfiguration>());
    registerNotification("BatteryConfigurationAdded", description, params);

    params.clear();
    description = "Emitted whenever a battery configuration has been removed from the energy engine with the given battery thing ID.";
    params.insert("batteryThingId", enumValueName(Uuid));
    registerNotification("BatteryConfigurationRemoved", description, params);

    params.clear();
    description = "Emitted whenever a battery configuration has changed in the energy engine.";
    params.insert("batteryConfiguration", objectRef<BatteryConfiguration>());
    registerNotification("BatteryConfigurationChanged", description, params);

    // plugged in event
    params.clear();
    description = "Emitted whenever the EvCharger state pluggedIn changes";
    params.insert("pluggedIn", enumValueName(Bool));
    registerNotification("PluggedInChanged", description, params);


    // Connections for the notification
/*  // not needed for now but can be interesting if the app needs to act and not the plugin
    connect(m_energyEngine, &EnergyEngine::pluggedInChanged, this, [=](QVariant pluggedIn){
        QVariantMap params;
        params.insert("pluggedIn", pluggedIn.toBool());
        qCWarning(dcConsolinnoEnergy()) << "The plugged in value has been Changed. jsonHandler";
        // ToDo: Here should the chargingconfig geändert werden.

        emit PluggedInChanged(params);
    });
*/
    connect(m_energyEngine, &EnergyEngine::availableUseCasesChanged, this, [=](EnergyEngine::HemsUseCases availableUseCases){
        QVariantMap params;
        params.insert("availableUseCases", flagValueNames(availableUseCases));
        emit AvailableUseCasesChanged(params);
    });

    connect(m_energyEngine, &EnergyEngine::housholdPhaseLimitChanged, this, [=](uint housholdPhaseLimit){
        QVariantMap params;
        params.insert("housholdPhaseLimit", housholdPhaseLimit);
        emit HousholdPhaseLimitChanged(params);
    });

    // UserConfig
    connect(m_energyEngine, &EnergyEngine::userConfigurationAdded, this, [=](const UserConfiguration &userConfiguration){
        QVariantMap params;
        params.insert("userConfiguration", pack(userConfiguration));
        emit UserConfigurationAdded(params);
    });

    connect(m_energyEngine, &EnergyEngine::userConfigurationRemoved, this, [=](const QUuid &userConfigID){
        QVariantMap params;
        params.insert("userConfigID", userConfigID);
        emit UserConfigurationRemoved(params);
    });

    connect(m_energyEngine, &EnergyEngine::userConfigurationChanged, this, [=](const UserConfiguration &userConfiguration){
        QVariantMap params;
        params.insert("userConfiguration", pack(userConfiguration));
        emit UserConfigurationChanged(params);
    });

    // Heating
    connect(m_energyEngine, &EnergyEngine::heatingConfigurationAdded, this, [=](const HeatingConfiguration &heatingConfiguration){
        QVariantMap params;
        params.insert("heatingConfiguration", pack(heatingConfiguration));
        emit HeatingConfigurationAdded(params);
    });

    connect(m_energyEngine, &EnergyEngine::heatingConfigurationRemoved, this, [=](const ThingId &heatPumpThingId){
        QVariantMap params;
        params.insert("heatPumpThingId", heatPumpThingId);
        emit HeatingConfigurationRemoved(params);
    });

    connect(m_energyEngine, &EnergyEngine::heatingConfigurationChanged, this, [=](const HeatingConfiguration &heatingConfiguration){
        QVariantMap params;
        params.insert("heatingConfiguration", pack(heatingConfiguration));
        emit HeatingConfigurationChanged(params);
    });


    // Heating rod
    connect(m_energyEngine, &EnergyEngine::heatingRodConfigurationAdded, this, [=](const HeatingRodConfiguration &heatingRodConfiguration){
        QVariantMap params;
        params.insert("heatingRodConfiguration", pack(heatingRodConfiguration));
        emit HeatingRodConfigurationAdded(params);
    });

    connect(m_energyEngine, &EnergyEngine::heatingRodConfigurationRemoved, this, [=](const ThingId &heatingRodThingId){
        QVariantMap params;
        params.insert("heatingRodThingId", heatingRodThingId);
        emit HeatingRodConfigurationRemoved(params);
    });

    connect(m_energyEngine, &EnergyEngine::heatingRodConfigurationChanged, this, [=](const HeatingRodConfiguration &heatingRodConfiguration){
        QVariantMap params;
        params.insert("heatingRodConfiguration", pack(heatingRodConfiguration));
        emit HeatingRodConfigurationChanged(params);
    });

    // Washing machine
    connect(m_energyEngine, &EnergyEngine::washingMachineConfigurationAdded, this, [=](const WashingMachineConfiguration &washingMachineConfiguration){
        QVariantMap params;
        params.insert("washingMachineConfiguration", pack(washingMachineConfiguration));
        emit WashingMachineConfigurationAdded(params);
    });

    connect(m_energyEngine, &EnergyEngine::washingMachineConfigurationRemoved, this, [=](const ThingId &washingMachineThingId){
        QVariantMap params;
        params.insert("washingMachineThingId", washingMachineThingId);
        emit WashingMachineConfigurationRemoved(params);
    });

    connect(m_energyEngine, &EnergyEngine::washingMachineConfigurationChanged, this, [=](const WashingMachineConfiguration &washingMachineConfiguration){
        QVariantMap params;
        params.insert("washingMachineConfiguration", pack(washingMachineConfiguration));
        emit WashingMachineConfigurationChanged(params);
    });

    //ConEMS
    connect(m_energyEngine, &EnergyEngine::conEMSStateAdded, this, [=](const ConEMSState &conEMSState){
        QVariantMap params;
        params.insert("conEMSState", pack(conEMSState));
        emit ConEMSStateAdded(params);
    });

    connect(m_energyEngine, &EnergyEngine::conEMSStateRemoved, this, [=](const QUuid &conEMSStateID){
        QVariantMap params;
        params.insert("conEMSStateID", conEMSStateID);
        emit ConEMSStateRemoved(params);
    });

    connect(m_energyEngine, &EnergyEngine::conEMSStateChanged, this, [=](const ConEMSState &conEMSState){
        QVariantMap params;
        params.insert("conEMSState", pack(conEMSState));
        emit ConEMSStateChanged(params);
    });



    connect(m_energyEngine, &EnergyEngine::pvConfigurationAdded, this, [=](const PvConfiguration &pvConfiguration){

        QVariantMap params;
       params.insert("pvConfiguration", pack(pvConfiguration));
        emit PvConfigurationAdded(params);
    });

    connect(m_energyEngine, &EnergyEngine::pvConfigurationRemoved, this, [=](const ThingId &pvThingId){
        QVariantMap params;
        params.insert("pvThingId", pvThingId);
        emit PvConfigurationRemoved(params);
    });

    connect(m_energyEngine, &EnergyEngine::pvConfigurationChanged, this, [=](const PvConfiguration &pvConfiguration){
        QVariantMap params;
        params.insert("pvConfiguration", pack(pvConfiguration));
        emit PvConfigurationChanged(params);
    });


    connect(m_energyEngine, &EnergyEngine::chargingSessionConfigurationAdded, this, [=](const ChargingSessionConfiguration &chargingSessionConfiguration){

        QVariantMap params;
       params.insert("chargingSessionConfiguration", pack(chargingSessionConfiguration));
        emit ChargingSessionConfigurationAdded(params);
    });

    connect(m_energyEngine, &EnergyEngine::chargingSessionConfigurationRemoved, this, [=](const ThingId &evChargerThingId){
        QVariantMap params;
        params.insert("evChargerThingId", evChargerThingId);
        emit ChargingSessionConfigurationRemoved(params);
    });

    connect(m_energyEngine, &EnergyEngine::chargingSessionConfigurationChanged, this, [=](const ChargingSessionConfiguration &chargingSessionConfiguration){
        QVariantMap params;
        params.insert("chargingSessionConfiguration", pack(chargingSessionConfiguration));
        emit ChargingSessionConfigurationChanged(params);
    });

    // Charging connections
    connect(m_energyEngine, &EnergyEngine::chargingConfigurationAdded, this, [=](const ChargingConfiguration &chargingConfiguration){
        QVariantMap params;
        params.insert("chargingConfiguration", pack(chargingConfiguration));
        emit ChargingConfigurationAdded(params);
    });

    connect(m_energyEngine, &EnergyEngine::chargingConfigurationRemoved, this, [=](const ThingId &evChargerThingId){
        QVariantMap params;
        params.insert("evChargerThingId", evChargerThingId);
        emit ChargingConfigurationRemoved(params);
    });

    connect(m_energyEngine, &EnergyEngine::chargingConfigurationChanged, this, [=](const ChargingConfiguration &chargingConfiguration){
        QVariantMap params;
        params.insert("chargingConfiguration", pack(chargingConfiguration));
        emit ChargingConfigurationChanged(params);
    });

    // Charging optimization connections
    connect(m_energyEngine, &EnergyEngine::chargingOptimizationConfigurationAdded, this, [=](const ChargingOptimizationConfiguration &chargingOptimizationConfiguration){
        QVariantMap params;
        params.insert("chargingOptimizationConfiguration", pack(chargingOptimizationConfiguration));
        emit ChargingOptimizationConfigurationAdded(params);
    });

    connect(m_energyEngine, &EnergyEngine::chargingOptimizationConfigurationRemoved, this, [=](const ThingId &evChargerThingId){
        QVariantMap params;
        params.insert("evChargerThingId", evChargerThingId);
        emit ChargingOptimizationConfigurationRemoved(params);
    });

    connect(m_energyEngine, &EnergyEngine::chargingOptimizationConfigurationChanged, this, [=](const ChargingOptimizationConfiguration &chargingOptimizationConfiguration){
        QVariantMap params;
        params.insert("chargingOptimizationConfiguration", pack(chargingOptimizationConfiguration));
        emit ChargingOptimizationConfigurationChanged(params);
    });


    connect(m_energyEngine, &EnergyEngine::batteryConfigurationAdded, this, [=](const BatteryConfiguration &batteryConfiguration){
        QVariantMap params;
        params.insert("batteryConfiguration", pack(batteryConfiguration));
        emit BatteryConfigurationAdded(params);
    });

    connect(m_energyEngine, &EnergyEngine::batteryConfigurationRemoved, this, [=](const ThingId &batteryThingId){
        QVariantMap params;
        params.insert("batteryThingId", batteryThingId);
        emit BatteryConfigurationRemoved(params);
    });

    connect(m_energyEngine, &EnergyEngine::batteryConfigurationChanged, this, [=](const BatteryConfiguration &batteryConfiguration){
        QVariantMap params;
        params.insert("batteryConfiguration", pack(batteryConfiguration));
        emit BatteryConfigurationChanged(params);
    });

}

QString ConsolinnoJsonHandler::name() const
{
    return "Hems";
}

JsonReply *ConsolinnoJsonHandler::GetHEMSVersion(const QVariantMap &params)
{
    Q_UNUSED(params)
    QVariantMap returns;
    returns.insert("product", m_versionInfo.product);
    returns.insert("version", m_versionInfo.version);
    returns.insert("stage", m_versionInfo.stage);
    return createReply(returns);
}

JsonReply *ConsolinnoJsonHandler::GetAvailableUseCases(const QVariantMap &params)
{
    Q_UNUSED(params)

    QVariantMap returns;
    returns.insert("availableUseCases", flagValueNames(m_energyEngine->availableUseCases()));
    return createReply(returns);
}

JsonReply *ConsolinnoJsonHandler::GetHousholdPhaseLimit(const QVariantMap &params)
{
    Q_UNUSED(params)

    QVariantMap returns;
    returns.insert("housholdPhaseLimit", m_energyEngine->housholdPhaseLimit());
    return createReply(returns);
}

JsonReply *ConsolinnoJsonHandler::SetHousholdPhaseLimit(const QVariantMap &params)
{
    uint phaseLimit = params.value("housholdPhaseLimit").toUInt();

    QVariantMap returns;
    returns.insert("hemsError", enumValueName(m_energyEngine->setHousholdPhaseLimit(phaseLimit)));
    return createReply(returns);
}




JsonReply *ConsolinnoJsonHandler::GetPvConfigurations(const QVariantMap &params)
{

    //qCDebug(dcConsolinnoEnergy()) << "GetPvConfigurationJsonHandler:" << params << "\n";
    Q_UNUSED(params)
    QVariantMap returns;
    QVariantList configurations;
    foreach (const PvConfiguration &pvConfig, m_energyEngine->pvConfigurations()) { 
        configurations << pack(pvConfig);

        }
    returns.insert("pvConfigurations", configurations);

    return createReply(returns);
}

JsonReply *ConsolinnoJsonHandler::SetPvConfiguration(const QVariantMap &params)
{
   EnergyEngine::HemsError error = m_energyEngine->setPvConfiguration(unpack<PvConfiguration>(params.value("pvConfiguration").toMap()));
   QVariantMap returns;
   returns.insert("hemsError", enumValueName(error));
   return createReply(returns);
}

// UserConfig
JsonReply *ConsolinnoJsonHandler::GetUserConfigurations(const QVariantMap &params)
{
    Q_UNUSED(params)
    QVariantMap returns;
    QVariantList configurations;
    foreach (const UserConfiguration &userConfig, m_energyEngine->userConfigurations()) {
        configurations << pack(userConfig);
    }
    returns.insert("userConfigurations", configurations);
    return createReply(returns);
}

JsonReply *ConsolinnoJsonHandler::SetUserConfiguration(const QVariantMap &params)
{
    EnergyEngine::HemsError error = m_energyEngine->setUserConfiguration(unpack<UserConfiguration>(params.value("userConfiguration").toMap()));
    QVariantMap returns;
    returns.insert("hemsError", enumValueName(error));
    return createReply(returns);
}

// Heating
JsonReply *ConsolinnoJsonHandler::GetHeatingConfigurations(const QVariantMap &params)
{
    Q_UNUSED(params)
    QVariantMap returns;
    QVariantList configurations;
    foreach (const HeatingConfiguration &heatingConfig, m_energyEngine->heatingConfigurations()) {
        configurations << pack(heatingConfig);
    }
    returns.insert("heatingConfigurations", configurations);
    return createReply(returns);
}

JsonReply *ConsolinnoJsonHandler::SetHeatingConfiguration(const QVariantMap &params)
{
    EnergyEngine::HemsError error = m_energyEngine->setHeatingConfiguration(unpack<HeatingConfiguration>(params.value("heatingConfiguration").toMap()));
    QVariantMap returns;
    returns.insert("hemsError", enumValueName(error));
    return createReply(returns);
}


// Heating rod
JsonReply *ConsolinnoJsonHandler::GetHeatingRodConfigurations(const QVariantMap &params)
{
    Q_UNUSED(params)
    QVariantMap returns;
    QVariantList configurations;
    foreach (const HeatingRodConfiguration &heatingRodConfig, m_energyEngine->heatingRodConfigurations()) {
        configurations << pack(heatingRodConfig);
    }
    returns.insert("heatingRodConfigurations", configurations);
    return createReply(returns);
}

JsonReply *ConsolinnoJsonHandler::SetHeatingRodConfiguration(const QVariantMap &params)
{
    EnergyEngine::HemsError error = m_energyEngine->setHeatingRodConfiguration(unpack<HeatingRodConfiguration>(params.value("heatingRodConfiguration").toMap()));
    QVariantMap returns;
    returns.insert("hemsError", enumValueName(error));
    return createReply(returns);
}

// Washing machine
JsonReply *ConsolinnoJsonHandler::GetWashingMachineConfigurations(const QVariantMap &params)
{
    Q_UNUSED(params)
    QVariantMap returns;
    QVariantList configurations;
    foreach (const WashingMachineConfiguration &washingMachineConfig, m_energyEngine->washingMachineConfigurations()) {
        configurations << pack(washingMachineConfig);
    }
    returns.insert("washingMachineConfigurations", configurations);
    return createReply(returns);
}

JsonReply *ConsolinnoJsonHandler::SetWashingMachineConfiguration(const QVariantMap &params)
{
    EnergyEngine::HemsError error = m_energyEngine->setWashingMachineConfiguration(unpack<WashingMachineConfiguration>(params.value("washingMachineConfiguration").toMap()));
    QVariantMap returns;
    returns.insert("hemsError", enumValueName(error));
    return createReply(returns);
}

//ConEMS
JsonReply *ConsolinnoJsonHandler::GetConEMSState(const QVariantMap &params)
{
    Q_UNUSED(params)
    QVariantMap returns;
    QVariantList Cstates;

    Cstates << pack(m_energyEngine->ConemsState());

    returns.insert("conEMSState", Cstates);
    return createReply(returns);
}

JsonReply *ConsolinnoJsonHandler::SetConEMSState(const QVariantMap &params)
{
    // unpacking json payload to a JSON object seems not work using the unpack macro
    // and results in an empty object...
    // Let's do it manually
    QJsonParseError err;
    QJsonDocument jsonResponse = QJsonDocument::fromJson(params.value("conEMSState").toMap().value("currentState").toString().toUtf8(), &err);
    QVariantMap returns;
    if (err.error != QJsonParseError::NoError) {
        qCWarning(dcConsolinnoEnergy()) << "Error parsing json: " << err.errorString();
        returns.insert("hemsError", EnergyEngine::HemsError::HemsErrorInvalidParameter);
        return createReply(returns);
    }    
    ConEMSState conemsstate = unpack<ConEMSState>(params.value("conEMSState").toMap());
    conemsstate.setCurrentState(jsonResponse.object());
    EnergyEngine::HemsError error = m_energyEngine->setConEMSState(conemsstate);
    returns.insert("hemsError", error);
    return createReply(returns);
}



JsonReply *ConsolinnoJsonHandler::GetChargingConfigurations(const QVariantMap &params)
{
    Q_UNUSED(params)

    QVariantMap returns;
    QVariantList configurations;
    foreach (const ChargingConfiguration &chargingConfig, m_energyEngine->chargingConfigurations()) {
        configurations << pack(chargingConfig);
    }
    returns.insert("chargingConfigurations", configurations);

    return createReply(returns);
}

JsonReply *ConsolinnoJsonHandler::SetChargingConfiguration(const QVariantMap &params)
{
    //qCDebug(dcConsolinnoEnergy()) << params;
    EnergyEngine::HemsError error = m_energyEngine->setChargingConfiguration(unpack<ChargingConfiguration>(params.value("chargingConfiguration").toMap()));
    QVariantMap returns;
    returns.insert("hemsError", enumValueName(error));
    return createReply(returns);
}


JsonReply *ConsolinnoJsonHandler::GetChargingOptimizationConfigurations(const QVariantMap &params)
{
    Q_UNUSED(params)

    QVariantMap returns;
    QVariantList configurations;
    foreach (const ChargingOptimizationConfiguration &chargingOptimizationConfig, m_energyEngine->chargingOptimizationConfigurations()) {
        configurations << pack(chargingOptimizationConfig);
    }
    returns.insert("chargingOptimizationConfigurations", configurations);

    return createReply(returns);
}

JsonReply *ConsolinnoJsonHandler::SetChargingOptimizationConfiguration(const QVariantMap &params)
{
    qCDebug(dcConsolinnoEnergy()) << params;
    EnergyEngine::HemsError error = m_energyEngine->setChargingOptimizationConfiguration(unpack<ChargingOptimizationConfiguration>(params.value("chargingOptimizationConfiguration").toMap()));
    QVariantMap returns;
    returns.insert("hemsError", enumValueName(error));
    return createReply(returns);
}



JsonReply *ConsolinnoJsonHandler::GetBatteryConfigurations(const QVariantMap &params)
{
    Q_UNUSED(params)

    QVariantMap returns;
    QVariantList configurations;
    foreach (const BatteryConfiguration &batteryConfig, m_energyEngine->batteryConfigurations()) {
        configurations << pack(batteryConfig);
    }
    returns.insert("batteryConfigurations", configurations);

    return createReply(returns);
}

JsonReply *ConsolinnoJsonHandler::SetBatteryConfiguration(const QVariantMap &params)
{
    qCDebug(dcConsolinnoEnergy()) << params;
    EnergyEngine::HemsError error = m_energyEngine->setBatteryConfiguration(unpack<BatteryConfiguration>(params.value("batteryConfiguration").toMap()));
    QVariantMap returns;
    returns.insert("hemsError", enumValueName(error));
    return createReply(returns);
}


JsonReply *ConsolinnoJsonHandler::GetChargingSessionConfigurations(const QVariantMap &params)
{
    Q_UNUSED(params)

    QVariantMap returns;
    QVariantList configurations;
    foreach (const ChargingSessionConfiguration &chargingSessionConfig, m_energyEngine->chargingSessionConfigurations()) {
        configurations << pack(chargingSessionConfig);
    }
    returns.insert("chargingSessionConfigurations", configurations);

    return createReply(returns);
}

JsonReply *ConsolinnoJsonHandler::SetChargingSessionConfiguration(const QVariantMap &params)
{
    //qCDebug(dcConsolinnoEnergy()) << params;
    EnergyEngine::HemsError error = m_energyEngine->setChargingSessionConfiguration(unpack<ChargingSessionConfiguration>(params.value("chargingSessionConfiguration").toMap()));
    QVariantMap returns;
    returns.insert("hemsError", enumValueName(error));
    return createReply(returns);
}


