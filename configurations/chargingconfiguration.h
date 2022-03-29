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

#ifndef CHARGINGCONFIGURATION_H
#define CHARGINGCONFIGURATION_H

#include <QTime>
#include <QObject>
#include <QDebug>

#include <typeutils.h>

class ChargingConfiguration
{
    Q_GADGET
    Q_PROPERTY(QUuid evChargerThingId READ evChargerThingId WRITE setEvChargerThingId)
    Q_PROPERTY(bool optimizationEnabled READ optimizationEnabled WRITE setOptimizationEnabled USER true)
    Q_PROPERTY(QUuid carThingId READ carThingId WRITE setCarThingId USER true)
    Q_PROPERTY(QString endTime READ endTime WRITE setEndTime USER true)
    Q_PROPERTY(uint targetPercentage READ targetPercentage WRITE setTargetPercentage USER true)
    Q_PROPERTY(bool zeroReturnPolicyEnabled READ zeroReturnPolicyEnabled WRITE setZeroReturnPolicyEnabled USER true)

public:
    ChargingConfiguration();

    ThingId evChargerThingId() const;
    void setEvChargerThingId(const ThingId &evChargerThingId);

    bool optimizationEnabled() const;
    void setOptimizationEnabled(bool optimizationEnabled);

    ThingId carThingId() const;
    void setCarThingId(const ThingId &carThingId);

    QString endTime() const;
    void setEndTime(const QString &endTime);

    uint targetPercentage() const;
    void setTargetPercentage(uint targetPercentage);

    bool zeroReturnPolicyEnabled() const;
    void setZeroReturnPolicyEnabled(bool zeroReturnPolicyEnabled);

    bool isValid() const;

    bool operator==(const ChargingConfiguration &other) const;
    bool operator!=(const ChargingConfiguration &other) const;

private:
    ThingId m_evChargerThingId;
    bool m_optimizationEnabled = false;
    ThingId m_carThingId;
    QString m_endTime = "12:30:00";
    uint m_targetPercentage = 100;
    bool m_zeroReturnPolicyEnabled = false;

};

QDebug operator<<(QDebug debug, const ChargingConfiguration &chargingConfig);


#endif // CHARGINGCONFIGURATION_H
