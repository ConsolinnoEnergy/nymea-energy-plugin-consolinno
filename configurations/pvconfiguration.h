
#ifndef PVCONFIGURATION_H
#define PVCONFIGURATION_H

#include <QObject>
#include <QDebug>
//#include <QMetaObject>

#include <typeutils.h>

//#include "hemsoptimizerinterface.h"

class PvConfiguration
{
    Q_GADGET
    Q_PROPERTY(QUuid pvThingId READ pvThingId WRITE setPvThingId)
    Q_PROPERTY(double longitude READ longitude WRITE setLongitude USER true)
    Q_PROPERTY(double latitude READ latitude WRITE setLatitude USER true)
    Q_PROPERTY(double roofPitch READ roofPitch WRITE setRoofPitch USER true)
    Q_PROPERTY(double alignment READ alignment WRITE setAlignment USER true)
    Q_PROPERTY(double kwPeak READ kwPeak WRITE setKwPeak USER true)

public:
    PvConfiguration();

    ThingId pvThingId() const;
    void setPvThingId(const ThingId &pvThingId);

    double longitude() const;
    void setLongitude(const double longitude);

    double latitude() const;
    void setLatitude(const double latitude);

    int roofPitch() const;
    void setRoofPitch(const int roofPitch);

    int alignment() const;
    void setAlignment(const int alignment);

    float kwPeak() const;
    void setKwPeak(const float kwPeak);


    bool operator==(const PvConfiguration &other) const;
    bool operator!=(const PvConfiguration &other) const;


private:
    ThingId m_pvThingId;
    double m_latitude = 0;
    double m_longitude = 0;
    int m_roofPitch = 0;
    int m_alignment = 0;
    float m_kwPeak = 0;




};

//Q_DECLARE_METATYPE(PvConfiguration)
QDebug operator<<(QDebug debug, const PvConfiguration &pvConfig);

#endif // PVCONFIGURATION_H
