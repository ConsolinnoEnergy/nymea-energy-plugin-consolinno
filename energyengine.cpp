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

Q_DECLARE_LOGGING_CATEGORY(dcConsolinnoExperience)

EnergyEngine::EnergyEngine(ThingManager *thingManager, QObject *parent):
    QObject(parent),
    m_thingManager(thingManager)
{
    qCDebug(dcConsolinnoExperience()) << "Start initializing consolinno energy engine.";
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

    m_optimizer = new HemsOptimizerEngine(this);

    QStringList timestamps;
    QDateTime currentDateTime = QDateTime::currentDateTime();
    for (int i = 0; i < 5; i++) {
        timestamps << currentDateTime.addSecs(i * 60 * 15).toString(Qt::ISODate);
    }

    QVariantMap ntpMap = m_optimizer->buildRootMeterInformation(timestamps, m_rootMeter, 0.3);
    qCDebug(dcConsolinnoExperience()) << qUtf8Printable(QJsonDocument::fromVariant(ntpMap).toJson(QJsonDocument::Indented));
}

void EnergyEngine::monitorHeatPump(Thing *thing)
{
    qCDebug(dcConsolinnoExperience()) << "Start monitoring heatpump" << thing;
    m_heatPumps.insert(thing->id(), thing);
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

    if (thing->thingClass().interfaces().contains("heatpump")) {
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
}

void EnergyEngine::evaluate()
{
    // We need a root meter, otherwise no optimization can be done.
    if (!m_rootMeter)
        return;

    evaluateHeatPumps();

}

void EnergyEngine::evaluateHeatPumps()
{

}
