#include "ZergBot.h"

using namespace sc2;

Suggestions ZergBot::ManageQueens::OnPeriod() {
  Suggestions suggestions;
  VECTOR_COMBINE(suggestions, TrainQueens());
  for (Queen queen : queens) {
    // TODO
    // SpawnCreep(queen);
    VECTOR_COMBINE(suggestions, InjectLarva(queen));
  }
  // TODO: Check if attack is needed.
  return suggestions;
}

Suggestions ZergBot::ManageQueens::OnUnitCreated(const Unit &unit) {
  if (IsQueen(unit)) {
    auto hives = ZergBot::GetHives(observation);
    Tag hiveTag;
    if (FindNearestUnit(unit.pos, hives, &hiveTag))
      queens.push_back({unit.tag, hiveTag});
  }
  return {};
}

Suggestions ZergBot::ManageQueens::OnUnitDestroyed(const Unit &unit) {
  if (IsQueen(unit)) {
    for (auto queen = queens.begin(); queen != queens.end(); queen++) {
      if (queen->tag == unit.tag) {
        queens.erase(queen);
        break;
      }
    }
  }
  return {};
}

Suggestions ZergBot::ManageQueens::TrainQueens() {
  Suggestions suggestions;
  auto hiveFilter = [this](const Unit &unit) {
   return IsUnitAbilityAvailable(query, unit.tag, ABILITY_ID::TRAIN_QUEEN);
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
      suggestions.emplace_back(hive.tag, ABILITY_ID::TRAIN_QUEEN);
  }
  return suggestions;
}

Suggestions ZergBot::ManageQueens::SpawnCreep(Queen queen) {
  // TODO
  return {};
}

Suggestions ZergBot::ManageQueens::InjectLarva(Queen queen) {
  if (IsUnitAbilityAvailable(query, queen.tag, ABILITY_ID::EFFECT_INJECTLARVA))
    return {SuggestedAction(queen.tag, ABILITY_ID::EFFECT_INJECTLARVA,
                            queen.hiveTag)};
  return {};
}

Suggestions ZergBot::ManageQueens::Attack(Queen queen) {
  // TODO
  return {};
}
