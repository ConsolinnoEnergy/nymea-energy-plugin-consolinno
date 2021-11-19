#ifndef WEATHERDATAPROVIDER_H
#define WEATHERDATAPROVIDER_H

#include <QObject>

class WeatherDataProvider : public QObject
{
    Q_OBJECT
public:
    explicit WeatherDataProvider(QObject *parent = nullptr);

signals:

};

#endif // WEATHERDATAPROVIDER_H
