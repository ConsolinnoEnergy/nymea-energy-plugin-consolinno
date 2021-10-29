#ifndef HEMSCONFIGURATION_H
#define HEMSCONFIGURATION_H

#include <QObject>

class HemsConfiguration : public QObject
{
    Q_OBJECT
public:
    explicit HemsConfiguration(QObject *parent = nullptr);

signals:

};

#endif // HEMSCONFIGURATION_H
