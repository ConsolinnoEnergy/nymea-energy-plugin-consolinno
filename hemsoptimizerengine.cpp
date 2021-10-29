#include "hemsoptimizerengine.h"

#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonParseError>

#include "loggingcategories.h"
Q_DECLARE_LOGGING_CATEGORY(dcConsolinnoExperience)

HemsOptimizerEngine::HemsOptimizerEngine(QObject *parent) :
    QObject(parent),
    m_networkManager(new QNetworkAccessManager(this))
{
    // Docs can be found here using the same username and password
    // https://lash-upstage.runner.consolinno.de/docs
    // All data values are kWh or kW
    m_apiBaseUrl = QUrl("https://lash-upstage.runner.consolinno.de");
    m_username = "nymea";
    m_password = "3aB!NnUJe@Rez*%f3JY7";

    QNetworkReply *reply = m_networkManager->get(buildRequest("/healthz"));
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
    connect(reply, &QNetworkReply::finished, this, [reply](){
        if (reply->error() != QNetworkReply::NoError) {
            qCWarning(dcConsolinnoExperience()) << "HemsOptimizer: Failed to get heathz status. The reply returned with error" << reply->errorString();
            return;
        }

        QByteArray data = reply->readAll();
        QJsonParseError error;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
        if (error.error != QJsonParseError::NoError) {
            qCWarning(dcConsolinnoExperience()) << "HemsOptimizer: Failed to parse healthz status data" << data << ":" << error.errorString();
            return;
        }

        qCDebug(dcConsolinnoExperience()) << "Get healthz status finished successfully:" << jsonDoc.toJson(QJsonDocument::Compact);
    });
}

QVariantMap HemsOptimizerEngine::buildRootMeterInformation(const QStringList &timestamps, Thing *rootMeter, double price)
{
    QVariantMap ntp;
    ntp.insert("uuid", rootMeter->id().toString());
    ntp.insert("priority", 1);
    ntp.insert("timestamps", timestamps);

    // optional schedule

    // Fill known prices depending on the price model, assume nor 0,3 €
    QVariantList priceList;
    for (int i = 0; i < timestamps.count(); i++) {
        priceList.append(price);
    }
    ntp.insert("price", priceList);

    QVariantList powerIn;
    for (int i = 0; i < timestamps.count(); i++) {
        powerIn.append(7.6);
    }
    ntp.insert("electric_power_in", powerIn);


    QVariantList powerOut;
    for (int i = 0; i < timestamps.count(); i++) {
        powerOut.append(7.6);
    }
    ntp.insert("electric_power_out", powerOut);
    return ntp;
}

QVariantMap HemsOptimizerEngine::buildPhotovoltaicInformation(const QStringList &timestamps, Thing *inverter, const QVariantList &forecast)
{
    QVariantMap dataMap;
    dataMap.insert("uuid", inverter->id().toString());
    dataMap.insert("priority", 1);
    dataMap.insert("timestamps", timestamps);
    dataMap.insert("forecast", forecast);
    return dataMap;
}

QVariantMap HemsOptimizerEngine::buildElectricDemandInformation(const QStringList &timestamps, const QUuid &householdUuid, const QVariantList &forecast)
{
    QVariantMap dataMap;
    dataMap.insert("uuid", householdUuid.toString());
    dataMap.insert("priority", 1);
    dataMap.insert("timestamps", timestamps);
    dataMap.insert("forecast", forecast);
    return dataMap;
}

QVariantMap HemsOptimizerEngine::buildHeatpumpInformation(const QStringList &timestamps, Thing *heatpump, double maxThermalEnergy, double soc, double electricPowerMax, const QVariantList &thermalDemandForecast, const QVariantList &copForecast, double rho)
{
    QVariantMap dataMap;
    dataMap.insert("uuid", heatpump->id().toString());
    dataMap.insert("priority", 1);
    dataMap.insert("timestamps", timestamps);
    dataMap.insert("is_on", true); // FIXME: find state
    dataMap.insert("thermal_energy_max", maxThermalEnergy); // kWh
    dataMap.insert("thermal_energy_max", soc);
    dataMap.insert("electric_power_max", electricPowerMax);
    dataMap.insert("thermal_demand_forecast", thermalDemandForecast);
    dataMap.insert("cop_forecast", copForecast);
    dataMap.insert("rho", rho);
    return dataMap;
}

QVariantMap HemsOptimizerEngine::buildEvChargerInformation(const QStringList &timestamps, Thing *evCharger, double maxPower, double minPower, double energyNeeded, const QDateTime &endTime)
{
    QVariantMap dataMap;
    dataMap.insert("uuid", evCharger->id().toString());
    dataMap.insert("priority", 1);
    dataMap.insert("timestamps", timestamps);
    dataMap.insert("electric_power_max", maxPower);
    dataMap.insert("electric_power_min", minPower);
    dataMap.insert("energy_needed", energyNeeded);
    dataMap.insert("finished_until", endTime.toString(Qt::ISODate));
    return dataMap;
}

QNetworkReply *HemsOptimizerEngine::pvOptimization(const QVariantMap &ntpInfos, const QVariantMap &photovoltaicInfos, const QVariantMap &electricDemandInfo, const QVariantMap &heatpumpInfo, const QVariantMap &evChargerInfo)
{
    QVariantMap requestMap;
    requestMap.insert("ntp", ntpInfos);
    requestMap.insert("photovoltaic", photovoltaicInfos);
    requestMap.insert("electric_demand", electricDemandInfo);
    requestMap.insert("heatpump", heatpumpInfo);
    requestMap.insert("chargepoint", evChargerInfo);
    return m_networkManager->post(buildRequest("/api/hems-pv/v1/pv-optimized/"), QJsonDocument::fromVariant(requestMap).toJson(QJsonDocument::Compact));
}

QNetworkRequest HemsOptimizerEngine::buildRequest(const QString &path)
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
