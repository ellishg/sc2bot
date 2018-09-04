#include "ZergBot.h"

using namespace std;
using namespace sc2;

Suggestions ZergBot::ManageDrones::OnPeriod() {
  Suggestions suggestions;
  for (auto hive : GetHives(observation)) {
    // FIXME: A drone is not actually assigned until it touches the first
    // minerals, so we usually get 18/16 drones for each hive. I could manually
    // keep track of how many drones a hive has. Or I could wait to decrement
    // pending drones until they touch the minerals.
    ssize_t harvesterDeficit = hive.ideal_harvesters
        - (hive.assigned_harvesters + pendingDronesForHive[hive.tag]);
    if (harvesterDeficit > 0) {
      // IDEA: Make this a while loop, the first TRAIN_DRONE gets high priority,
      //       the rest for the hive have low priority.
      Units larvae = observation->GetUnits(Unit::Alliance::Self,
                                           IsUnit(UNIT_TYPEID::ZERG_LARVA));
      if (larvae.empty() || not IsUnitAbilityAvailable(
                     query, larvae[0].tag, ABILITY_ID::TRAIN_DRONE))
        break;
      // TODO: Pick closest larva.
      // TODO: Set dest to mineral field.
      suggestions.emplace_back(larvae[0].tag, ABILITY_ID::TRAIN_DRONE,
                               SuggestedAction::PRIORITY_LEVEL::HIGH,
                               [this, hive]{
                                 pendingDronesForHive[hive.tag]++;
                               });
    }
  }
  return suggestions;
}

Suggestions ZergBot::ManageDrones::OnUnitCreated(const Unit &unit) {
  if (IsDrone(unit)) {
    uint64_t hiveTag;
    if (FindNearestUnit(unit.pos, GetHives(observation), &hiveTag)) {
      pendingDronesForHive[hiveTag] = (pendingDronesForHive[hiveTag] == 0)
            ? 0 : pendingDronesForHive[hiveTag] - 1;
    }
    return AssignWork(unit);
  }
  switch (unit.unit_type.ToType()) {
    case UNIT_TYPEID::ZERG_HIVE:
    case UNIT_TYPEID::ZERG_LAIR:
    case UNIT_TYPEID::ZERG_HATCHERY:
      pendingDronesForHive[unit.tag] = 0;
    default:
      break;
  }
  return {};
}

Suggestions ZergBot::ManageDrones::OnUnitDestroyed(const Unit &unit) {
  // IDEA: Check for larva that would have been a drone.
  return {};
}

Suggestions ZergBot::ManageDrones::OnUnitIdle(const Unit &unit) {
  if (IsDrone(unit))
    return AssignWork(unit);
  return {};
}

Suggestions ZergBot::ManageDrones::AssignWork(const Unit &drone) {
  // TODO: Check for pending actions
  return MineMinerals(drone);
  // MineVespeneGas(drone);
}

Suggestions ZergBot::ManageDrones::MineMinerals(const Unit &drone) {
  Tag mineralFieldTag;
  if (FindNearestUnit(drone.pos, ZergBot::GetMineralFields(observation),
                      &mineralFieldTag))
    return {SuggestedAction(drone.tag, ABILITY_ID::SMART, mineralFieldTag)};
  return {};
}

Suggestions ZergBot::ManageDrones::MineVespeneGas(const Unit &drone) {
  // TODO
  return {};
}

Suggestions ZergBot::ManageSupply::OnPeriod() {
  if (observation->GetFoodCap() >= 200) return {};
  const ssize_t supplyBuffer = 5;
  ssize_t supplyDeficit = observation->GetFoodUsed()
                        - (observation->GetFoodCap() + pendingSupply);
  if (supplyDeficit > -supplyBuffer) {
    Units larvae = observation->GetUnits(Unit::Alliance::Self,
                                         IsUnit(UNIT_TYPEID::ZERG_LARVA));
    if (not larvae.empty() && IsUnitAbilityAvailable(
                    query, larvae[0].tag, ABILITY_ID::TRAIN_OVERLORD)) {
      // IDEA: Very priority based on supply deficit.
      return {SuggestedAction(larvae[0].tag, ABILITY_ID::TRAIN_OVERLORD,
                              SuggestedAction::PRIORITY_LEVEL::HIGH,
                              [this]{ pendingSupply += OverlordSupply; })};
    }
  }
  return {};
}

Suggestions ZergBot::ManageSupply::OnUnitCreated(const Unit &unit) {
  // IDEA: Move overlord to area to reduce fog.
  if (unit.unit_type.ToType() == UNIT_TYPEID::ZERG_OVERLORD)
    pendingSupply = (pendingSupply <= OverlordSupply)
                      ? 0 : pendingSupply -= OverlordSupply;
  return {};
}

Suggestions ZergBot::ManageSupply::OnUnitDestroyed(const Unit &unit) {
  // IDEA: Check for larva that would have been an overlord.
  return {};
}
