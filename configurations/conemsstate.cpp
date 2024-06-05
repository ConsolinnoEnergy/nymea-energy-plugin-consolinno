/* Copyright (C) Consolinno Energy GmbH - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include "conemsstate.h"

ConEMSState::ConEMSState()
{

}


QJsonObject ConEMSState::currentState() const
{
    return m_currentState;
}

void ConEMSState::setCurrentState(const QJsonObject currentState)
{
    m_currentState = currentState;
}

long long ConEMSState::timestamp() const
{
    return m_timestamp;
}

void ConEMSState::setTimestamp(const long long timestamp)
{
    m_timestamp = timestamp;
}

bool ConEMSState::operator==(const ConEMSState &other) const
{
    return m_currentState == other.currentState() &&
           m_timestamp == other.timestamp();
}

bool ConEMSState::operator!=(const ConEMSState &other) const
{
    return !(*this == other);
}


QDebug operator<<(QDebug debug, const ConEMSState &conEMSState)
{
    debug.nospace() << "ConEMSState (";
    debug.nospace() << "timestamp: " << conEMSState.timestamp();
    debug.nospace() << ")";
    return debug.maybeSpace();
}
