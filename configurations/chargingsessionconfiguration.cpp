#include "chargingsessionconfiguration.h"

ChargingSessionConfiguration::ChargingSessionConfiguration()
{

}

ThingId ChargingSessionConfiguration::chargingSessionThingId() const
{
    return m_chargingSessionthingId;
}

void ChargingSessionConfiguration::setChargingSessionthingId(const ThingId &chargingSessionThingId)
{
    m_chargingSessionthingId = chargingSessionThingId;
}

ThingId ChargingSessionConfiguration::carThingId() const
{
    return m_carThingId;
}

void ChargingSessionConfiguration::setCarThingId(const ThingId &carThingId)
{
    m_carThingId = carThingId;
}

ThingId ChargingSessionConfiguration::evChargerThingId() const
{
    return m_evChargerThingId;
}

void ChargingSessionConfiguration::setEvChargerThingId(const ThingId &evChargerThingId)
{
    m_evChargerThingId = evChargerThingId;
}

QTime ChargingSessionConfiguration::startedAt() const
{
    return m_started_at;
}

void ChargingSessionConfiguration::setStartedAt(const QTime started_at)
{
    m_started_at = started_at;
}

QTime ChargingSessionConfiguration::finishedAt() const
{
    return m_finished_at;
}

void ChargingSessionConfiguration::setFinishedAt(const QTime finished_at)
{
    m_finished_at = finished_at;
}

float ChargingSessionConfiguration::initialBatteryEnergy() const
{
    return m_initial_battery_energy;
}

void ChargingSessionConfiguration::setInitialBatteryEnergy(const float initial_battery_energy)
{
    m_initial_battery_energy = initial_battery_energy;
}

int ChargingSessionConfiguration::duration() const
{
    return m_duration;
}

void ChargingSessionConfiguration::setDuration(const int duration)
{
    m_duration = duration;
}

float ChargingSessionConfiguration::energyCharged() const
{
    return m_energy_charged;
}

void ChargingSessionConfiguration::setEnergyCharged(const float energy_charged)
{
    m_energy_charged = energy_charged;
}

float ChargingSessionConfiguration::energyBattery() const
{
    return m_energy_battery;
}

void ChargingSessionConfiguration::setEnergyBattery(const float energy_battery)
{
    m_energy_battery = energy_battery;
}

int ChargingSessionConfiguration::batteryLevel() const
{
    return m_battery_level;
}

void ChargingSessionConfiguration::setBatteryLevel(const int battery_level)
{
    m_battery_level = battery_level;
}

bool ChargingSessionConfiguration::operator ==(const ChargingSessionConfiguration &other) const
{
    return m_chargingSessionthingId == other.chargingSessionThingId() &&
            m_carThingId == other.carThingId() &&
            m_evChargerThingId == other.evChargerThingId() &&
            m_started_at == other.startedAt() &&
            m_finished_at == other.finishedAt() &&
            m_initial_battery_energy == other.initialBatteryEnergy() &&
            m_duration == other.duration() &&
            m_energy_charged == other.energyCharged() &&
            m_energy_battery == other.energyBattery() &&
            m_battery_level == other.batteryLevel();
}

bool ChargingSessionConfiguration::operator !=(const ChargingSessionConfiguration &other) const
{
    return !(*this == other);
}



QDebug operator<<(QDebug debug, const ChargingSessionConfiguration &chargingSessionConfig)
{
    debug.nospace() << "ChargingSessionConfiguration(" << chargingSessionConfig.chargingSessionThingId().toString();
    debug.nospace() << "Car: " << chargingSessionConfig.carThingId().toString();
    debug.nospace() << "Wallbox: " << chargingSessionConfig.evChargerThingId().toString();
    debug.nospace() << ", started at:  " << chargingSessionConfig.startedAt().toString();
    if (!chargingSessionConfig.finishedAt().isNull()){
        debug.nospace() << " not finished yet";
    } else {
        debug.nospace() << chargingSessionConfig.finishedAt().toString();
    }
    debug.nospace() << ", initial battery energy:  " << chargingSessionConfig.initialBatteryEnergy();
    debug.nospace() << ", duration in seconds:  " << chargingSessionConfig.duration();
    debug.nospace() << ", energy charged:  " << chargingSessionConfig.energyCharged() << "kWh";
    debug.nospace() << ", energy battery:  " << chargingSessionConfig.energyBattery() << "kWh";
    debug.nospace() << ", battery level:  " << chargingSessionConfig.batteryLevel() << "%";
    debug.nospace() << ")";
    return debug.maybeSpace();
}














