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

#ifndef ENERGYENGINE_H
#define ENERGYENGINE_H

#include <QHash>
#include <QTimer>
#include <QNetworkAccessManager>

#include <integrations/thingmanager.h>
#include <energymanager.h>

#include "configurations/chargingconfiguration.h"
#include "configurations/chargingsessionconfiguration.h"
#include "configurations/heatingconfiguration.h"
#include "configurations/pvconfiguration.h"
#include "configurations/conemsstate.h"
#include "configurations/userconfiguration.h"
#include "configurations/batteryconfiguration.h"


class EnergyEngine : public QObject
{
    Q_OBJECT
public:
    enum HemsError {
        HemsErrorNoError,
        HemsErrorInvalidParameter,
        HemsErrorInvalidThing,
        HemsErrorThingNotFound,
        HemsErrorInvalidPhaseLimit
    };
    Q_ENUM(HemsError)

    enum HemsUseCase {
        HemsUseCaseBlackoutProtection = 0x01,
        HemsUseCaseHeating = 0x02,
        HemsUseCaseCharging = 0x04,
        HemsUseCasePv = 0x08,
        HemsUseCaseBattery = 0x10
    };
    Q_ENUM(HemsUseCase)
    Q_DECLARE_FLAGS(HemsUseCases, HemsUseCase)
    Q_FLAG(HemsUseCases)

    explicit EnergyEngine(ThingManager *thingManager, EnergyManager *energyManager, QObject *parent = nullptr);

    EnergyEngine::HemsUseCases availableUseCases() const;

    uint housholdPhaseLimit() const;
    EnergyEngine::HemsError setHousholdPhaseLimit(uint housholdPhaseLimit);

    // User configurations
    QList<UserConfiguration> userConfigurations() const;
    EnergyEngine::HemsError setUserConfiguration(const UserConfiguration &userConfiguration);

    // Heating configurations
    QList<HeatingConfiguration> heatingConfigurations() const;
    EnergyEngine::HemsError setHeatingConfiguration(const HeatingConfiguration &heatingConfiguration);

    // Charging configurations
    QList<ChargingConfiguration> chargingConfigurations() const;
    EnergyEngine::HemsError setChargingConfiguration(const ChargingConfiguration &chargingConfiguration);

    // Battery configurations
    QList<BatteryConfiguration> batteryConfigurations() const;
    EnergyEngine::HemsError setBatteryConfiguration(const BatteryConfiguration &batteryConfiguration);

    // Charging Session configurations
    QList<ChargingSessionConfiguration> chargingSessionConfigurations() const;
    EnergyEngine::HemsError setChargingSessionConfiguration(const ChargingSessionConfiguration &chargingSessionConfiguration);

    // Pv configurations
    QList<PvConfiguration> pvConfigurations() const;
    EnergyEngine::HemsError setPvConfiguration(const PvConfiguration &pvConfiguration);

    // ConEMSStates
    ConEMSState ConemsState() const;
    EnergyEngine::HemsError setConEMSState(const ConEMSState &conEMSState);


signals:
    void availableUseCasesChanged(EnergyEngine::HemsUseCases availableUseCases);
    void housholdPhaseLimitChanged(uint housholdPhaseLimit);
    void pluggedInChanged(QVariant pluggedIn);


    void userConfigurationAdded(const UserConfiguration &userConfiguration);
    void userConfigurationChanged(const UserConfiguration &userConfiguration);
    void userConfigurationRemoved(const QUuid &userConfigID);

    void heatingConfigurationAdded(const HeatingConfiguration &heatingConfiguration);
    void heatingConfigurationChanged(const HeatingConfiguration &heatingConfiguration);
    void heatingConfigurationRemoved(const ThingId &heatPumpThingId);

    void chargingConfigurationAdded(const ChargingConfiguration &chargingConfiguration);
    void chargingConfigurationChanged(const ChargingConfiguration &chargingConfiguration);
    void chargingConfigurationRemoved(const ThingId &evChargerThingId);

    void batteryConfigurationAdded(const BatteryConfiguration &batteryConfiguration);
    void batteryConfigurationChanged(const BatteryConfiguration &batteryConfiguration);
    void batteryConfigurationRemoved(const ThingId &batteryThingId);

    void pvConfigurationAdded(const PvConfiguration &pvConfiguration);
    void pvConfigurationChanged(const PvConfiguration &pvConfiguration);
    void pvConfigurationRemoved(const ThingId &pvThingId);

    void chargingSessionConfigurationAdded(const ChargingSessionConfiguration &chargingSessionConfiguration);
    void chargingSessionConfigurationChanged(const ChargingSessionConfiguration &chargingSessionConfiguration);
    void chargingSessionConfigurationRemoved(const ThingId &evChargerThingId);

    void conEMSStatesAdded(const ConEMSState &conEMSState);
    void conEMSStatesChanged(const ConEMSState &conEMSState);
    void conEMSStatesRemoved(const QUuid &conEMSStateID);

private:
    ThingManager *m_thingManager = nullptr;
    EnergyManager *m_energyManager = nullptr;

    // System information
    HemsUseCases m_availableUseCases;
    uint m_housholdPhaseLimit = 25;
    uint m_housholdPhaseCount = 3;
    double m_housholdPowerLimit = m_housholdPhaseCount * m_housholdPhaseLimit;

    QHash<ThingId, HeatingConfiguration> m_heatingConfigurations;
    QHash<ThingId, ChargingConfiguration> m_chargingConfigurations;
    QHash<ThingId, BatteryConfiguration> m_batteryConfigurations;
    QHash<ThingId, PvConfiguration> m_pvConfigurations;
    QHash<ThingId, ChargingSessionConfiguration> m_chargingSessionConfigurations;
    QHash<QUuid, UserConfiguration> m_userConfigurations;
    ConEMSState m_conEMSState;

    QHash<ThingId, Thing *> m_inverters;
    QHash<ThingId, Thing *> m_heatPumps;
    QHash<ThingId, Thing *> m_evChargers;
    QHash<ThingId, Thing *> m_batteries;


    void monitorHeatPump(Thing *thing);
    void monitorInverter(Thing *thing);
    void monitorBattery(Thing *thing);
    void monitorEvCharger(Thing *thing);
    void monitorChargingSession(Thing *thing);
    void monitorUserConfig();

    void pluggedInEventHandling(Thing *thing);

private slots:
    void onThingAdded(Thing *thing);
    void onThingRemoved(const ThingId &thingId);

    void onRootMeterChanged();

    void evaluate();

    void evaluateAvailableUseCases();


    void loadUserConfiguration();
    void saveUserConfigurationToSettings(const UserConfiguration &userConfiguration);
    void removeUserConfigurationFromSettings();

    void loadHeatingConfiguration(const ThingId &heatPumpThingId);
    void saveHeatingConfigurationToSettings(const HeatingConfiguration &heatingConfiguration);
    void removeHeatingConfigurationFromSettings(const ThingId &heatPumpThingId);

    void loadChargingConfiguration(const ThingId &evChargerThingId);
    void saveChargingConfigurationToSettings(const ChargingConfiguration &chargingConfiguration);
    void removeChargingConfigurationFromSettings(const ThingId &evChargerThingId);

    void loadBatteryConfiguration(const ThingId &batteryThingId);
    void saveBatteryConfigurationToSettings(const BatteryConfiguration &batteryConfiguration);
    void removeBatteryConfigurationFromSettings(const ThingId &batteryThingId);

    void loadChargingSessionConfiguration(const ThingId &chargingSessionThingId);
    void saveChargingSessionConfigurationToSettings(const ChargingSessionConfiguration &chargingSessionConfiguration);
    void removeChargingSessionConfigurationFromSettings(const ThingId &chargingSessionThingId);

    void loadPvConfiguration(const ThingId &pvThingId);
    void savePvConfigurationToSettings(const PvConfiguration &pvConfiguration);
    void removePvConfigurationFromSettings(const ThingId &pvThingId);

};

Q_DECLARE_OPERATORS_FOR_FLAGS(EnergyEngine::HemsUseCases)

#endif // ENERGYENGINE_H
