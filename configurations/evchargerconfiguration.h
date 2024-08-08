/* Copyright (C) Consolinno Energy GmbH - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef EVCHARGERCONFIGURATION_H
#define EVCHARGERCONFIGURATION_H
#include <QObject>
#include <QDebug>

#include <typeutils.h>

class EVchargerConfiguration
{
    Q_GADGET
    Q_PROPERTY(QUuid evchargerThingId READ evchargerThingId WRITE setEVchargerThingId)
    Q_PROPERTY(bool controllableLocalSystem READ controllableLocalSystem WRITE setControllableLocalSystem USER true)

public:
    EVchargerConfiguration();

    ThingId evchargerThingId() const;
    void setEVchargerThingId(const ThingId &evchargerThingId);

    bool controllableLocalSystem() const;
    void setControllableLocalSystem(bool controllableLocalSystem);

    bool operator==(const EVchargerConfiguration &other) const;
    bool operator!=(const EVchargerConfiguration &other) const;

private:
    ThingId m_evchargerThingId;
    bool m_controllableLocalSystem = false;
};

QDebug operator<<(QDebug debug, const EVchargerConfiguration &evchargerConfig);

#endif // EVCHARGERCONFIGURATION_H
