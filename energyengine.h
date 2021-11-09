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

#include <integrations/thingmanager.h>

#include "configurations/chargingconfiguration.h"
#include "configurations/heatingconfiguration.h"

class HemsOptimizerEngine;

class EnergyEngine : public QObject
{
    Q_OBJECT
public:
    enum HemsError {
        HemsErrorNoError,
        HemsErrorInvalidParameter,
        HemsErrorInvalidThing,
        HemsErrorThingNotFound
    };
    Q_ENUM(HemsError)

    enum HemsUseCase {
        HemsUseCaseBlackoutProtection = 0x01,
        HemsUseCaseHeating = 0x02,
        HemsUseCaseCharging = 0x04
    };
    Q_ENUM(HemsUseCase)
    Q_DECLARE_FLAGS(HemsUseCases, HemsUseCase)
    Q_FLAG(HemsUseCases)

    explicit EnergyEngine(ThingManager *thingManager, QObject *parent = nullptr);

    EnergyEngine::HemsUseCases availableUseCases() const;

    // Heating configurations
    QList<HeatingConfiguration> heatingConfigurations() const;
    EnergyEngine::HemsError setHeatingConfiguration(const HeatingConfiguration &heatingConfiguration);

    // Charging configurations
    QList<ChargingConfiguration> chargingConfigurations() const;
    EnergyEngine::HemsError setChargingConfiguration(const ChargingConfiguration &chargingConfiguration);

signals:
    void availableUseCasesChanged(EnergyEngine::HemsUseCases availableUseCases);
    void heatingConfigurationAdded(const HeatingConfiguration &heatingConfiguration);
    void heatingConfigurationChanged(const HeatingConfiguration &heatingConfiguration);
    void heatingConfigurationRemoved(const ThingId &heatPumpThingId);
    void chargingConfigurationAdded(const ChargingConfiguration &chargingConfiguration);
    void chargingConfigurationChanged(const ChargingConfiguration &chargingConfiguration);
    void chargingConfigurationRemoved(const ThingId &evChargerThingId);

private:
    ThingManager *m_thingManager = nullptr;
    Thing *m_rootMeter = nullptr;
    HemsOptimizerEngine *m_optimizer = nullptr;

    HemsUseCases m_availableUseCases;

    QHash<ThingId, HeatingConfiguration> m_heatingConfigurations;
    QHash<ThingId, ChargingConfiguration> m_chargingConfigurations;

    QHash<ThingId, Thing *> m_inverters;
    QHash<ThingId, Thing *> m_heatPumps;
    QHash<ThingId, Thing *> m_evChargers;

    void monitorHeatPump(Thing *thing);
    void monitorInverter(Thing *thing);
    void monitorEvCharger(Thing *thing);

private slots:
    void onThingAdded(Thing *thing);
    void onThingRemoved(const ThingId &thingId);

    void evaluate();

    void evaluateHeatPumps();
    void evaluateInverters();
    void evaluateEvChargers();

    void updateSchedules();

    void evaluateAvailableUseCases();

    QList<QDateTime> generateTimeStamps(uint resolutionMinutes, uint durationHours);
    QVariantList getPvForecast(const QList<QDateTime> &timestamps, Thing *inverter);
    QVariantList getConsumptionForecast(const QList<QDateTime> &timestamps);
    QVariantList getThermalDemandForecast(const QList<QDateTime> &timestamps, Thing *heatPump);

    void saveHeatingConfigurationToSettings(const HeatingConfiguration &heatingConfiguration);
    void removeHeatingConfigurationFromSettings(const ThingId &heatPumpThingId);

    void saveChargingConfigurationToSettings(const ChargingConfiguration &chargingConfiguration);
    void removeChargingConfigurationFromSettings(const ThingId &evChargerThingId);

};

Q_DECLARE_OPERATORS_FOR_FLAGS(EnergyEngine::HemsUseCases)

#endif // ENERGYENGINE_H
