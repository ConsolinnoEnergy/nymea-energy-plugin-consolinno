/* Copyright (C) Consolinno Energy GmbH - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef ENERGYENGINE_H
#define ENERGYENGINE_H

#include <QHash>
#include <QTimer>
#include <QNetworkAccessManager>

#include <integrations/thingmanager.h>
#include <integrations/integrationplugin.h>
#include <energymanager.h>

#include "configurations/chargingconfiguration.h"
#include "configurations/chargingoptimizationconfiguration.h"
#include "configurations/chargingsessionconfiguration.h"
#include "configurations/heatingconfiguration.h"
#include "configurations/heatingrodconfiguration.h"
#include "configurations/washingmachineconfiguration.h"
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
        HemsUseCaseBattery = 0x10,
        HemsUseCaseHeatingRod = 0x20,
        HemsUseCaseWashingMachine = 0x30
    };
    Q_ENUM(HemsUseCase)

    enum ChargingMode {
        NO_OPTIMIZATION = 0,
        PV_OPTIMIZED = 1,
        PV_EXCESS = 2,
        SIMPLE_PV_EXCESS = 3
    };
    Q_ENUM(ChargingMode)

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

    // Heating rod configurations
    QList<HeatingRodConfiguration> heatingRodConfigurations() const;
    EnergyEngine::HemsError setHeatingRodConfiguration(const HeatingRodConfiguration &heatingRodConfiguration);
    
    // Washing machine configurations
    QList<WashingMachineConfiguration> washingMachineConfigurations() const;
    EnergyEngine::HemsError setWashingMachineConfiguration(const WashingMachineConfiguration &washingMachineConfiguration);

    // Charging configurations
    QList<ChargingConfiguration> chargingConfigurations() const;
    EnergyEngine::HemsError setChargingConfiguration(const ChargingConfiguration &chargingConfiguration);

    // Charging optimization configurations
    QList<ChargingOptimizationConfiguration> chargingOptimizationConfigurations() const;
    EnergyEngine::HemsError setChargingOptimizationConfiguration(const ChargingOptimizationConfiguration &chargingOptimizationConfiguration);

    // Battery configurations
    QList<BatteryConfiguration> batteryConfigurations() const;
    EnergyEngine::HemsError setBatteryConfiguration(const BatteryConfiguration &batteryConfiguration);

    // Charging Session configurations
    QList<ChargingSessionConfiguration> chargingSessionConfigurations() const;
    EnergyEngine::HemsError setChargingSessionConfiguration(const ChargingSessionConfiguration &chargingSessionConfiguration);

    // Pv configurations
    QList<PvConfiguration> pvConfigurations() const;
    EnergyEngine::HemsError setPvConfiguration(const PvConfiguration &pvConfiguration);

    // ConEMSState
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

    void heatingRodConfigurationAdded(const HeatingRodConfiguration &heatingRodConfiguration);
    void heatingRodConfigurationChanged(const HeatingRodConfiguration &heatingRodConfiguration);
    void heatingRodConfigurationRemoved(const ThingId &heatingRodThingId);

    void washingMachineConfigurationAdded(const WashingMachineConfiguration &washingMachineConfiguration);
    void washingMachineConfigurationChanged(const WashingMachineConfiguration &washingMachineConfiguration);
    void washingMachineConfigurationRemoved(const ThingId &washingMachineThingId);

    void chargingConfigurationAdded(const ChargingConfiguration &chargingConfiguration);
    void chargingConfigurationChanged(const ChargingConfiguration &chargingConfiguration);
    void chargingConfigurationRemoved(const ThingId &evChargerThingId);

    void chargingOptimizationConfigurationAdded(const ChargingOptimizationConfiguration &chargingOptimizationConfiguration);
    void chargingOptimizationConfigurationChanged(const ChargingOptimizationConfiguration &chargingOptimizationConfiguration);
    void chargingOptimizationConfigurationRemoved(const ThingId &evChargerThingId);

    void batteryConfigurationAdded(const BatteryConfiguration &batteryConfiguration);
    void batteryConfigurationChanged(const BatteryConfiguration &batteryConfiguration);
    void batteryConfigurationRemoved(const ThingId &batteryThingId);

    void pvConfigurationAdded(const PvConfiguration &pvConfiguration);
    void pvConfigurationChanged(const PvConfiguration &pvConfiguration);
    void pvConfigurationRemoved(const ThingId &pvThingId);

    void chargingSessionConfigurationAdded(const ChargingSessionConfiguration &chargingSessionConfiguration);
    void chargingSessionConfigurationChanged(const ChargingSessionConfiguration &chargingSessionConfiguration);
    void chargingSessionConfigurationRemoved(const ThingId &evChargerThingId);

    void conEMSStateAdded(const ConEMSState &conEMSState);
    void conEMSStateChanged(const ConEMSState &conEMSState);
    void conEMSStateRemoved(const QUuid &conEMSStateID);

private:
    ThingManager *m_thingManager = nullptr;
    EnergyManager *m_energyManager = nullptr;

    // System information
    HemsUseCases m_availableUseCases;
    uint m_housholdPhaseLimit = 25;
    uint m_housholdPhaseCount = 3;
    double m_housholdPowerLimit = m_housholdPhaseCount * m_housholdPhaseLimit;

    QHash<ThingId, HeatingConfiguration> m_heatingConfigurations;
    QHash<ThingId, HeatingRodConfiguration> m_heatingRodConfigurations;
    QHash<ThingId, WashingMachineConfiguration> m_washingMachineConfigurations;
    QHash<ThingId, ChargingOptimizationConfiguration> m_chargingOptimizationConfigurations;
    QHash<ThingId, ChargingConfiguration> m_chargingConfigurations;
    QHash<ThingId, BatteryConfiguration> m_batteryConfigurations;
    QHash<ThingId, PvConfiguration> m_pvConfigurations;
    QHash<ThingId, ChargingSessionConfiguration> m_chargingSessionConfigurations;
    QHash<QUuid, UserConfiguration> m_userConfigurations;
    ConEMSState m_conEMSState;

    QHash<ThingId, Thing *> m_inverters;
    QHash<ThingId, Thing *> m_heatPumps;
    QHash<ThingId, Thing *> m_heatingRods;
    QHash<ThingId, Thing *> m_washingMachines;
    QHash<ThingId, Thing *> m_evChargers;
    QHash<ThingId, Thing *> m_batteries;

    bool m_hybridSimulationEnabled = false;
    QMap<QString, QVariant> m_hybridSimulationMap;
    bool m_hybridSimIgnoreSimulated = true;


    void monitorHeatPump(Thing *thing);
    void monitorHeatingRod(Thing *thing);    
    void monitorWashingMachine(Thing *thing);
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
    void updateHybridSimulation(Thing *thing);

    void evaluateAvailableUseCases();

    void loadUserConfiguration();
    void saveUserConfigurationToSettings(const UserConfiguration &userConfiguration);
    void removeUserConfigurationFromSettings();

    void loadHeatingConfiguration(const ThingId &heatPumpThingId);
    void saveHeatingConfigurationToSettings(const HeatingConfiguration &heatingConfiguration);
    void removeHeatingConfigurationFromSettings(const ThingId &heatPumpThingId);

    void loadHeatingRodConfiguration(const ThingId &heatingRodThingId);
    void saveHeatingRodConfigurationToSettings(const HeatingRodConfiguration &heatingRodConfiguration);
    void removeHeatingRodConfigurationFromSettings(const ThingId &heatingRodThingId);

    void loadWashingMachineConfiguration(const ThingId &washingMachineThingId);
    void saveWashingMachineConfigurationToSettings(const WashingMachineConfiguration &washingMachineConfiguration);
    void removeWashingMachineConfigurationFromSettings(const ThingId &washingMachineThingId);

    void loadChargingConfiguration(const ThingId &evChargerThingId);
    void saveChargingConfigurationToSettings(const ChargingConfiguration &chargingConfiguration);
    void removeChargingConfigurationFromSettings(const ThingId &evChargerThingId);

    void loadChargingOptimizationConfiguration(const ThingId &evChargerThingId);
    void saveChargingOptimizationConfigurationToSettings(const ChargingOptimizationConfiguration &chargingOptimizationConfiguration);
    void removeChargingOptimizationConfigurationFromSettings(const ThingId &evChargerThingId);

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
