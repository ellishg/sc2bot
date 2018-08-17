#include "ZergBot.h"

using namespace sc2;

void ZergBot::ManageQueens::OnPeriod() {
  TrainQueens();
  for (Queen queen : queens) {
    // TODO
    // SpawnCreep(queen);
    InjectLarva(queen);
  }
  // TODO: Check if attack is needed.
}

void ZergBot::ManageQueens::OnUnitCreated(const Unit &unit) {
  if (IsQueen(unit)) {
    // auto hiveFilter = [](const Unit &unit) {
    //   return IsUnit(UNIT_TYPEID::ZERG_HIVE)(unit)
    //       || IsUnit(UNIT_TYPEID::ZERG_LAIR)(unit)
    //       || IsUnit(UNIT_TYPEID::ZERG_HATCHERY)(unit);
    // };
    // auto hives = observation->GetUnits(Unit::Alliance::Self, hiveFilter);
    auto hives = ZergBot::GetHives(observation);
    Tag hiveTag;
    if (FindNearestUnit(unit.pos, hives, &hiveTag))
      queens.push_back({unit.tag, hiveTag});
  }
}

void ZergBot::ManageQueens::OnUnitDestroyed(const Unit &unit) {
  if (IsQueen(unit)) {
    for (auto queen = queens.begin(); queen != queens.end(); queen++) {
      if (queen->tag == unit.tag) {
        queens.erase(queen);
        break;
      }
    }
  }
}

void ZergBot::ManageQueens::TrainQueens() {
  auto queryCopy = query;
  auto hiveFilter = [queryCopy](const Unit &unit) {
   return IsUnitAbilityAvailable(queryCopy, unit.tag,
                                 ABILITY_ID::TRAIN_QUEEN);
  };
  auto hives = observation->GetUnits(Unit::Alliance::Self, hiveFilter);
  for (auto hive : hives) {
    // TODO: Check if hive has pending queen.
    bool found = false;
    for (auto queen : queens) {
      if (queen.hiveTag == hive.tag) {
        found = true;
        break;
      }
    }
    if (!found)
      action->UnitCommand(hive.tag, ABILITY_ID::TRAIN_QUEEN);
  }
}

void ZergBot::ManageQueens::SpawnCreep(Queen queen) {
  // TODO
}

void ZergBot::ManageQueens::InjectLarva(Queen queen) {
  if (IsUnitAbilityAvailable(query, queen.tag, ABILITY_ID::EFFECT_INJECTLARVA))
    action->UnitCommand(queen.tag, ABILITY_ID::EFFECT_INJECTLARVA,
                        queen.hiveTag);
}

void ZergBot::ManageQueens::Attack(Queen queen) {
  // TODO
}
