#include "userconfiguration.h"

UserConfiguration::UserConfiguration()
{

}

ThingId UserConfiguration::userConfigID() const
{
    return m_userConfigId;
}

QUuid UserConfiguration::lastSelectedCar() const
{
    return m_lastSelectedCar;
}

void UserConfiguration::setLastSelectedCar(const QUuid &lastselectedCar)
{
    m_lastSelectedCar = lastselectedCar;
}

int UserConfiguration::defaultChargingMode() const
{
    return m_defaultChargingMode;
}

void UserConfiguration::setDefaultChargingMode(const int &chargingMode)
{
    m_defaultChargingMode = chargingMode;
}

bool UserConfiguration::operator==(const UserConfiguration &other) const
{
    return m_userConfigId == other.userConfigID() &&
            m_lastSelectedCar == other.lastSelectedCar() &&
            m_defaultChargingMode == other.defaultChargingMode();
}

bool UserConfiguration::operator!=(const UserConfiguration &other) const
{
    return !(*this == other);
}

QDebug operator<<(QDebug debug, const UserConfiguration &userConfig)
{
    debug.nospace() << " UserConfiguration(" << userConfig.userConfigID().toString();
    debug.nospace() << " Last Selected Car: " << userConfig.lastSelectedCar().toString();
    debug.nospace() << " Default Charging Mode: " << userConfig.defaultChargingMode();
    debug.nospace() << ")";
    return debug.maybeSpace();
}
