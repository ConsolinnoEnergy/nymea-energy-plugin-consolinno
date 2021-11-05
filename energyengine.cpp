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
#include "hemsoptimizerengine.h"

#include <QJsonDocument>
#include <QNetworkReply>

Q_DECLARE_LOGGING_CATEGORY(dcConsolinnoExperience)

EnergyEngine::EnergyEngine(ThingManager *thingManager, QObject *parent):
    QObject(parent),
    m_thingManager(thingManager)
{
    qCDebug(dcConsolinnoExperience()) << "Initializing consolinno energy engine...";
    foreach (Thing *thing, m_thingManager->configuredThings()) {
        onThingAdded(thing);
    }

    connect(thingManager, &ThingManager::thingAdded, this, &EnergyEngine::onThingAdded);
    connect(thingManager, &ThingManager::thingRemoved, this, &EnergyEngine::onThingRemoved);

    QSettings settings(NymeaSettings::settingsPath() + "/consolinno.conf", QSettings::IniFormat);
    // FIXME: get the root meter from the overall energy experience, not just pick the first one
    if (m_rootMeter) {
        connect(m_rootMeter, &Thing::stateValueChanged, this, [=](const StateTypeId &stateTypeId, const QVariant &/*value*/){
            StateType stateType = m_rootMeter->thingClass().getStateType(stateTypeId);
            if (stateType.name() == "currentPower") {
                evaluate();
            }
        });
    } else {
        qCWarning(dcConsolinnoExperience()) << "No root meter specified yet.";
    }

    // Engine for interacting with the online Hems optimizer
    m_optimizer = new HemsOptimizerEngine(this);

    qCDebug(dcConsolinnoExperience()) << "Consolinno energy engine initialized";

    updateSchedules();
}

EnergyEngine::HemsUseCases EnergyEngine::availableUseCases() const
{
    return m_availableUseCases;
}

QList<HeatingConfiguration> EnergyEngine::heatingConfigurations() const
{
    return m_heatingConfigurations.values();
}

EnergyEngine::HemsError EnergyEngine::setHeatingConfiguration(const HeatingConfiguration &heatingConfiguration)
{
    // TODO
    Q_UNUSED(heatingConfiguration)
    return HemsErrorNoError;
}

QList<ChargingConfiguration> EnergyEngine::chargingConfigurations() const
{
    return m_chargingConfigurations.values();
}

EnergyEngine::HemsError EnergyEngine::setChargingConfiguration(const ChargingConfiguration &chargingConfiguration)
{
    // TODO
    Q_UNUSED(chargingConfiguration)
    return HemsErrorNoError;
}

void EnergyEngine::monitorHeatPump(Thing *thing)
{
    qCDebug(dcConsolinnoExperience()) << "Start monitoring heatpump" << thing;
    m_heatPumps.insert(thing->id(), thing);
}

void EnergyEngine::monitorInverter(Thing *thing)
{
    qCDebug(dcConsolinnoExperience()) << "Start monitoring inverter" << thing;
    m_inverters.insert(thing->id(), thing);
}

void EnergyEngine::onThingAdded(Thing *thing)
{
    if (thing->thingClass().interfaces().contains("smartmeter")) {
        // FIXME: get the root meter from the overall energy experience
        if (!m_rootMeter) {
            m_rootMeter = thing;
            qCDebug(dcConsolinnoExperience()) << "Using root meter" << m_rootMeter;
            return;
        }
    }

    if (thing->thingClass().interfaces().contains("solarinverter")) {
        // FIXME: pic the first for oiptimizer for now
        if (!m_inverter) {
            m_inverter = thing;
            qCDebug(dcConsolinnoExperience()) << "Using inverter" << m_inverter;
            return;
        }

        monitorInverter(thing);
    }

    if (thing->thingClass().interfaces().contains("heatpump")) {
        // FIXME: pic the first for oiptimizer for now
        if (!m_heatPump) {
            m_heatPump = thing;
            qCDebug(dcConsolinnoExperience()) << "Using heat pump" << m_heatPump;
            return;
        }

        monitorHeatPump(thing);
    }
}

void EnergyEngine::onThingRemoved(const ThingId &thingId)
{
    if (m_rootMeter->id() == thingId) {
        m_rootMeter = nullptr;
        qCWarning(dcConsolinnoExperience()) << "The root meter has been removed. The energy manager will not work any more.";
    }

    if (m_heatPumps.contains(thingId)) {
        m_heatPumps.remove(thingId);
        qCDebug(dcConsolinnoExperience()) << "Removed heat pump from energy manager.";
    }

    if (m_inverters.contains(thingId)) {
        m_inverters.remove(thingId);
        qCDebug(dcConsolinnoExperience()) << "Removed inverter from energy manager.";
    }
}

void EnergyEngine::evaluate()
{
    // We need a root meter, otherwise no optimization can be done.
    if (!m_rootMeter)
        return;

    evaluateHeatPumps();
    evaluateInverters();
}

void EnergyEngine::evaluateHeatPumps()
{

}

void EnergyEngine::evaluateInverters()
{

}

void EnergyEngine::updateSchedules()
{
    // Make sure we have a setup for this testcase
    if (!m_rootMeter || !m_inverter || !m_heatPump) {
        qCWarning(dcConsolinnoExperience()) << "Cannot update schedule because we don't have the required things.";
        return;
    }

    qCDebug(dcConsolinnoExperience()) << "Update schedules for the next 12 hours on 10 minutes resolution...";
    // Create timestamps for the next 12 hours in 10 min slots
    QList<QDateTime> timestamps = generateTimeStamps(15, 12);

    // Root meter
    QVariantMap ntpMap = m_optimizer->buildRootMeterInformation(timestamps, m_rootMeter, 0.3);

    // Inverter
    QVariantList pvForecast = getPvForecast(timestamps, m_inverter);
    QVariantMap pvMap = m_optimizer->buildPhotovoltaicInformation(timestamps, m_inverter, pvForecast);

    // Electrical demand
    QVariantMap electricalDemandMap = m_optimizer->buildElectricDemandInformation(timestamps, QUuid::createUuid(), getConsumptionForecast(timestamps));

    // Heat pump
    QVariantList thermalDemandForcast = getThermalDemandForecast(timestamps, m_heatPump);
    QVariantList copForecast;
    for (int i = 0; i < timestamps.count(); i++) {
        // Default to 3 for now
        copForecast << 3;
    }
    // Heatpump has a maximum of 9 kW
    double maxElectricalPower = 9;
    // We assume a maximal themral energy of 4kWh
    double maxThermalEnergy = 4;

    QVariantMap heatPumpMap = m_optimizer->buildHeatpumpInformation(timestamps, m_heatPump, maxThermalEnergy, -1, maxElectricalPower, thermalDemandForcast, copForecast, 0.1);

    QNetworkReply *reply = m_optimizer->pvOptimization(ntpMap, pvMap, electricalDemandMap, heatPumpMap);
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
    connect(reply, &QNetworkReply::finished, reply, [=](){
        if (reply->error() != QNetworkReply::NoError) {
            qCWarning(dcConsolinnoExperience()) << "HemsOptimizer: Failed to get pv optimization. The reply returned with error" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) << reply->errorString();
            QByteArray responsedata = reply->readAll();
            qCWarning(dcConsolinnoExperience()) << qUtf8Printable(responsedata);
            return;
        }

        QByteArray data = reply->readAll();
        QJsonParseError error;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
        if (error.error != QJsonParseError::NoError) {
            qCWarning(dcConsolinnoExperience()) << "HemsOptimizer: Failed to parse pv optimization data" << data << ":" << error.errorString();
            return;
        }

        qCDebug(dcConsolinnoExperience()) << "HemsOptimizer: Request pv optimization finished successfully";
        qCDebug(dcConsolinnoExperience()) << "<--" << qUtf8Printable(jsonDoc.toJson(QJsonDocument::Indented));
    });

}

QList<QDateTime> EnergyEngine::generateTimeStamps(uint resolutionMinutes, uint durationHours)
{
    QList<QDateTime> timestamps;
    // Example: resolution of 10 minutes for the next 12 hours = 72 timestamps
    uint timestampsCount = (durationHours * 60) / resolutionMinutes;
    QDateTime currentDateTime = QDateTime::currentDateTime();
    //qCDebug(dcConsolinnoExperience()) << currentDateTime.toString(Qt::ISODate)
    for (uint i = 0; i < timestampsCount; i++) {
        timestamps << currentDateTime.addSecs(i * 60 * resolutionMinutes);
    }
    return timestamps;
}

QVariantList EnergyEngine::getPvForecast(const QList<QDateTime> &timestamps, Thing *inverter)
{
    Q_UNUSED(inverter)
    // FIXME: get actual inverter data

    QVariantList forecast;
    double currentValue = 0;
    for (int i = 0; i < timestamps.count(); i++) {
        if (i < timestamps.count() / 2) {
            currentValue += (qrand() % 2) / 10.0;
        } else {
            currentValue -= (qrand() % 2) / 10.0;
            if (currentValue < 0)
                currentValue = 0;
        }

        forecast << currentValue;
    }

    return forecast;
}

QVariantList EnergyEngine::getConsumptionForecast(const QList<QDateTime> &timestamps)
{
    // FIXME: get household consumption
    QVariantList forecast;
    for (int i = 0; i < timestamps.count(); i++) {
        forecast << (200 + (qrand() % 50)) / 1000.0; // kW
    }
    return forecast;
}

QVariantList EnergyEngine::getThermalDemandForecast(const QList<QDateTime> &timestamps, Thing *heatPump)
{
    Q_UNUSED(heatPump)

    // FIXME: get actual thermal demand depending on outdoor temperature
    QVariantList forecast;
    for (int i = 0; i < timestamps.count(); i++) {
        forecast << (2500 + (qrand() % 500)) / 1000.0; // kWh
    }
    return forecast;
}

