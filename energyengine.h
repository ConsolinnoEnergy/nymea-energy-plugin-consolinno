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
#include "configurations/heatingconfiguration.h"
#include "configurations/pvconfiguration.h"

class HemsOptimizerEngine;
class WeatherDataProvider;

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
        HemsUseCasePv = 0x08
    };
    Q_ENUM(HemsUseCase)
    Q_DECLARE_FLAGS(HemsUseCases, HemsUseCase)
    Q_FLAG(HemsUseCases)

    explicit EnergyEngine(ThingManager *thingManager, EnergyManager *energyManager, QObject *parent = nullptr);

    EnergyEngine::HemsUseCases availableUseCases() const;

    uint housholdPhaseLimit() const;
    EnergyEngine::HemsError setHousholdPhaseLimit(uint housholdPhaseLimit);

    // Heating configurations
    QList<HeatingConfiguration> heatingConfigurations() const;
    EnergyEngine::HemsError setHeatingConfiguration(const HeatingConfiguration &heatingConfiguration);

    // Charging configurations
    QList<ChargingConfiguration> chargingConfigurations() const;
    EnergyEngine::HemsError setChargingConfiguration(const ChargingConfiguration &chargingConfiguration);

    // Pv configurations
    QList<PvConfiguration> pvConfigurations() const;
    EnergyEngine::HemsError setPvConfiguration(const PvConfiguration &pvConfiguration);


signals:
    void availableUseCasesChanged(EnergyEngine::HemsUseCases availableUseCases);
    void housholdPhaseLimitChanged(uint housholdPhaseLimit);

    void heatingConfigurationAdded(const HeatingConfiguration &heatingConfiguration);
    void heatingConfigurationChanged(const HeatingConfiguration &heatingConfiguration);
    void heatingConfigurationRemoved(const ThingId &heatPumpThingId);

    void chargingConfigurationAdded(const ChargingConfiguration &chargingConfiguration);
    void chargingConfigurationChanged(const ChargingConfiguration &chargingConfiguration);
    void chargingConfigurationRemoved(const ThingId &evChargerThingId);

    void pvConfigurationAdded(const PvConfiguration &pvConfiguration);
    void pvConfigurationChanged(const PvConfiguration &pvConfiguration);
    void pvConfigurationRemoved(const ThingId &pvThingId);

private:
    ThingManager *m_thingManager = nullptr;
    EnergyManager *m_energyManager = nullptr;
    HemsOptimizerEngine *m_optimizerEngine = nullptr;
    QNetworkAccessManager *m_networkManager = nullptr;
    WeatherDataProvider *m_weatherDataProvider = nullptr;

    // System information
    HemsUseCases m_availableUseCases;
    uint m_housholdPhaseLimit = 25;
    uint m_housholdPhaseCount = 3;
    double m_housholdPowerLimit = m_housholdPhaseCount * m_housholdPhaseLimit;

    QHash<ThingId, HeatingConfiguration> m_heatingConfigurations;
    QHash<ThingId, ChargingConfiguration> m_chargingConfigurations;
    QHash<ThingId, PvConfiguration> m_pvConfigurations;

    QHash<ThingId, Thing *> m_inverters;
    QHash<ThingId, Thing *> m_heatPumps;
    QHash<ThingId, Thing *> m_evChargers;

    Thing *m_weatherThing = nullptr;

    void monitorHeatPump(Thing *thing);
    void monitorInverter(Thing *thing);
    void monitorEvCharger(Thing *thing);

private slots:
    void onThingAdded(Thing *thing);
    void onThingRemoved(const ThingId &thingId);

    void onRootMeterChanged();

    void evaluate();

    void evaluateHeatPumps();
    void evaluateInverters();
    void evaluateEvChargers();

    void updateSchedules();

    void evaluateAvailableUseCases();

    void loadHeatingConfiguration(const ThingId &heatPumpThingId);
    void saveHeatingConfigurationToSettings(const HeatingConfiguration &heatingConfiguration);
    void removeHeatingConfigurationFromSettings(const ThingId &heatPumpThingId);

    void loadChargingConfiguration(const ThingId &evChargerThingId);
    void saveChargingConfigurationToSettings(const ChargingConfiguration &chargingConfiguration);
    void removeChargingConfigurationFromSettings(const ThingId &evChargerThingId);

    void loadPvConfiguration(const ThingId &pvThingId);
    void savePvConfigurationToSettings(const PvConfiguration &pvConfiguration);
    void removePvConfigurationFromSettings(const ThingId &pvThingId);

};

Q_DECLARE_OPERATORS_FOR_FLAGS(EnergyEngine::HemsUseCases)

#endif // ENERGYENGINE_H
