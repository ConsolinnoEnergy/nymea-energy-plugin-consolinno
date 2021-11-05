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

Q_DECLARE_LOGGING_CATEGORY(dcConsolinnoExperience)

ConsolinnoJsonHandler::ConsolinnoJsonHandler(EnergyEngine *energyEngine, QObject *parent) :
    JsonHandler(parent),
    m_energyEngine(energyEngine)
{
    // Enums / Flags
    registerEnum<EnergyEngine::HemsError>();
    registerEnum<EnergyEngine::HemsUseCase, EnergyEngine::HemsUseCases>();

    // Types
    registerObject<HeatingConfiguration>();
    registerObject<ChargingConfiguration>();

    // Methods
    QVariantMap params, returns;
    QString description;

    params.clear(); returns.clear();
    description = "Get the current available optimization UseCases based on the thing setup available in the system.";
    returns.insert("availableUseCases", enumRef<EnergyEngine::HemsUseCases>());
    registerMethod("GetAvailableUseCases", description, params, returns);

    params.clear(); returns.clear();
    description = "Get the list of available heating configurations from the energy engine.";
    returns.insert("heatingConfigurations", QVariantList() << objectRef<HeatingConfiguration>());
    registerMethod("GetHeatingConfigurations", description, params, returns);

    params.clear(); returns.clear();
    description = "Reconfigure a heating configuration to the given heating configuration. The heat pump thing ID will be used as an identifier.";
    params.insert("heatingConfiguration", objectRef<HeatingConfiguration>());
    returns.insert("hemsError", enumRef<EnergyEngine::HemsError>());
    registerMethod("SetHeatingConfiguration", description, params, returns);

    params.clear(); returns.clear();
    description = "Get the list of available charging configurations from the energy engine.";
    returns.insert("chargingConfigurations", QVariantList() << objectRef<ChargingConfiguration>());
    registerMethod("GetChargingConfigurations", description, params, returns);

    params.clear(); returns.clear();
    description = "Reconfigure a charging configuration to the given charging configuration. The ev charger thing ID will be used as an identifier.";
    params.insert("chargingConfiguration", objectRef<ChargingConfiguration>());
    returns.insert("hemsError", enumRef<EnergyEngine::HemsError>());
    registerMethod("SetChargingConfiguration", description, params, returns);


    // Notifications
    params.clear();
    description = "Emitted whenever the available energy uses cases in the energy engine have changed depending on the thing constelation.";
    params.insert("availableUseCases", enumRef<EnergyEngine::HemsUseCases>());
    registerNotification("AvailableUseCasesChanged", description, params);

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
        params.insert("availableUseCases", enumValueName(availableUseCases));
        emit AvailableUseCasesChanged(params);
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
    returns.insert("availableUseCases", enumValueName(m_energyEngine->availableUseCases()));
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

    return createReply(returns);
}

JsonReply *ConsolinnoJsonHandler::SetChargingConfiguration(const QVariantMap &params)
{
    EnergyEngine::HemsError error = m_energyEngine->setChargingConfiguration(unpack<ChargingConfiguration>(params.value("chargingConfiguration").toMap()));
    QVariantMap returns;
    returns.insert("hemsError", enumValueName(error));
    return createReply(returns);
}
