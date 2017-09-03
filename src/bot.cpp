#include "bot.h"

using namespace std;
using namespace sc2;

void Bot::OnGameStart() {
  const ObservationInterface * observation = Observation();
  Units units = observation->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_HATCHERY));
  foundationBuildings.push_back(units[0]);
  vector<Point3D> locs = search::CalculateExpansionLocations(observation, Query());
  for (auto& loc : locs) {
    _expansions.push_back(Point2D(loc.x, loc.y));
  }
}

void Bot::OnStep() {
  WorkEconomy();
}

void Bot::OnUnitCreated(const Unit& unit) {
  const ObservationInterface * observation = Observation();
  switch (unit.unit_type.ToType()) {
    case UNIT_TYPEID::ZERG_HATCHERY: {
      foundationBuildings.push_back(unit);
      break;
    }
    case UNIT_TYPEID::ZERG_LAIR: {
      foundationBuildings.push_back(unit);
      break;
    }
    case UNIT_TYPEID::ZERG_HIVE: {
      foundationBuildings.push_back(unit);
      break;
    }
    case UNIT_TYPEID::ZERG_DRONE: {
      uint64_t target;
      if (FindNearestUnit(unit.pos, foundationBuildings, &target)) {
        const Unit * foundationBuilding = observation->GetUnit(target);
        if (FindNearestUnit(foundationBuilding->pos, observation->GetUnits(Unit::Alliance::Neutral, IsUnit(UNIT_TYPEID::NEUTRAL_MINERALFIELD)), &target)) {
          const Unit * mineralField = observation->GetUnit(target);
          Actions()->UnitCommand(unit.tag, ABILITY_ID::SMART, mineralField->tag);
          _workers.push_back(unit);
        }
      }
    }
    default: {
      break;
    }
  }
}

void Bot::OnUnitDestroyed(const Unit & unit) {
  for (Units::iterator f = foundationBuildings.begin(); f != foundationBuildings.end(); f++) {
    if (*f == unit) {
      foundationBuildings.erase(f);
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
  const ObservationInterface * observation = Observation();
  return observation->GetFoodUsed() >= observation->GetFoodCap();
}

bool Bot::NeedsMineralHarvesters(const Unit * foundationBuilding) {
  return foundationBuilding->assigned_harvesters < foundationBuilding->ideal_harvesters;
}

bool Bot::NeedsQueen(const Unit& foundationBuilding) {
  return _queens.size() < foundationBuildings.size();
}

void Bot::TryExpand() {
  if (_workers.size() > 0) {
    Unit drone = _workers.back();
    Point2D expansionPoint;
    QueryInterface * query = Query();
    if (FindNearestPoint2D(drone.pos, _expansions, &expansionPoint, [query](const Point2D& p){return query->Placement(ABILITY_ID::BUILD_HATCHERY, p);})) {
      Actions()->UnitCommand(drone.tag, ABILITY_ID::BUILD_HATCHERY, expansionPoint);
      _workers.pop_back();
    }
  }
}

void Bot::WorkEconomy() {
  const ObservationInterface * observation = Observation();
  Units larvas = observation->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_LARVA));
  for (auto & larva : larvas) {
    if (NeedSupply()) {
      Actions()->UnitCommand(larva.tag, ABILITY_ID::TRAIN_OVERLORD);
      continue;
    }
    uint64_t target;
    if (FindNearestUnit(larva.pos, foundationBuildings, &target)) {
      const Unit* foundationBuilding = observation->GetUnit(target);
      if (NeedsMineralHarvesters(foundationBuilding)) {
        Actions()->UnitCommand(larva.tag, ABILITY_ID::TRAIN_DRONE);
        continue;
      }
    }
  }
  for (auto & foundationBuilding : foundationBuildings) {
    if (NeedsQueen(foundationBuilding)) {
      Actions()->UnitCommand(foundationBuilding.tag, ABILITY_ID::TRAIN_QUEEN);
    }
  }
  TryExpand();
}
