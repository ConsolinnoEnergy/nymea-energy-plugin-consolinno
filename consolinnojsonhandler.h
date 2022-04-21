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

#ifndef CONSOLINNOJSONHANDLER_H
#define CONSOLINNOJSONHANDLER_H

#include <QObject>
#include "jsonrpc/jsonhandler.h"

class EnergyEngine;

class ConsolinnoJsonHandler : public JsonHandler
{
    Q_OBJECT
public:
    explicit ConsolinnoJsonHandler(EnergyEngine *energyEngine, QObject *parent = nullptr);

    QString name() const override;

    Q_INVOKABLE JsonReply* GetAvailableUseCases(const QVariantMap &params);

    Q_INVOKABLE JsonReply* GetHousholdPhaseLimit(const QVariantMap &params);
    Q_INVOKABLE JsonReply* SetHousholdPhaseLimit(const QVariantMap &params);

    Q_INVOKABLE JsonReply* GetHeatingConfigurations(const QVariantMap &params);
    Q_INVOKABLE JsonReply* SetHeatingConfiguration(const QVariantMap &params);

    Q_INVOKABLE JsonReply* GetPvConfigurations(const QVariantMap &params);
    Q_INVOKABLE JsonReply* SetPvConfiguration(const QVariantMap &params);

    Q_INVOKABLE JsonReply* GetChargingSessionConfigurations(const QVariantMap &params);
    Q_INVOKABLE JsonReply* SetChargingSessionConfiguration(const QVariantMap &params);

    Q_INVOKABLE JsonReply* GetChargingConfigurations(const QVariantMap &params);
    Q_INVOKABLE JsonReply* SetChargingConfiguration(const QVariantMap &params);

signals:
    void AvailableUseCasesChanged(const QVariantMap &params);

    void HousholdPhaseLimitChanged(const QVariantMap &params);

    void HeatingConfigurationAdded(const QVariantMap &params);
    void HeatingConfigurationRemoved(const QVariantMap &params);
    void HeatingConfigurationChanged(const QVariantMap &params);

    void ChargingConfigurationAdded(const QVariantMap &params);
    void ChargingConfigurationRemoved(const QVariantMap &params);
    void ChargingConfigurationChanged(const QVariantMap &params);

    void PvConfigurationAdded(const QVariantMap &params);
    void PvConfigurationRemoved(const QVariantMap &params);
    void PvConfigurationChanged(const QVariantMap &params);

    void ChargingSessionConfigurationAdded (const QVariantMap &params);
    void ChargingSessionConfigurationRemoved(const QVariantMap &params);
    void ChargingSessionConfigurationChanged(const QVariantMap &params);


private:
    EnergyEngine *m_energyEngine = nullptr;

};

#endif // CONSOLINNOJSONHANDLER_H
