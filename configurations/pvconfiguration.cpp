/* Copyright (C) Consolinno Energy GmbH - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include "pvconfiguration.h"

PvConfiguration::PvConfiguration()
{

}

ThingId PvConfiguration::pvThingId() const
{
 return m_pvThingId;
}

void PvConfiguration::setPvThingId(const ThingId &pvThingId)
{
    m_pvThingId = pvThingId;
}

float PvConfiguration::longitude() const
{
 return m_longitude;
}

void PvConfiguration::setLongitude(const float longitude)
{
    m_longitude = longitude;
}


float PvConfiguration::latitude() const
{
 return m_latitude;
}

void PvConfiguration::setLatitude(const float latitude)
{
    m_latitude = latitude;
}

int PvConfiguration::roofPitch() const
{
    return m_roofPitch;
}

void PvConfiguration::setRoofPitch(const int roofPitch)
{
    m_roofPitch = roofPitch;
}

int PvConfiguration::alignment() const
{
    return m_alignment;
}

void PvConfiguration::setAlignment(const int alignment)
{
    m_alignment = alignment;
}

float PvConfiguration::kwPeak() const
{
    return m_kwPeak;
}

void PvConfiguration::setKwPeak(const float kwPeak)
{
    m_kwPeak = kwPeak;
}



bool PvConfiguration::operator==(const PvConfiguration &other) const
{
    return m_pvThingId == other.pvThingId() &&
           m_longitude == other.longitude()&&
           m_roofPitch == other.roofPitch()&&
           m_alignment == other.alignment()&&
           m_kwPeak == other.kwPeak()&&
           m_latitude == other.latitude();

}

bool PvConfiguration::operator!=(const PvConfiguration &other) const
{
       return !(*this == other);
}

QDebug operator<<(QDebug debug, const PvConfiguration &pvConfig)
{
    debug.nospace() << "PvConfiguration(" << pvConfig.pvThingId().toString();
    debug.nospace() << ", longitude: " << pvConfig.longitude();
    debug.nospace() << ", latitude: " << pvConfig.latitude();
    debug.nospace() << ", roofPitch: " << pvConfig.roofPitch();
    debug.nospace() << ", alignment: " << pvConfig.alignment();
    debug.nospace() << ", kwPeak: " << pvConfig.kwPeak();
    debug.nospace() << ")";
    return debug.maybeSpace();
}
