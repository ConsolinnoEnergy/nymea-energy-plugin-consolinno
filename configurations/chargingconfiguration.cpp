/* Copyright (C) Consolinno Energy GmbH - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include "chargingconfiguration.h"

ChargingConfiguration::ChargingConfiguration() {}

ThingId ChargingConfiguration::evChargerThingId() const {
  return m_evChargerThingId;
}

void ChargingConfiguration::setEvChargerThingId(
    const ThingId &evChargerThingId) {
  m_evChargerThingId = evChargerThingId;
}

int ChargingConfiguration::optimizationMode() const {
  return m_optimizationMode;
}

void ChargingConfiguration::setOptimizationMode(int optimizationMode) {
  m_optimizationMode = optimizationMode;
}

int ChargingConfiguration::optimizationModeBase() {
  if (optimizationMode() < 1000) {
    return 0;
  }
  if (optimizationMode() >= 1000 && optimizationMode() < 2000) {
    return 1;
  }
  if (optimizationMode() >= 2000 && optimizationMode() < 3000) {
    return 2;
  }
  if (optimizationMode() >= 3000 && optimizationMode() < 4000) {
    return 3;
  }
  return -1;
}

bool ChargingConfiguration::optimizationEnabled() const {
  return m_optimizationEnabled;
}

void ChargingConfiguration::setOptimizationEnabled(bool optimizationEnabled) {
  m_optimizationEnabled = optimizationEnabled;
}

ThingId ChargingConfiguration::carThingId() const { return m_carThingId; }

void ChargingConfiguration::setCarThingId(const ThingId &carThingId) {
  m_carThingId = carThingId;
}

QString ChargingConfiguration::endTime() const { return m_endTime; }

void ChargingConfiguration::setEndTime(const QString &endTime) {
  m_endTime = endTime;
}

uint ChargingConfiguration::targetPercentage() const {
  return m_targetPercentage;
}

void ChargingConfiguration::setTargetPercentage(uint targetPercentage) {
  m_targetPercentage = targetPercentage;
}

QUuid ChargingConfiguration::uniqueIdentifier() const {
  return m_uniqueIdentifier;
}

void ChargingConfiguration::setUniqueIdentifier(QUuid uniqueIdentifier) {
  m_uniqueIdentifier = uniqueIdentifier;
}

bool ChargingConfiguration::isValid() const {
  return !m_evChargerThingId.isNull() && !m_carThingId.isNull();
}

bool ChargingConfiguration::controllableLocalSystem() const {
    return m_controllableLocalSystem;
}

void ChargingConfiguration::setControllableLocalSystem(bool controllableLocalSystem) {
    m_controllableLocalSystem = controllableLocalSystem;
}

bool ChargingConfiguration::operator==(
    const ChargingConfiguration &other) const {
  return m_evChargerThingId == other.evChargerThingId() &&
         m_optimizationEnabled == other.optimizationEnabled() &&
         m_optimizationMode == other.optimizationMode() &&
         m_carThingId == other.carThingId() && m_endTime == other.endTime() &&
         m_uniqueIdentifier == other.uniqueIdentifier() &&
         m_targetPercentage == other.targetPercentage();
}

bool ChargingConfiguration::operator!=(
    const ChargingConfiguration &other) const {
  return !(*this == other);
}

QDebug operator<<(QDebug debug, const ChargingConfiguration &chargingConfig) {
  debug.nospace() << "ChargingConfiguration("
                  << chargingConfig.evChargerThingId().toString();
  debug.nospace() << "unique Identifier: "
                  << chargingConfig.uniqueIdentifier().toString();
  debug.nospace() << "optimization: "
                  << (chargingConfig.optimizationEnabled() ? "enabled"
                                                           : "disabled");
  if (!chargingConfig.carThingId().isNull()) {
    debug.nospace() << ", assigned car: "
                    << chargingConfig.carThingId().toString();
  } else {
    debug.nospace() << ", no car assigned";
  }
  debug.nospace() << ", optimization Mode: "
                  << chargingConfig.optimizationMode();
  debug.nospace() << ", target percentage: "
                  << chargingConfig.targetPercentage() << "%";
  debug.nospace() << ", target time: " << chargingConfig.endTime();
  debug.nospace() << "CLS" << (chargingConfig.controllableLocalSystem() ? "enabled" : "disabled");
  debug.nospace() << ")";
  return debug.maybeSpace();
}
