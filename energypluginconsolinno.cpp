/* Copyright (C) Consolinno Energy GmbH - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */
#include <QFile>
#include <QTextStream>
#include <QStringList>

#include "energypluginconsolinno.h"
#include "energyengine.h"
#include "consolinnojsonhandler.h"

#include <jsonrpc/jsonrpcserver.h>
#include <loggingcategories.h>


NYMEA_LOGGING_CATEGORY(dcConsolinnoEnergy, "ConsolinnoEnergy")

bool parseVersionFile(const QString& filePath, HEMSVersionInfo& info) {
    // Open the file
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning(dcConsolinnoEnergy()) << "Failed to open file: " << filePath;
        return false;
    }

    // Read the single line
    QTextStream in(&file);
    QString line = in.readLine();

    // Split the line into components
    QStringList parts = line.split(' ', QString::SkipEmptyParts);

    if (parts.size() == 3) {
        info.product = parts[0];
        info.version = parts[1];
        QStringList versionParts = parts[1].split('.');
        if (versionParts.size() == 3) {
            info.major = versionParts[0].toInt();
            info.minor = versionParts[1].toInt();
            info.patch = versionParts[2].toInt();
        } else {
            qWarning(dcConsolinnoEnergy()) << "Invalid version format: " << parts[1];
            return false;
        }
        
        info.stage = parts[2];
        return true;
    } else {
        qWarning(dcConsolinnoEnergy()) << "Invalid line: " << line;
        return false;
    }
}


EnergyPluginConsolinno::EnergyPluginConsolinno()
{
    qCDebug(dcConsolinnoEnergy()) << "Loading consolinno energy plugin";
}

void EnergyPluginConsolinno::init()
{
    qCDebug(dcConsolinnoEnergy()) << "Initializing energy plugin...";
    HEMSVersionInfo versionInfo;
    parseVersionFile("/etc/consolinno-release", versionInfo); 
    EnergyEngine *energyEngine = new EnergyEngine(thingManager(), energyManager(), this);
    jsonRpcServer()->registerExperienceHandler(new ConsolinnoJsonHandler(energyEngine, versionInfo,  this), versionInfo.major, versionInfo.minor, versionInfo.patch);
}
