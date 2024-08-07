/* Copyright (C) Consolinno Energy GmbH - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef CONSOLINNOJSONHANDLER_H
#define CONSOLINNOJSONHANDLER_H

#include "energypluginconsolinno.h"
#include "jsonrpc/jsonhandler.h"
#include <QObject>

class EnergyEngine;

class ConsolinnoJsonHandler : public JsonHandler {
    Q_OBJECT
public:
    explicit ConsolinnoJsonHandler(
        EnergyEngine* energyEngine, HEMSVersionInfo versionInfo, QObject* parent = nullptr);

    QString name() const override;

    Q_INVOKABLE JsonReply* GetHEMSVersion(const QVariantMap& params);
    Q_INVOKABLE JsonReply* GetAvailableUseCases(const QVariantMap& params);

    Q_INVOKABLE JsonReply* GetHousholdPhaseLimit(const QVariantMap& params);
    Q_INVOKABLE JsonReply* SetHousholdPhaseLimit(const QVariantMap& params);

    Q_INVOKABLE JsonReply* GetHeatingConfigurations(const QVariantMap& params);
    Q_INVOKABLE JsonReply* SetHeatingConfiguration(const QVariantMap& params);

    Q_INVOKABLE JsonReply* GetHeatingRodConfigurations(const QVariantMap& params);
    Q_INVOKABLE JsonReply* SetHeatingRodConfiguration(const QVariantMap& params);

    Q_INVOKABLE JsonReply* GetDynamicElectricPricingConfigurations(const QVariantMap& params);
    Q_INVOKABLE JsonReply* SetDynamicElectricPricingConfiguration(const QVariantMap& params);

    Q_INVOKABLE JsonReply* GetWashingMachineConfigurations(const QVariantMap& params);
    Q_INVOKABLE JsonReply* SetWashingMachineConfiguration(const QVariantMap& params);

    Q_INVOKABLE JsonReply* GetUserConfigurations(const QVariantMap& params);
    Q_INVOKABLE JsonReply* SetUserConfiguration(const QVariantMap& params);

    Q_INVOKABLE JsonReply* GetPvConfigurations(const QVariantMap& params);
    Q_INVOKABLE JsonReply* SetPvConfiguration(const QVariantMap& params);

    Q_INVOKABLE JsonReply* GetChargingSessionConfigurations(const QVariantMap& params);
    Q_INVOKABLE JsonReply* SetChargingSessionConfiguration(const QVariantMap& params);

    Q_INVOKABLE JsonReply* GetChargingConfigurations(const QVariantMap& params);
    Q_INVOKABLE JsonReply* SetChargingConfiguration(const QVariantMap& params);

    Q_INVOKABLE JsonReply* GetChargingOptimizationConfigurations(const QVariantMap& params);
    Q_INVOKABLE JsonReply* SetChargingOptimizationConfiguration(const QVariantMap& params);

    Q_INVOKABLE JsonReply* GetBatteryConfigurations(const QVariantMap& params);
    Q_INVOKABLE JsonReply* SetBatteryConfiguration(const QVariantMap& params);

    Q_INVOKABLE JsonReply* GetConEMSState(const QVariantMap& params);
    Q_INVOKABLE JsonReply* SetConEMSState(const QVariantMap& params);

    Q_INVOKABLE JsonReply* GetGridSupportThing(const QVariantMap& params);

signals:
    void PluggedInChanged(const QVariantMap& params);

    void AvailableUseCasesChanged(const QVariantMap& params);

    void HousholdPhaseLimitChanged(const QVariantMap& params);

    void UserConfigurationAdded(const QVariantMap& params);
    void UserConfigurationRemoved(const QVariantMap& params);
    void UserConfigurationChanged(const QVariantMap& params);

    void HeatingConfigurationAdded(const QVariantMap& params);
    void HeatingConfigurationRemoved(const QVariantMap& params);
    void HeatingConfigurationChanged(const QVariantMap& params);

    void HeatingRodConfigurationAdded(const QVariantMap& params);
    void HeatingRodConfigurationRemoved(const QVariantMap& params);
    void HeatingRodConfigurationChanged(const QVariantMap& params);

    void DynamicElectricPricingConfigurationAdded(const QVariantMap& params);
    void DynamicElectricPricingConfigurationRemoved(const QVariantMap& params);
    void DynamicElectricPricingConfigurationChanged(const QVariantMap& params);

    void WashingMachineConfigurationAdded(const QVariantMap& params);
    void WashingMachineConfigurationRemoved(const QVariantMap& params);
    void WashingMachineConfigurationChanged(const QVariantMap& params);

    void ChargingConfigurationAdded(const QVariantMap& params);
    void ChargingConfigurationRemoved(const QVariantMap& params);
    void ChargingConfigurationChanged(const QVariantMap& params);

    void ChargingOptimizationConfigurationAdded(const QVariantMap& params);
    void ChargingOptimizationConfigurationRemoved(const QVariantMap& params);
    void ChargingOptimizationConfigurationChanged(const QVariantMap& params);

    void BatteryConfigurationAdded(const QVariantMap& params);
    void BatteryConfigurationRemoved(const QVariantMap& params);
    void BatteryConfigurationChanged(const QVariantMap& params);

    void PvConfigurationAdded(const QVariantMap& params);
    void PvConfigurationRemoved(const QVariantMap& params);
    void PvConfigurationChanged(const QVariantMap& params);

    void ChargingSessionConfigurationAdded(const QVariantMap& params);
    void ChargingSessionConfigurationRemoved(const QVariantMap& params);
    void ChargingSessionConfigurationChanged(const QVariantMap& params);

    void ConEMSStateAdded(const QVariantMap& params);
    void ConEMSStateRemoved(const QVariantMap& params);
    void ConEMSStateChanged(const QVariantMap& params);

private:
    EnergyEngine* m_energyEngine = nullptr;
    HEMSVersionInfo m_versionInfo;
};

#endif // CONSOLINNOJSONHANDLER_H
