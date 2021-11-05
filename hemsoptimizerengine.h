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

#ifndef HEMSOPTIMIZERENGINE_H
#define HEMSOPTIMIZERENGINE_H

#include <QUrl>
#include <QObject>
#include <QNetworkRequest>
#include <QNetworkAccessManager>

#include "integrations/thing.h"

class HemsOptimizerEngine : public QObject
{
    Q_OBJECT
public:
    explicit HemsOptimizerEngine(QObject *parent = nullptr);

    // Build individual information blocks
    QVariantMap buildRootMeterInformation(const QList<QDateTime> &timestamps, Thing *rootMeter, double price);
    QVariantMap buildPhotovoltaicInformation(const QList<QDateTime> &timestamps, Thing *inverter, const QVariantList &forecast);
    QVariantMap buildElectricDemandInformation(const QList<QDateTime> &timestamps, const QUuid &householdUuid, const QVariantList &forecast);
    QVariantMap buildHeatpumpInformation(const QList<QDateTime> &timestamps, Thing *heatpump, double maxThermalEnergy, double soc, double electricPowerMax, const QVariantList &thermalDemandForecast, const QVariantList &copForecast, double rho = 0.1);
    QVariantMap buildEvChargerInformation(const QList<QDateTime> &timestamps, Thing *evCharger, double maxPower, double minPower, double energyNeeded, const QDateTime &endTime);

    // Request optimization schedules
    QNetworkReply *pvOptimization(const QVariantMap &ntpInfos, const QVariantMap &photovoltaicInfos, const QVariantMap &electricDemandInfo, const QVariantMap &heatpumpInfo, const QVariantMap &evChargerInfo = QVariantMap());

signals:

private:
    QNetworkAccessManager *m_networkManager = nullptr;
    QUrl m_apiBaseUrl;
    QString m_username;
    QString m_password;

    QNetworkRequest buildRequest(const QString &path);

    QStringList convertTimestampsToStringList(const QList<QDateTime> &timestamps);
};

#endif // HEMSOPTIMIZERENGINE_H
