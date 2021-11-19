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

#ifndef HEMSOPTIMIZERINTERFACE_H
#define HEMSOPTIMIZERINTERFACE_H

#include <QUrl>
#include <QDebug>
#include <QObject>
#include <QDateTime>
#include <QNetworkRequest>
#include <QNetworkAccessManager>

#include "integrations/thing.h"


class HemsOptimizerSchedule
{
public:
    HemsOptimizerSchedule(const QDateTime &timestamp, double value) : m_timestamp(timestamp), m_value(value) { }
    QDateTime timestamp() const { return m_timestamp; }
    double value() const  { return m_value; }

private:
    QDateTime m_timestamp;
    double m_value;
};

inline QDebug operator<<(QDebug debug, const HemsOptimizerSchedule &schedule) {
    debug.nospace().noquote() << "HemsSchedule(" << schedule.timestamp().toString("dd.MM.yyyy hh:mm:ss") << ", " << schedule.value() << ")";
    return debug.maybeSpace().maybeQuote();
}


class HemsOptimizerSchedules: public QList<HemsOptimizerSchedule>
{
public:
    HemsOptimizerSchedules() = default;

    inline void sort() {
        std::sort(begin(), end(), [](const HemsOptimizerSchedule &a, const HemsOptimizerSchedule &b) {
            return a.timestamp() < b.timestamp();
        });
    }
};


class HemsOptimizerInterface : public QObject
{
    Q_OBJECT
public:
    enum HouseType {
        HouseTypePassive,
        HouseTypeLowEnergy,
        HouseTypeEnEV2016,
        HouseTypeBefore1949,
        HouseTypeSince1949,
        HouseTypeSince1969,
        HouseTypeSince1979,
        HouseTypeSince1984
    };
    Q_ENUM(HouseType)

    explicit HemsOptimizerInterface(QNetworkAccessManager *networkManager, QObject *parent = nullptr);

    // Request optimization schedules
    QNetworkReply *pvOptimization(const QVariantMap &ntpInfos, const QVariantMap &photovoltaicInfos, const QVariantMap &electricDemandInfo, const QVariantMap &heatpumpInfo, const QVariantMap &evChargerInfo = QVariantMap());

    // Get the electic demand based on the annual demand (kWh) within the given timestamps
    QNetworkReply *electricPowerDemandStandardProfile(const QVector<QDateTime> &timestamps, double annualDemand = 4000);

    // Get the floor heaing power demand based on the house type and living area in m^2 and the hourly 24h history temperature and 24 h forcast temperature
    QNetworkReply *florHeatingPowerDemandStandardProfile(HouseType houseType, double livingArea, const QVariantMap &temperatureHistory, const QVariantMap &temperatureForecast);

    // Build individual information blocks
    QVariantMap buildRootMeterInformation(const QVector<QDateTime> &timestamps, Thing *rootMeter, double housholdPowerLimit, double price);
    QVariantMap buildPhotovoltaicInformation(const QVector<QDateTime> &timestamps, const QVariantList &forecast);
    QVariantMap buildElectricDemandInformation(const QVector<QDateTime> &timestamps, const QUuid &householdUuid, const QVariantList &forecast);
    QVariantMap buildHeatpumpInformation(const QVector<QDateTime> &timestamps, const QUuid &heatpumpUuid, double maxThermalEnergy, double soc, double electricPowerMax, const QVariantList &thermalDemandForecast, const QVariantList &copForecast, double rho = 0.1);
    QVariantMap buildEvChargerInformation(const QVector<QDateTime> &timestamps, Thing *evCharger, double maxPower, double minPower, double energyNeeded, const QDateTime &endTime);

    HemsOptimizerSchedules parseSchedules(const QVariantMap &schedulesMap);

signals:

private:
    QNetworkAccessManager *m_networkManager = nullptr;
    QUrl m_apiBaseUrl;
    QString m_username;
    QString m_password;

    QNetworkRequest buildRequest(const QString &path);

    QStringList convertTimestampsToStringList(const QVector<QDateTime> &timestamps);
};

#endif // HEMSOPTIMIZERINTERFACE_H
