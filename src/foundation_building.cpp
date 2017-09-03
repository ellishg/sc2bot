#include "foundation_building.h"

using namespace std;
using namespace sc2;

bool FoundationBuilding::NeedsMineralHarvesters(const ObservationInterface* observation) {
  const Unit* building = observation->GetUnit(buildingTag);
  return building->assigned_harvesters + numPendingWorkers < building->ideal_harvesters;
}

bool FoundationBuilding::NeedsQueen() {
  return queenTags.size() + numPendingQueens < 1;
}

void FoundationBuilding::TryTrainWorker(ActionInterface* actions, const ObservationInterface* observation, QueryInterface* query) {
  const int32_t requiredMinerals = 50;
  const int32_t requiredVespene = 0;
  const int32_t requiredFood = 1;
  if (NeedsMineralHarvesters(observation)) {
    const Unit* unit = observation->GetUnit(buildingTag);
    Units larvas = observation->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_LARVA));
    uint64_t larvaTag;
    if (FindNearestUnit(unit->pos, larvas, &larvaTag)) {
      if (IsUnitAbilityAvailable(query, larvaTag, ABILITY_ID::TRAIN_DRONE)) {
        actions->UnitCommand(larvaTag, ABILITY_ID::TRAIN_DRONE);
        numPendingWorkers++;
      }
    }
  }
}

bool FoundationBuilding::TryAssignWorker(ActionInterface* actions, const ObservationInterface* observation, const Units& mineralFields, const Unit& worker) {
  if (numPendingWorkers > 0) {
    const Unit* unit = observation->GetUnit(buildingTag);
    uint64_t mineralFieldTag;
    if (FindNearestUnit(unit->pos, mineralFields, &mineralFieldTag)) {
      actions->UnitCommand(worker.tag, ABILITY_ID::SMART, mineralFieldTag);
      numPendingWorkers--;
      return true;
    }
  }
  return false;
}

bool FoundationBuilding::OnQueenCreated(ActionInterface* actions, Tag queenTag) {
  if (numPendingQueens > 0) {
    queenTags.push_back(queenTag);
    numPendingQueens--;
    return true;
  }
  return false;
}

bool FoundationBuilding::OnQueenDestroyed(Tag queenTag) {
  for (vector<Tag>::iterator q = queenTags.begin(); q != queenTags.end(); q++) {
    if (*q == queenTag) {
      queenTags.erase(q);
      return true;
    }
  }
  return false;
}

void FoundationBuilding::TryTrainQueen(ActionInterface* actions, const ObservationInterface* observation, QueryInterface* query) {
  const int32_t requiredMinerals = 150;
  const int32_t requiredVespene = 0;
  const int32_t requiredFood = 2;
  if (NeedsQueen() && IsUnitAbilityAvailable(query, buildingTag, ABILITY_ID::TRAIN_QUEEN)) {
    cout << "training queen" << endl;
    actions->UnitCommand(buildingTag, ABILITY_ID::TRAIN_QUEEN);
    numPendingQueens++;
  }
}

void FoundationBuilding::TryExpand() {

}
