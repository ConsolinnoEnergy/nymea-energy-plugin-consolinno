#ifndef USERCONFIGURATION_H
#define USERCONFIGURATION_H

#include <QTime>
#include <QObject>
#include <QDebug>

#include <typeutils.h>

class UserConfiguration
{

    Q_GADGET
    Q_PROPERTY(QUuid userConfigID READ userConfigID CONSTANT)
    Q_PROPERTY(QUuid lastSelectedCar READ lastSelectedCar WRITE setLastSelectedCar USER true)
    Q_PROPERTY(int defaultChargingMode READ defaultChargingMode WRITE setDefaultChargingMode USER true)

public:
    UserConfiguration();

    ThingId userConfigID()const;

    QUuid lastSelectedCar() const;
    void setLastSelectedCar(const QUuid &lastSelectedCar);

    int defaultChargingMode() const;
    void setDefaultChargingMode(const int &defaultChargingMode);

    bool operator==(const UserConfiguration &other) const;
    bool operator!=(const UserConfiguration &other) const;



private:

    QUuid m_userConfigId = "528b3820-1b6d-4f37-aea7-a99d21d42e72";
    QUuid m_lastSelectedCar = "282d39a8-3537-4c22-a386-b31faeebbb55";
    int m_defaultChargingMode = 0;

};

QDebug operator<<(QDebug debug, const UserConfiguration &userConfig);


#endif // USERCONFIGURATION_H
