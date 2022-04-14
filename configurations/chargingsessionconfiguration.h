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
    Q_PROPERTY(QTime startedAt READ startedAt WRITE setStartedAt USER true)
    Q_PROPERTY(QTime finishedAt READ finishedAt WRITE setFinishedAt USER true)
    Q_PROPERTY(float initialBatteryEnergy READ initialBatteryEnergy WRITE setInitialBatteryEnergy USER true)
    Q_PROPERTY(int duration READ duration WRITE setDuration USER true)
    Q_PROPERTY(float energyCharged READ energyCharged WRITE setEnergyCharged USER true)
    Q_PROPERTY(float energyBattery READ energyBattery WRITE setEnergyBattery USER true)
    Q_PROPERTY(int batteryLevel READ batteryLevel WRITE setBatteryLevel USER true)




public:
    ChargingSessionConfiguration();



    ThingId carThingId() const;
    void setCarThingId(const ThingId &carThingId);

    ThingId evChargerThingId() const;
    void setEvChargerThingId(const ThingId &evChargerThingId);

    QTime startedAt() const;
    void setStartedAt(const QTime started_at);

    QTime finishedAt() const;
    void setFinishedAt(const QTime finished_at);

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

    bool operator == (const ChargingSessionConfiguration &other) const;
    bool operator != (const ChargingSessionConfiguration &other) const;


private:
    ThingId m_carThingId;
    ThingId m_evChargerThingId;
    QTime m_started_at = QTime(3,2) ;
    QTime m_finished_at = QTime(4,2);
    float m_initial_battery_energy;
    int m_duration;
    float m_energy_charged = 0;
    float m_energy_battery;
    int m_battery_level = 0;



};

QDebug operator<<(QDebug debug, const ChargingSessionConfiguration &chargingSessionConfig);

#endif // CHARGINGSESSIONCONFIGURATION_H
