/* Copyright (C) Consolinno Energy GmbH - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef CHARGINGSESSIONCONFIGURATION_H
#define CHARGINGSESSIONCONFIGURATION_H

#include <QTime>
#include <QObject>
#include <QDebug>

#include <typeutils.h>


class ChargingSessionConfiguration
{
    Q_GADGET
    Q_PROPERTY(QUuid carThingId READ carThingId WRITE setCarThingId USER true)
    Q_PROPERTY(QUuid evChargerThingId READ evChargerThingId WRITE setEvChargerThingId USER true)
    Q_PROPERTY(QString startedAt READ startedAt WRITE setStartedAt USER true)
    Q_PROPERTY(QString finishedAt READ finishedAt WRITE setFinishedAt USER true)
    Q_PROPERTY(float initialBatteryEnergy READ initialBatteryEnergy WRITE setInitialBatteryEnergy USER true)
    Q_PROPERTY(int duration READ duration WRITE setDuration USER true)
    Q_PROPERTY(float energyCharged READ energyCharged WRITE setEnergyCharged USER true)
    Q_PROPERTY(float energyBattery READ energyBattery WRITE setEnergyBattery USER true)
    Q_PROPERTY(int batteryLevel READ batteryLevel WRITE setBatteryLevel USER true)
    Q_PROPERTY(int state READ state WRITE setState USER true)
    Q_PROPERTY(QUuid sessionId READ sessionId WRITE setSessionId USER true)
    Q_PROPERTY(int timestamp READ timestamp WRITE setTimestamp USER true )



public:
    ChargingSessionConfiguration();

    enum State {
        Initiation = 0,
        Running = 1,
        ToBeCanceled = 2,
        Canceled = 3

    };
    Q_ENUM(State);


    ThingId carThingId() const;
    void setCarThingId(const ThingId &carThingId);

    ThingId evChargerThingId() const;
    void setEvChargerThingId(const ThingId &evChargerThingId);

    QString startedAt() const;
    void setStartedAt(const QString started_at);

    QString finishedAt() const;
    void setFinishedAt(const QString finished_at);

    float initialBatteryEnergy() const;
    void setInitialBatteryEnergy( const float initial_battery_energy);

    int duration()const;
    void setDuration(const int duration);

    float energyCharged() const;
    void setEnergyCharged(const float energy_charged);

    float energyBattery() const;
    void setEnergyBattery(const float energy_battery);

    int batteryLevel() const;
    void setBatteryLevel(const int battery_level);

    QUuid sessionId() const;
    void setSessionId(const QUuid sessionId);

    int state() const;
    void setState(const int state);

    int timestamp() const;
    void setTimestamp(const int timestamp);


    bool operator == (const ChargingSessionConfiguration &other) const;
    bool operator != (const ChargingSessionConfiguration &other) const;


private:
    ThingId m_carThingId;
    ThingId m_evChargerThingId;
    QString m_started_at = "";
    QString m_finished_at = "";
    float m_initial_battery_energy = 0;
    int m_duration = 0;
    float m_energy_charged = 0;
    float m_energy_battery = 0;
    int m_battery_level = 0;
    QUuid m_sessionId;
    int m_state = 0;
    int m_timestamp = 0;




};

QDebug operator<<(QDebug debug, const ChargingSessionConfiguration &chargingSessionConfig);

#endif // CHARGINGSESSIONCONFIGURATION_H
