/* Copyright (C) Consolinno Energy GmbH - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

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

QString UserConfiguration::installerName() const
{
    return m_installerName;
}

void UserConfiguration::setInstallerName(const QString &installerName)
{
    m_installerName = installerName;
}

QString UserConfiguration::installerEmail() const
{
    return m_installerEmail;
}

void UserConfiguration::setInstallerEmail(const QString &installerEmail)
{
    m_installerEmail = installerEmail;
}

QString UserConfiguration::installerPhoneNr() const
{
    return m_installerPhoneNr;
}

void UserConfiguration::setInstallerPhoneNr(const QString &installerPhoneNr)
{
    m_installerPhoneNr = installerPhoneNr;
}

QString UserConfiguration::installerWorkplace() const
{
    return m_installerWorkplace;
}

void UserConfiguration::setInstallerWorkplace(const QString &installerWorkplace)
{
    m_installerWorkplace = installerWorkplace;
}

bool UserConfiguration::operator==(const UserConfiguration &other) const
{
    return m_userConfigId == other.userConfigID() &&
            m_lastSelectedCar == other.lastSelectedCar() &&
            m_defaultChargingMode == other.defaultChargingMode() &&
            m_installerEmail == other.installerEmail() &&
            m_installerPhoneNr == other.installerPhoneNr() &&
            m_installerWorkplace == other.installerWorkplace() &&
            m_installerName == other.installerName();


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
    debug.nospace() << " installer Name: " << userConfig.installerName();
    debug.nospace() << " installer e-mail: " << userConfig.installerEmail();
    debug.nospace() << " installer Phonenumber: " << userConfig.installerPhoneNr();
    debug.nospace() << " installer Workplace: " << userConfig.installerWorkplace();
    debug.nospace() << ")";
    return debug.maybeSpace();
}
