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

#include "hemsoptimizerinterface.h"

#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QMetaEnum>

#include <loggingcategories.h>
Q_DECLARE_LOGGING_CATEGORY(dcHemsOptimizer)
Q_DECLARE_LOGGING_CATEGORY(dcHemsOptimizerTraffic)

HemsOptimizerInterface::HemsOptimizerInterface(QNetworkAccessManager *networkManager, QObject *parent) :
    QObject(parent),
    m_networkManager(networkManager)
{
    // Docs can be found here using the same username and password
    // https://lash-upstage.runner.consolinno.de/docs
    // All data values are kWh or kW, time always in UTC
    m_apiBaseUrl = QUrl("https://lash-upstage.services.consolinno.de/");
    m_username = "nymea";
    m_password = "3aB!NnUJe@Rez*%f3JY7";

    QNetworkReply *reply = m_networkManager->get(buildRequest("/healthz"));
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
    connect(reply, &QNetworkReply::finished, this, [this, reply](){
        if (reply->error() != QNetworkReply::NoError) {
            qCWarning(dcHemsOptimizer()) << "Failed to get heathz status. The reply returned with error" << reply->errorString();
            return;
        }

        QByteArray data = reply->readAll();
        QJsonParseError error;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
        if (error.error != QJsonParseError::NoError) {
            qCWarning(dcHemsOptimizer()) << "Failed to parse healthz status data" << data << ":" << error.errorString();
            return;
        }

        // qCDebug(dcHemsOptimizerTraffic()) << "<--" << qUtf8Printable(jsonDoc.toJson(QJsonDocument::Indented));
        QVariantMap dataMap = jsonDoc.toVariant().toMap();
        if (dataMap.contains("status") && dataMap.value("status").toString() == "healthz") {
            qCDebug(dcHemsOptimizer()) << "Get healthz status finished successfully";
            m_available = true;
            emit availableChanged(m_available);
        } else {
            qCWarning(dcHemsOptimizer()) << "Get healthz status finished with error" << qUtf8Printable(jsonDoc.toJson(QJsonDocument::Compact));
        }

        // TODO: use this as check if the optimizer is available

    });
}

bool HemsOptimizerInterface::available() const
{
    return m_available;
}

QVariantMap HemsOptimizerInterface::buildRootMeterInformation(const QVector<QDateTime> &timestamps, Thing *rootMeter, double housholdPowerLimit, double price)
{
    QVariantMap ntp;
    ntp.insert("uuid", rootMeter->id().toString().remove('{').remove('}'));
    ntp.insert("priority", 1);
    ntp.insert("timestamps", convertTimestampsToStringList(timestamps));

    // Fill known prices depending on the price model, assume nor 0,3 €
    QVariantList priceList;
    for (int i = 0; i < timestamps.count(); i++) {
        priceList.append(price);
    }
    ntp.insert("price", priceList);

    // Maximal power in (houshold phase limit * phases)
    double limit = housholdPowerLimit / 1000.0; // in kW
    QVariantList powerIn;
    for (int i = 0; i < timestamps.count(); i++) {
        powerIn.append(limit);
    }
    ntp.insert("electric_power_in", powerIn);

    // Set return to the maximal power in (houshold limit),
    // we must allow returning energy otherwise there might be no solution
    QVariantList powerOut;
    for (int i = 0; i < timestamps.count(); i++) {
        powerOut.append(limit);
    }
    ntp.insert("electric_power_out", powerOut);
    return ntp;
}

QVariantMap HemsOptimizerInterface::buildPhotovoltaicInformation(const QVector<QDateTime> &timestamps, const QVariantList &forecast)
{
    QVariantMap dataMap;
    dataMap.insert("uuid", QUuid::createUuid().toString().remove('{').remove('}'));
    dataMap.insert("priority", 1);
    dataMap.insert("timestamps", convertTimestampsToStringList(timestamps));
    dataMap.insert("forecast", forecast);
    return dataMap;
}

QVariantMap HemsOptimizerInterface::buildElectricDemandInformation(const QVector<QDateTime> &timestamps, const QUuid &householdUuid, const QVariantList &forecast)
{
    QVariantMap dataMap;
    dataMap.insert("uuid", householdUuid.toString().remove('{').remove('}'));
    dataMap.insert("priority", 1);
    dataMap.insert("timestamps", convertTimestampsToStringList(timestamps));
    dataMap.insert("forecast", forecast);
    return dataMap;
}

QVariantMap HemsOptimizerInterface::buildHeatpumpInformation(const QVector<QDateTime> &timestamps, const QUuid &heatpumpUuid, double maxThermalEnergy, double soc, double electricPowerMax, const QVariantList &thermalDemandForecast, const QVariantList &copForecast, double rho)
{
    QVariantMap dataMap;
    dataMap.insert("uuid", heatpumpUuid.toString().remove('{').remove('}'));
    dataMap.insert("priority", 1);
    dataMap.insert("timestamps", convertTimestampsToStringList(timestamps));
    dataMap.insert("is_on", true); // FIXME: find state
    dataMap.insert("thermal_energy_max", maxThermalEnergy); // get if from the datasheet, assum 9kWh for now
    dataMap.insert("electric_power_max", electricPowerMax);
    dataMap.insert("thermal_demand_forecast", thermalDemandForecast); // api endpoint

    // Soc of the buffe, default to 0.5 for now
    if (soc >= 0)
        dataMap.insert("soc", soc);

    // Default to 3 for now
    dataMap.insert("cop_forecast", copForecast); // Default to 3.5 for no

    dataMap.insert("rho", rho);
    return dataMap;
}

QVariantMap HemsOptimizerInterface::buildEvChargerInformation(const QVector<QDateTime> &timestamps, ThingId evChargerId, double maxPower, double minPower, double energyNeeded, const QDateTime &endTime)
{
    QVariantMap dataMap;
    dataMap.insert("uuid", evChargerId.toString().remove('{').remove('}'));
    dataMap.insert("priority", 1);
    dataMap.insert("timestamps", convertTimestampsToStringList(timestamps));
    dataMap.insert("electric_power_max", maxPower / 1000.0);
    dataMap.insert("electric_power_min", minPower / 1000.0);
    dataMap.insert("energy_needed", energyNeeded  / 1000.0);
    dataMap.insert("finished_until", endTime.toString(Qt::ISODate));
    return dataMap;
}

HemsOptimizerSchedules HemsOptimizerInterface::parseSchedules(const QVariantMap &schedulesMap)
{
    HemsOptimizerSchedules schedules;
    foreach (const QString &timestampString, schedulesMap.keys()) {
        schedules << HemsOptimizerSchedule(QDateTime::fromString(timestampString, Qt::ISODate), schedulesMap.value(timestampString).toDouble());
    }
    schedules.sort();
    return schedules;
}

QNetworkReply *HemsOptimizerInterface::pvOptimization(const QVariantMap &ntpInfos, const QVariantMap &photovoltaicInfos, const QVariantMap &electricDemandInfo, const QVariantMap &heatpumpInfo, const QVariantMap &evChargerInfo)
{
    QVariantMap requestMap;
    requestMap.insert("ntp", ntpInfos);
    requestMap.insert("photovoltaic", photovoltaicInfos);
    requestMap.insert("electric_demand", electricDemandInfo);
    if (!heatpumpInfo.isEmpty())
        requestMap.insert("heatpump", heatpumpInfo);

    if (!evChargerInfo.isEmpty())
        requestMap.insert("chargepoint", evChargerInfo);

    QByteArray requestData = QJsonDocument::fromVariant(requestMap).toJson(QJsonDocument::Compact);
    qCDebug(dcHemsOptimizer()) << "Request pv optimization...";
    qCDebug(dcHemsOptimizerTraffic()) << "-->" << qUtf8Printable(QJsonDocument::fromVariant(requestMap).toJson(QJsonDocument::Indented));

    return m_networkManager->post(buildRequest("/api/hems-pv/v1/pv-optimized/"), requestData);
}

QNetworkReply *HemsOptimizerInterface::electricPowerDemandStandardProfile(const QVector<QDateTime> &timestamps, double annualDemand)
{
    QVariantMap requestMap;
    requestMap.insert("annual_demand", annualDemand);
    requestMap.insert("timestamps", convertTimestampsToStringList(timestamps));

    QByteArray requestData = QJsonDocument::fromVariant(requestMap).toJson(QJsonDocument::Compact);
    qCDebug(dcHemsOptimizer()) << "Request pv electric demand from standrad profile based on annual demand of" << annualDemand << "kWh";
    qCDebug(dcHemsOptimizerTraffic()) << "-->" << qUtf8Printable(QJsonDocument::fromVariant(requestMap).toJson(QJsonDocument::Indented));
    return m_networkManager->post(buildRequest("/api/forecast/v1/electric/standard-load-profile"), requestData);
}

QNetworkReply *HemsOptimizerInterface::florHeatingPowerDemandStandardProfile(HouseType houseType, double livingArea, const QVariantMap &temperatureHistory, const QVariantMap &temperatureForecast)
{
    QVariantMap requestMap;
    QMetaEnum houseTypeMetaEnum = QMetaEnum::fromType<HemsOptimizerInterface::HouseType>();
    requestMap.insert("house_type", QString(houseTypeMetaEnum.valueToKey(houseType)).remove("HouseType"));
    requestMap.insert("area", livingArea);
    requestMap.insert("temperature_past", temperatureHistory);
    requestMap.insert("temperature_forecast", temperatureForecast);

    QByteArray requestData = QJsonDocument::fromVariant(requestMap).toJson(QJsonDocument::Compact);
    qCDebug(dcHemsOptimizer()) << "Request flor heating demand from standrad profile based" << houseType << livingArea << "m^2";
    qCDebug(dcHemsOptimizerTraffic()) << "-->" << qUtf8Printable(QJsonDocument::fromVariant(requestMap).toJson(QJsonDocument::Indented));
    return m_networkManager->post(buildRequest("/api/forecast/v1/heating/floor-heating"), requestData);
}

QNetworkRequest HemsOptimizerInterface::buildRequest(const QString &path)
{
    // Url
    QUrl url = m_apiBaseUrl;
    url.setPath(path);

    // Authentication
    QString token = m_username + ":" + m_password;
    QByteArray tokenData = token.toLocal8Bit().toBase64();
    QString headerData = "Basic " + tokenData;

    // Request
    QNetworkRequest request;
    request.setRawHeader("Authorization", headerData.toLocal8Bit());
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/json"));
    request.setUrl(url);
    return request;
}

QStringList HemsOptimizerInterface::convertTimestampsToStringList(const QVector<QDateTime> &timestamps)
{
    QStringList timestampList;
    for (int i = 0; i < timestamps.count(); i++) {
        timestampList << timestamps.at(i).toUTC().toString(Qt::ISODate);
    }
    return timestampList;
}
