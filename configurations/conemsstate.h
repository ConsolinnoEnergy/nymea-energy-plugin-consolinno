/* Copyright (C) Consolinno Energy GmbH - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef CONEMSSTATE_H
#define CONEMSSTATE_H



#include <QTime>
#include <QObject>
#include <QDebug>
#include <QJsonObject>


#include <typeutils.h>

class ConEMSState
{
    Q_GADGET
    Q_PROPERTY(QJsonObject currentState READ currentState WRITE setCurrentState USER true)
    Q_PROPERTY(long timestamp READ timestamp WRITE setTimestamp USER true)

public:
    ConEMSState();
    QJsonObject conemsState;


    QJsonObject currentState() const;
    void setCurrentState(const QJsonObject currentState);

    long timestamp() const;
    void setTimestamp(const long timestamp);

    bool operator==(const ConEMSState &other) const;
    bool operator!=(const ConEMSState &other) const;

private:
    QJsonObject m_currentState = QJsonObject() ;
    long m_timestamp = 0;



};

QDebug operator<<(QDebug debug, const ConEMSState &conEMSState);

//Q_DECLARE_METATYPE(ConEMSState)
#endif // CONEMSSTATE_H
