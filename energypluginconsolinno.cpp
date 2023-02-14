/* Copyright (C) Consolinno Energy GmbH - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include "energypluginconsolinno.h"
#include "energyengine.h"
#include "consolinnojsonhandler.h"

#include <jsonrpc/jsonrpcserver.h>
#include <loggingcategories.h>

NYMEA_LOGGING_CATEGORY(dcConsolinnoEnergy, "ConsolinnoEnergy")

EnergyPluginConsolinno::EnergyPluginConsolinno()
{
    qCDebug(dcConsolinnoEnergy()) << "Loading consolinno energy plugin";
}

void EnergyPluginConsolinno::init()
{
    qCDebug(dcConsolinnoEnergy()) << "Initializing energy plugin...";
    EnergyEngine *energyEngine = new EnergyEngine(thingManager(), energyManager(), this);
    jsonRpcServer()->registerExperienceHandler(new ConsolinnoJsonHandler(energyEngine, this), 0, 1);
}
