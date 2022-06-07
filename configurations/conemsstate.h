#ifndef CONEMSSTATE_H
#define CONEMSSTATE_H



#include <QTime>
#include <QObject>
#include <QDebug>

#include <typeutils.h>

class ConEMSState
{
    Q_GADGET
    Q_PROPERTY(QUuid ConEMSStateID READ ConEMSStateID WRITE setConEMSStateID)
    Q_PROPERTY(State currentState READ currentState WRITE setCurrentState USER true)
    Q_PROPERTY(int operationMode READ operationMode WRITE setOperationMode USER true)
    Q_PROPERTY(int timestamp READ timestamp WRITE setTimestamp USER true)

public:
    ConEMSState();

    enum State {
        Unknown,
        Running,
        Optimizer_Busy,
        Restarting,
        Error
    };

    Q_ENUM(State)

    QUuid ConEMSStateID() const;
    void setConEMSStateID( const QUuid &conEMSStateID);

    ConEMSState::State currentState() const;
    void setCurrentState(const State currentState);

    int operationMode() const;
    void setOperationMode( const int &operationMode);

    int timestamp() const;
    void setTimestamp(const int timestamp);



    bool operator==(const ConEMSState &other) const;
    bool operator!=(const ConEMSState &other) const;



private:
    QUuid m_conEMSStateID = "f002d80e-5f90-445c-8e95-a0256a0b464e" ;
    ConEMSState::State m_currentState = Unknown;
    int m_operationMode = 0;
    int m_timestamp = 0;



};

QDebug operator<<(QDebug debug, const ConEMSState &conEMSState);

//Q_DECLARE_METATYPE(ConEMSState)
#endif // CONEMSSTATE_H
