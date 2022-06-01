#include "conemsstate.h"

ConEMSState::ConEMSState()
{

}

QUuid ConEMSState::ConEMSStateID() const
{
    return m_conEMSStateID;
}

void ConEMSState::setConEMSStateID(const QUuid &conEMSStateID)
{
    m_conEMSStateID = conEMSStateID;
}

ConEMSState::State ConEMSState::currentState() const
{
    return m_currentState;
}

void ConEMSState::setCurrentState(const State currentState)
{
    m_currentState = currentState;
}

int ConEMSState::operationMode() const
{
    return m_operationMode;
}

void ConEMSState::setOperationMode(const int &operationMode)
{
    m_operationMode = operationMode;
}

int ConEMSState::timestamp() const
{
    return m_timestamp;
}

void ConEMSState::setTimestamp(const int timestamp)
{
    m_timestamp = timestamp;
}

bool ConEMSState::operator==(const ConEMSState &other) const
{
    return m_conEMSStateID == other.ConEMSStateID() &&
           m_currentState == other.currentState() &&
           m_timestamp == other.timestamp() &&
           m_operationMode == other.operationMode();

}

bool ConEMSState::operator!=(const ConEMSState &other) const
{
    return !(*this == other);
}


QDebug operator<<(QDebug debug, const ConEMSState &conEMSState)
{
    debug.nospace() << "ConEMSState (" << conEMSState.ConEMSStateID().toString();
    debug.nospace() << ", " << conEMSState.currentState();
    debug.nospace() << ", operation Mode: " << conEMSState.operationMode();
    debug.nospace() << ", timestamp: " << conEMSState.timestamp();
    debug.nospace() << ")";
    return debug.maybeSpace();
}
