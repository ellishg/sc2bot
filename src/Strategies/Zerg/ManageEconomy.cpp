#include "ZergBot.h"

using namespace std;
using namespace sc2;

// TODO:
// Keep a map of hives to a list of drones.
// If a hive is destroyed, find work for all owned drones.
// If drone is destroyed or is producing a building, remove it from hive.
// If drone is created, assign to closest hive that needs drones.
// I'll need to keep track of pending drones.
// I'll need to decide which hive should produce drones. I don't want one hive producing all.

// I need to find a way to balance mining minerals and gas.
// Maybe have extra drones for producing buildings.

void ZergBot::ManageDrones::OnGameStart() {
  // auto hive = GetHives(observation)[0];
  // auto drones = observation->GetUnits(Unit::Alliance::Self,
  //                                     IsUnit(UNIT_TYPEID::ZERG_DRONE));
  // for (auto drone : drones)
  //   droneList.AssignDroneToHive(drone.tag, hive.tag);
}

void ZergBot::ManageDrones::OnPeriod() {
  for (auto hive : GetHives(observation)) {
    cout << pendingDronesForHive[hive.tag] << endl;
    ssize_t harvesterDeficit = hive.ideal_harvesters
        - (hive.assigned_harvesters + pendingDronesForHive[hive.tag]);
    if (harvesterDeficit > 0) {
      // TODO: It is wasteful to call `GetUnits` every time.
      Units larvae = observation->GetUnits(Unit::Alliance::Self,
                                           IsUnit(UNIT_TYPEID::ZERG_LARVA));
      if (larvae.empty() || not IsUnitAbilityAvailable(
                     query, larvae[0].tag, ABILITY_ID::TRAIN_DRONE))
        break;
      // TODO: Pick closest larva.
      action->UnitCommand(larvae[0].tag, ABILITY_ID::TRAIN_DRONE);
      // TODO: Set dest to mineral field.
      pendingDronesForHive[hive.tag]++;
      // harvesterDeficit--;
    }
  }
}

void ZergBot::ManageDrones::OnUnitCreated(const Unit &unit) {
  if (IsDrone(unit)) {
    uint64_t hiveTag;
    if (FindNearestUnit(unit.pos, GetHives(observation), &hiveTag)) {
      pendingDronesForHive[hiveTag] = (pendingDronesForHive[hiveTag] == 0)
            ? 0 : pendingDronesForHive[hiveTag] - 1;
    }
    AssignWork(unit);
  }
  switch (unit.unit_type.ToType()) {
    case UNIT_TYPEID::ZERG_HIVE:
    case UNIT_TYPEID::ZERG_LAIR:
    case UNIT_TYPEID::ZERG_HATCHERY:
      pendingDronesForHive[unit.tag] = 0;
    default:
      break;
  }
}
void ZergBot::ManageDrones::OnUnitIdle(const Unit &unit) {
  if (IsDrone(unit))
    AssignWork(unit);
}

void ZergBot::ManageDrones::AssignWork(const Unit &drone) {
  // TODO: Check for pending actions
  MineMinerals(drone);
  // MineVespeneGas(drone);
}

void ZergBot::ManageDrones::MineMinerals(const Unit &drone) {
  // Tag mineralFieldTag;
  // const Units mineralFields = observation->GetUnits(
  //         Unit::Alliance::Neutral, IsUnit(UNIT_TYPEID::NEUTRAL_MINERALFIELD));
  // if (FindNearestUnit(drone.pos, mineralFields, &mineralFieldTag))
  //   action->UnitCommand(drone.tag, ABILITY_ID::SMART, mineralFieldTag);
  uint64_t hiveTag;
  if (FindNearestUnit(drone.pos, GetHives(observation), &hiveTag)) {
    Tag mineralFieldTag;
    if (FindNearestUnit(drone.pos, ZergBot::GetMineralFields(observation),
                        &mineralFieldTag))
      action->UnitCommand(drone.tag, ABILITY_ID::SMART, mineralFieldTag);
  }
}

void ZergBot::ManageDrones::MineVespeneGas(const Unit &drone) {
  // Tag vespeneGeuserTag;
  // const Units mineralFields = observation->GetUnits(
  //         Unit::Alliance::Neutral, IsUnit(UNIT_TYPEID::ZERG_VESPENE??));
  // if (FindNearestUnit(drone.pos, mineralFields, &mineralFieldTag))
  //   action->UnitCommand(drone.tag, ABILITY_ID::SMART, mineralFieldTag);
}

void ZergBot::ManageSupply::OnPeriod() {
  if (observation->GetFoodCap() >= 200) return;
  const ssize_t supplyBuffer = 5;
  ssize_t supplyDeficit = observation->GetFoodUsed()
                        - (observation->GetFoodCap() + pendingSupply);
  if (supplyDeficit > -supplyBuffer) {
    // TODO: It is wasteful to call `GetUnits` every time.
    Units larvae = observation->GetUnits(Unit::Alliance::Self,
                                         IsUnit(UNIT_TYPEID::ZERG_LARVA));
    if (not larvae.empty() && IsUnitAbilityAvailable(
                    query, larvae[0].tag, ABILITY_ID::TRAIN_OVERLORD)) {
      action->UnitCommand(larvae[0].tag, ABILITY_ID::TRAIN_OVERLORD);
      pendingSupply += OverlordSupply;
      // supplyDeficit -= OverlordSupply;
    }
  }
}

void ZergBot::ManageSupply::OnUnitCreated(const Unit &unit) {
  // TODO: Move overlord to area to reduce fog.
  if (unit.unit_type.ToType() == UNIT_TYPEID::ZERG_OVERLORD)
    pendingSupply = (pendingSupply <= OverlordSupply)
                      ? 0 : pendingSupply -= OverlordSupply;
}

void ZergBot::ManageSupply::OnUnitDestroyed(const Unit &unit) {
  // TODO: Check for larva that would have been an overlord.
}
