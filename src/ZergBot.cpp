#include "ZergBot.h"

#include <iostream>

using namespace std;
using namespace sc2;

void ZergBot::OnGameStart() {
  auto *obs = Observation();
  vector<Point3D> locs = search::CalculateExpansionLocations(obs, Query());
  for (auto& loc : locs) {
    expansions.push_back(Point2D(loc.x, loc.y));
  }
}

void ZergBot::OnStep() {
  // cout << "Num foundation buildings: " << foundationBuildings.size() << endl;
  WorkEconomy();
}

void ZergBot::OnUnitCreated(const Unit &unit) {
  const ObservationInterface * observation = Observation();
  ActionInterface * actions = Actions();
  // cout << "Created " << unit.unit_type.to_string() << endl;
  switch (unit.unit_type.ToType()) {
    case UNIT_TYPEID::ZERG_HIVE:
    case UNIT_TYPEID::ZERG_LAIR:
    case UNIT_TYPEID::ZERG_HATCHERY: {
      foundationBuildings.push_back(FoundationBuilding(unit.tag));
      break;
    }
    case UNIT_TYPEID::ZERG_DRONE: {
      workerTags.push_back(unit.tag);
      const Units mineralFields =
              observation->GetUnits(Unit::Alliance::Neutral,
                                    IsUnit(UNIT_TYPEID::NEUTRAL_MINERALFIELD));
      for (auto& b : foundationBuildings) {
        if (b.TryAssignWorker(actions, observation, mineralFields, unit)) {
          break;
        }
      }
      break;
    }
    case UNIT_TYPEID::ZERG_QUEEN: {
      for (auto& foundationBuilding : foundationBuildings) {
        if (foundationBuilding.OnQueenCreated(unit.tag)) {
          break;
        }
      }
      break;
    }
    case UNIT_TYPEID::ZERG_OVERLORD: {
      if (pendingSupply >= 10) {
        pendingSupply -= 10;
      }
      break;
    }
    default: {
      break;
    }
  }
}

void ZergBot::OnUnitDestroyed(const Unit &unit) {
  switch (unit.unit_type.ToType()) {
    case UNIT_TYPEID::ZERG_HIVE:
    case UNIT_TYPEID::ZERG_LAIR:
    case UNIT_TYPEID::ZERG_HATCHERY: {
      auto f = foundationBuildings.begin();
      while (f != foundationBuildings.end()) {
        if ((*f).buildingTag == unit.tag) {
          // TODO: Clean up orphaned workers and queens.
          foundationBuildings.erase(f);
          break;
        }
        f++;
      }
      break;
    }
    case UNIT_TYPEID::ZERG_DRONE: {
      auto t = workerTags.begin();
      while (t != workerTags.end()) {
        if (*t == unit.tag) {
          workerTags.erase(t);
        }
        t++;
      }
      break;
    }
    case UNIT_TYPEID::ZERG_QUEEN: {
      for (auto& b : foundationBuildings) {
        if (b.OnQueenDestroyed(unit.tag)) {
          break;
        }
      }
      break;
    }
    default: {
      break;
    }
  }
}

void ZergBot::OnUnitIdle(const Unit &unit) {
  switch (unit.unit_type.ToType()) {
    default: {
      break;
    }
  }
}

bool ZergBot::NeedSupply() {
  auto *obs = Observation();
  return obs->GetFoodUsed() >= obs->GetFoodCap() + pendingSupply;
}

void ZergBot::TryGetSupply() {
  auto *obs = Observation();
  Units larvae = obs->GetUnits(Unit::Alliance::Self,
                               IsUnit(UNIT_TYPEID::ZERG_LARVA));
  if (larvae.size() > 0) {
    if (IsUnitAbilityAvailable(Query(), larvae[0].tag,
                               ABILITY_ID::TRAIN_OVERLORD)) {
      Actions()->UnitCommand(larvae[0].tag, ABILITY_ID::TRAIN_OVERLORD);
      pendingSupply += 10;
    }
  }
}

void ZergBot::TryBuildStructure(ABILITY_ID buildStructureAbility) {
  auto *obs = Observation();
  if (workerTags.size() > 0) {
    const Unit* drone = obs->GetUnit(workerTags.back());
    QueryInterface* query = Query();
    Point2D targetPoint;
    float radius = 10;
    auto filter = [query, buildStructureAbility](const Point2D& p) {
      return query->Placement(buildStructureAbility, p);
    };
    if (FindRandomPoint(drone->pos, &targetPoint, radius, filter)) {
      if (IsUnitAbilityAvailable(Query(), drone->tag, buildStructureAbility)) {
        Actions()->UnitCommand(drone->tag, buildStructureAbility, targetPoint);
        workerTags.pop_back();
      }
    }
  }
}

void ZergBot::TryExpand() {
  auto *obs = Observation();
  if (workerTags.size() > 0) {
    const Unit* drone = obs->GetUnit(workerTags.back());
    Point2D expansionPoint;
    QueryInterface* query = Query();
    auto filter = [query](const Point2D& p) {
      return query->Placement(ABILITY_ID::BUILD_HATCHERY, p);
    };
    if (FindNearestPoint2D(drone->pos, expansions, &expansionPoint, filter)) {
      if (IsUnitAbilityAvailable(query, drone->tag,
                                 ABILITY_ID::BUILD_HATCHERY)) {
        Actions()->UnitCommand(drone->tag, ABILITY_ID::BUILD_HATCHERY,
                               expansionPoint);
        workerTags.pop_back();
      }
    }
  }
}

void ZergBot::TryExpandCreep() {
  QueryInterface* query = Query();
  auto *obs = Observation();
  ActionInterface* actions = Actions();
  auto tumorFilter = [query](const Unit& unit) {
    bool isCreepTumor = IsUnit(UNIT_TYPEID::ZERG_CREEPTUMOR)(unit)
                     || IsUnit(UNIT_TYPEID::ZERG_CREEPTUMORBURROWED)(unit)
                     || IsUnit(UNIT_TYPEID::ZERG_CREEPTUMORQUEEN)(unit);
    return IsUnitAbilityAvailable(query, unit.tag,
                                  ABILITY_ID::BUILD_CREEPTUMOR) && isCreepTumor;
  };
  Units tumorUnits = obs->GetUnits(Unit::Alliance::Self, tumorFilter);
  for (Unit& tumorUnit : tumorUnits) {
    if (tumorUnit.orders.empty()) {
      Point2D target;
      auto creepFilter = [query](const Point2D& p) {
        return query->Placement(ABILITY_ID::BUILD_CREEPTUMOR_TUMOR, p);
      };
      if (FindRandomPoint(tumorUnit.pos, &target, 10, creepFilter)) {
        actions->UnitCommand(tumorUnit.tag, ABILITY_ID::BUILD_CREEPTUMOR,
                             target);
      }
    }
  }
}

void ZergBot::WorkEconomy() {
  auto *actions = Actions();
  auto *obs = Observation();
  QueryInterface* query = Query();
  if (NeedSupply()) {
    TryGetSupply();
  }
  for (auto& f : foundationBuildings) {
    f.TryTrainQueen(actions, query);
    f.TryBuildCreepTumor(actions, obs, query);
    // f.TryInjectLarva(actions, query);
  }
  for (auto& f : foundationBuildings) {
    f.TryTrainWorker(actions, obs, query);
  }
  if (obs->GetUnits(Unit::Alliance::Self,
                    IsUnit(UNIT_TYPEID::ZERG_SPAWNINGPOOL)).empty()) {
    TryBuildStructure(ABILITY_ID::BUILD_SPAWNINGPOOL);
  }
  TryExpand();
  TryExpandCreep();
}
