/* Copyright (C) Consolinno Energy GmbH - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef ENERGYPLUGINCONSOLINNO_H
#define ENERGYPLUGINCONSOLINNO_H

#include <QLoggingCategory>

#include <energyplugin.h>

Q_DECLARE_LOGGING_CATEGORY(dcConsolinnoEnergy)

struct HEMSVersionInfo {
    QString product;
    QString version;
    int major;
    int minor;
    int patch;
    QString stage;
};

class EnergyPluginConsolinno: public EnergyPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "io.nymea.EnergyPlugin")
    Q_INTERFACES(EnergyPlugin)

public:
    EnergyPluginConsolinno();

    void init() override;
    HEMSVersionInfo m_versionInfo;

};

#endif // ENERGYPLUGINCONSOLINNO_H
