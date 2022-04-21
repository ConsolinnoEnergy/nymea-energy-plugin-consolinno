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

#include "consolinnojsonhandler.h"
#include "energyengine.h"

Q_DECLARE_LOGGING_CATEGORY(dcConsolinnoEnergy)

ConsolinnoJsonHandler::ConsolinnoJsonHandler(EnergyEngine *energyEngine, QObject *parent) :
    JsonHandler(parent),
    m_energyEngine(energyEngine)
{
    // Enums
    registerEnum<EnergyEngine::HemsError>();
    registerEnum<HemsOptimizerInterface::HouseType>();

    // Flags
    registerFlag<EnergyEngine::HemsUseCase, EnergyEngine::HemsUseCases>();

    // Types
    registerObject<HeatingConfiguration>();
    registerObject<ChargingConfiguration>();
    registerObject<ChargingSessionConfiguration>();
    registerObject<PvConfiguration>();

    QVariantMap params, returns;
    QString description;

    // Methods
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


    params.clear(); returns.clear();
    description = "Get the list of available heating configurations from the energy engine.";
    returns.insert("heatingConfigurations", QVariantList() << objectRef<HeatingConfiguration>());
    registerMethod("GetHeatingConfigurations", description, params, returns);

    params.clear(); returns.clear();
    description = "Update a heating configuration to the given heating configuration. The heat pump thing ID will be used as an identifier.";
    params.insert("heatingConfiguration", objectRef<HeatingConfiguration>());
    returns.insert("hemsError", enumRef<EnergyEngine::HemsError>());
    registerMethod("SetHeatingConfiguration", description, params, returns);


    params.clear(); returns.clear();
    description = "Get the list of available pv configurations from the energy engine.";
    returns.insert("pvConfigurations", QVariantList() << objectRef<PvConfiguration>());
    registerMethod("GetPvConfigurations", description, params, returns);

    params.clear(); returns.clear();
    description = "Update a pv configuration to the given pv configuration. The pv thing ID will be used as an identifier.";
    params.insert("pvConfiguration", objectRef<PvConfiguration>());
    returns.insert("hemsError", enumRef<EnergyEngine::HemsError>());
    registerMethod("SetPvConfiguration", description, params, returns);

    params.clear(); returns.clear();
    description = "Get the list of available chargingSession configurations from the energy engine.";
    returns.insert("chargingSessionConfiguration", QVariantList() << objectRef<ChargingSessionConfiguration>());
    registerMethod("GetChargingSessionConfigurations", description, params, returns);


    params.clear(); returns.clear();
    description = "Update a chargingSession configuration to the given chargingSession configuration. The chargingSession thing ID will be used as an identifier.";
    params.insert("chargingSessionConfiguration", objectRef<ChargingSessionConfiguration>());
    returns.insert("hemsError", enumRef<EnergyEngine::HemsError>());
    registerMethod("SetChargingSessionConfiguration", description, params, returns);

    params.clear(); returns.clear();
    description = "Get the list of available charging configurations from the energy engine.";
    returns.insert("chargingConfigurations", QVariantList() << objectRef<ChargingConfiguration>());
    registerMethod("GetChargingConfigurations", description, params, returns);

    params.clear(); returns.clear();
    description = "Update a charging configuration to the given charging configuration. The ev charger thing ID will be used as an identifier.";
    params.insert("chargingConfiguration", objectRef<ChargingConfiguration>());
    returns.insert("hemsError", enumRef<EnergyEngine::HemsError>());
    registerMethod("SetChargingConfiguration", description, params, returns);


    // Notifications
    params.clear();
    description = "Emitted whenever the available energy uses cases in the energy engine have changed depending on the thing constelation.";
    params.insert("availableUseCases", flagRef<EnergyEngine::HemsUseCases>());
    registerNotification("AvailableUseCasesChanged", description, params);


    params.clear();
    description = "Emitted whenever the houshold phase limit in amperes has changed.";
    params.insert("housholdPhaseLimit", enumValueName(Uint));
    registerNotification("HousholdPhaseLimitChanged", description, params);


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

    // Connections for the notification
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
        params.insert("evCharger", evChargerThingId);
        emit ChargingSessionConfigurationRemoved(params);
    });

    connect(m_energyEngine, &EnergyEngine::chargingSessionConfigurationChanged, this, [=](const ChargingSessionConfiguration &chargingSessionConfiguration){
        QVariantMap params;
        params.insert("chargingSessionConfiguration", pack(chargingSessionConfiguration));
        emit ChargingSessionConfigurationChanged(params);
    });


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
}

QString ConsolinnoJsonHandler::name() const
{
    return "Hems";
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

    qCDebug(dcConsolinnoEnergy()) << "GetPvConfigurationJsonHandler:" << params << "\n";
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
    qCDebug(dcConsolinnoEnergy()) << params;
    EnergyEngine::HemsError error = m_energyEngine->setChargingConfiguration(unpack<ChargingConfiguration>(params.value("chargingConfiguration").toMap()));
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
    qCDebug(dcConsolinnoEnergy()) << params;
    EnergyEngine::HemsError error = m_energyEngine->setChargingSessionConfiguration(unpack<ChargingSessionConfiguration>(params.value("chargingSessionConfiguration").toMap()));
    QVariantMap returns;
    returns.insert("hemsError", enumValueName(error));
    return createReply(returns);
}


