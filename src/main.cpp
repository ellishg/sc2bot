#include <sc2lib/sc2_lib.h>
#include <sc2api/sc2_api.h>

#include <iostream>

using namespace sc2;
using namespace std;

class Bot : public Agent {

private:

  Units foundationBuildings;
  vector<Point2D> _expansions;
  Units _workers;
  Units _queens;

public:

  virtual void OnGameStart() final {
    const ObservationInterface * observation = Observation();
    Units units = observation->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_HATCHERY));
    foundationBuildings.push_back(units[0]);
    vector<Point3D> locs = search::CalculateExpansionLocations(observation, Query());
    for (auto& loc : locs) {
      _expansions.push_back(Point2D(loc.x, loc.y));
    }
  }

  virtual void OnStep() final {
    WorkEconomy();
  }

  virtual void OnUnitCreated(const Unit& unit) {
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
        if (FindNearest(unit.pos, foundationBuildings, &target)) {
          const Unit * foundationBuilding = observation->GetUnit(target);
          if (FindNearest(foundationBuilding->pos, observation->GetUnits(Unit::Alliance::Neutral, IsUnit(UNIT_TYPEID::NEUTRAL_MINERALFIELD)), &target)) {
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

  virtual void OnUnitDestroyed(const Unit & unit) {
    for (Units::iterator f = foundationBuildings.begin(); f != foundationBuildings.end(); f++) {
      if (*f == unit) {
        foundationBuildings.erase(f);
        break;
      }
    }
  }

  virtual void OnUnitIdle(const Unit& unit) final {
    switch (unit.unit_type.ToType()) {
      default: {
        break;
      }
    }
  }

  bool FindNearest(const Point2D& start, const Units& units, uint64_t * target) {

    if (units.size() == 0) {
      return false;
    }
    float distance = std::numeric_limits<float>::max();
    for (const auto& u : units) {
      float d = DistanceSquared2D(u.pos, start);
      if (d < distance) {
        distance = d;
        *target = u.tag;
      }
    }
    if (distance == std::numeric_limits<float>::max()) {
      return false;
    }
    return true;
  }

  bool NeedSupply() {
    const ObservationInterface * observation = Observation();
    return observation->GetFoodUsed() >= observation->GetFoodCap();
  }

  bool NeedsMineralHarvesters(const Unit * foundationBuilding) {
    return foundationBuilding->assigned_harvesters < foundationBuilding->ideal_harvesters;
  }

  bool NeedsQueen(const Unit& foundationBuilding) {
    return _queens.size() < foundationBuildings.size();
  }

  bool ClosestPoint(const Point2D& point, const vector<Point2D>& candidates, Point2D * target, function<bool(const Point2D&)> filter = function<bool(const Point2D&)>([](const Point2D&){return true;})) {
    float bestDistanceSqrd = numeric_limits<float>::max();
    for (auto & c : candidates) {
      float distSqrd = Distance2D(point, c);
      if (filter(c) && distSqrd < bestDistanceSqrd) {
        bestDistanceSqrd = distSqrd;
        *target = c;
      }
    }
    if (bestDistanceSqrd == numeric_limits<float>::max()) {
      return false;
    }
    return true;
  }

  void TryExpand() {
    if (_workers.size() > 0) {
      Unit drone = _workers.back();
      Point2D expansionPoint;
      QueryInterface * query = Query();
      if (ClosestPoint(drone.pos, _expansions, &expansionPoint, [query](const Point2D& p){return query->Placement(ABILITY_ID::BUILD_HATCHERY, p);})) {
        Actions()->UnitCommand(drone.tag, ABILITY_ID::BUILD_HATCHERY, expansionPoint);
        _workers.pop_back();
      }
    }
  }

  void WorkEconomy() {
    const ObservationInterface * observation = Observation();
    Units larvas = observation->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_LARVA));
    for (auto & larva : larvas) {
      if (NeedSupply()) {
        Actions()->UnitCommand(larva.tag, ABILITY_ID::TRAIN_OVERLORD);
        continue;
      }
      uint64_t target;
      if (FindNearest(larva.pos, foundationBuildings, &target)) {
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

};

int main(int argc, char* argv[]) {
  Coordinator coordinator;
  coordinator.LoadSettings(argc, argv);
  //  coordinator.SetRealtime(true);

  Bot bot;
  coordinator.SetParticipants({
    CreateParticipant(Race::Zerg, &bot),
    CreateComputer(Race::Terran)
  });

  coordinator.LaunchStarcraft();
  coordinator.StartGame(sc2::kMapBelShirVestigeLE);

  while (coordinator.Update()) {
  }

  return 0;
}
