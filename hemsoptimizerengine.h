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
    QVariantMap buildRootMeterInformation(const QStringList &timestamps, Thing *rootMeter, double price);
    QVariantMap buildPhotovoltaicInformation(const QStringList &timestamps, Thing *inverter, const QVariantList &forecast);
    QVariantMap buildElectricDemandInformation(const QStringList &timestamps, const QUuid &householdUuid, const QVariantList &forecast);
    QVariantMap buildHeatpumpInformation(const QStringList &timestamps, Thing *heatpump, double maxThermalEnergy, double soc, double electricPowerMax, const QVariantList &thermalDemandForecast, const QVariantList &copForecast, double rho = 0.1);
    QVariantMap buildEvChargerInformation(const QStringList &timestamps, Thing *evCharger, double maxPower, double minPower, double energyNeeded, const QDateTime &endTime);

    // Request optimization schedules
    QNetworkReply *pvOptimization(const QVariantMap &ntpInfos, const QVariantMap &photovoltaicInfos, const QVariantMap &electricDemandInfo, const QVariantMap &heatpumpInfo, const QVariantMap &evChargerInfo);

signals:

private:
    QNetworkAccessManager *m_networkManager = nullptr;
    QUrl m_apiBaseUrl;
    QString m_username;
    QString m_password;

    QNetworkRequest buildRequest(const QString &path);
};

#endif // HEMSOPTIMIZERENGINE_H
