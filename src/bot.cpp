#include "bot.h"

#include <iostream>

using namespace std;
using namespace sc2;

void Bot::OnGameStart() {
  const ObservationInterface * observation = Observation();
  vector<Point3D> locs = search::CalculateExpansionLocations(observation, Query());
  for (auto& loc : locs) {
    _expansions.push_back(Point2D(loc.x, loc.y));
  }
}

void Bot::OnStep() {
  // cout << "Num foundation buildings: " << foundationBuildings.size() << endl;
  WorkEconomy();
}

void Bot::OnUnitCreated(const Unit& unit) {
  const ObservationInterface * observation = Observation();
  ActionInterface * actions = Actions();
  switch (unit.unit_type.ToType()) {
    case UNIT_TYPEID::ZERG_HIVE:
    case UNIT_TYPEID::ZERG_LAIR:
    case UNIT_TYPEID::ZERG_HATCHERY: {
      _foundationBuildings.push_back(FoundationBuilding(unit.tag));
      break;
    }
    case UNIT_TYPEID::ZERG_DRONE: {
      _workerTags.push_back(unit.tag);
      const Units mineralFields = observation->GetUnits(Unit::Alliance::Neutral, IsUnit(UNIT_TYPEID::NEUTRAL_MINERALFIELD));
      for (auto& b : _foundationBuildings) {
        if (b.TryAssignWorker(actions, observation, mineralFields, unit)) {
          break;
        }
      }
      break;
    }
    case UNIT_TYPEID::ZERG_QUEEN: {
      // uint64_t target;
      // if (FindNearestUnit(unit.pos, foundationBuildings.units, &target)) {
      //   for (auto& b : foundationBuildings) {
      //     if (b.unit.tag == target && b.OnQueenCreated(Actions(), unit)) {
      //       break;
      //     }
      //   }
      // }
      break;
    }
    case UNIT_TYPEID::ZERG_OVERLORD: {
      if (_pendingSupply >= 10) {
        _pendingSupply -= 10;
      }
    }
    default: {
      break;
    }
  }
}

void Bot::OnUnitDestroyed(const Unit & unit) {
  switch (unit.unit_type.ToType()) {
    case UNIT_TYPEID::ZERG_HIVE:
    case UNIT_TYPEID::ZERG_LAIR:
    case UNIT_TYPEID::ZERG_HATCHERY: {
      for (vector<FoundationBuilding>::iterator f = _foundationBuildings.begin(); f != _foundationBuildings.end(); f++) {
        if ((*f).buildingTag == unit.tag) {
          // TODO: Clean up orphaned workers and queens.
          _foundationBuildings.erase(f);
          break;
        }
      }
      break;
    }
    case UNIT_TYPEID::ZERG_DRONE: {
      for (vector<Tag>::iterator t = _workerTags.begin(); t != _workerTags.end(); t++) {
        if (*t == unit.tag) {
          _workerTags.erase(t);
        }
      }
      break;
    }
    case UNIT_TYPEID::ZERG_QUEEN: {
      for (auto& b : _foundationBuildings) {
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

void Bot::OnUnitIdle(const Unit& unit) {
  switch (unit.unit_type.ToType()) {
    default: {
      break;
    }
  }
}

bool Bot::NeedSupply() {
  const ObservationInterface* observation = Observation();
  return observation->GetFoodUsed() >= observation->GetFoodCap() + _pendingSupply;
}

void Bot::TryGetSupply() {
  const ObservationInterface* observation = Observation();
  Units larvas = observation->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_LARVA));
  if (larvas.size() > 0) {
    if (IsUnitAbilityAvailable(Query(), larvas[0].tag, ABILITY_ID::TRAIN_OVERLORD)) {
      Actions()->UnitCommand(larvas[0].tag, ABILITY_ID::TRAIN_OVERLORD);
      _pendingSupply += 10;
    }
  }
}

void Bot::TryBuildStructure(ABILITY_ID buildStructureAbility) {
  const ObservationInterface* observation = Observation();
  if (_workerTags.size() > 0) {
    const Unit* drone = observation->GetUnit(_workerTags.back());
    QueryInterface* query = Query();
    Point2D targetPoint;
    float radius = 10;
    if (FindRandomPoint(drone->pos, &targetPoint, radius, [query, buildStructureAbility](const Point2D& p){return query->Placement(buildStructureAbility, p);})) {
      if (IsUnitAbilityAvailable(Query(), drone->tag, buildStructureAbility)) {
        Actions()->UnitCommand(drone->tag, buildStructureAbility, targetPoint);
        _workerTags.pop_back();
      }
    }
  }
}

void Bot::TryExpand() {
  const ObservationInterface* observation = Observation();
  if (_workerTags.size() > 0) {
    const Unit* drone = observation->GetUnit(_workerTags.back());
    Point2D expansionPoint;
    QueryInterface* query = Query();
    if (FindNearestPoint2D(drone->pos, _expansions, &expansionPoint, [query](const Point2D& p){return query->Placement(ABILITY_ID::BUILD_HATCHERY, p);})) {
      if (IsUnitAbilityAvailable(query, drone->tag, ABILITY_ID::BUILD_HATCHERY)) {
        Actions()->UnitCommand(drone->tag, ABILITY_ID::BUILD_HATCHERY, expansionPoint);
        _workerTags.pop_back();
      }
    }
  }
}

void Bot::WorkEconomy() {
  ActionInterface* actions = Actions();
  const ObservationInterface* observation = Observation();
  QueryInterface* query = Query();
  if (NeedSupply()) {
    TryGetSupply();
  }
  for (auto& f : _foundationBuildings) {
    f.TryTrainQueen(actions, observation, query);
  }
  for (auto& f : _foundationBuildings) {
    f.TryTrainWorker(actions, observation, query);
  }
  if (observation->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_SPAWNINGPOOL)).empty()) {
    TryBuildStructure(ABILITY_ID::BUILD_SPAWNINGPOOL);
  }
  TryExpand();
}
