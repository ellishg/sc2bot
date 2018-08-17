#include "ZergBot.h"

using namespace sc2;
using namespace std;

void ZergBot::ManageStructures::OnPeriod() {
  // TODO: Check for pending buildings.
  const vector<Structure> cruicialStructures = {
    Structure(
      ABILITY_ID::BUILD_SPAWNINGPOOL,
      /*BuildCondition*/
      [](const ObservationInterface *observation) {
        const uint32_t requiredMinerals = 200;
        return observation->GetMinerals() >= requiredMinerals &&
               observation->GetUnits(Unit::Alliance::Self,
                    IsUnit(UNIT_TYPEID::ZERG_SPAWNINGPOOL)).empty();
      },
      /*GetTargetLocation*/
      [](const ObservationInterface *observation, QueryInterface *query) {
        return observation->GetStartLocation();
      },
      /*Tolerance*/ 10
    ),
    Structure(
      ABILITY_ID::BUILD_EXTRACTOR,
      /*BuildCondition*/
      [](const ObservationInterface *observation) {
        const uint32_t requiredMinerals = 50;
        bool hasSpawningPool = not observation->GetUnits(Unit::Alliance::Self,
                    IsUnit(UNIT_TYPEID::ZERG_SPAWNINGPOOL)).empty();
        size_t numHives = ZergBot::GetHives(observation).size();
        size_t numExtractors = observation->GetUnits(Unit::Alliance::Self,
                     IsUnit(UNIT_TYPEID::ZERG_EXTRACTOR)).size();
        return observation->GetMinerals() >= requiredMinerals &&
               hasSpawningPool && numExtractors < 2 * numHives;
      },
      /*GetTargetTag*/
      [](const ObservationInterface *observation, QueryInterface *query) {
        // TODO: Check how far Vespene Geysers can be from hive.
        const float range = 15.f;
        // Find the nearest Vespene Geyser without an Extractor near some hive
        for (auto hive : ZergBot::GetHives(observation)) {
          auto nearbyExtractorFilter = [hive, range](const Unit &unit) {
            return IsUnit(UNIT_TYPEID::ZERG_EXTRACTOR)(unit) &&
                   DistanceSquared2D(hive.pos, unit.pos) < range*range;
          };
          Units nearbyExtractors = observation->GetUnits(Unit::Alliance::Self,
                                                         nearbyExtractorFilter);
          if (nearbyExtractors.size() < 2) {
            auto placeableTargets = [query, hive, range](const Unit &unit) {
              return IsUnit(UNIT_TYPEID::NEUTRAL_VESPENEGEYSER)(unit) &&
                     DistanceSquared2D(hive.pos, unit.pos) < range*range &&
                     query->Placement(ABILITY_ID::BUILD_EXTRACTOR, unit.pos);
            };
            auto vespeneGeysers = observation->GetUnits(Unit::Alliance::Neutral,
                                                        placeableTargets);
            if (not vespeneGeysers.empty())
              return vespeneGeysers[0].tag;
          }
        }
        assert(false && "Could not find valid placement for Extractor");
        return Tag(0);
      }
    ),
    Structure(
      ABILITY_ID::BUILD_ROACHWARREN,
      /*BuildCondition*/
      [](const ObservationInterface *observation) {
        const uint32_t requiredMinerals = 150;
        return observation->GetMinerals() >= requiredMinerals &&
               observation->GetUnits(Unit::Alliance::Self,
                    IsUnit(UNIT_TYPEID::ZERG_ROACHWARREN)).empty();
      },
      /*GetTargetLocation*/
      [](const ObservationInterface *observation, QueryInterface *query) {
        return observation->GetStartLocation();
      },
      /*Tolerance*/ 10
    )
  };

  for (auto structure : cruicialStructures) {
    if (structure.BuildCondition(observation)) {
      if (structure.GetTargetLocation) {
        TryBuildStructureNear(structure.ability,
                              structure.GetTargetLocation(observation, query),
                              structure.tolerance);
      } else if (structure.GetTargetTag) {
        Tag targetTag = structure.GetTargetTag(observation, query);
        Point2D target;
        if (auto targetUnit = observation->GetUnit(targetTag))
          target = targetUnit->pos;
        uint64_t droneTag;
        if (FindNearestUnit(target, GetFreeDrones(), &droneTag)) {
          action->UnitCommand(droneTag, structure.ability, targetTag);
        }
      } else
        assert(false &&
               "Structure must have GetTargetLocation or GetTargetTag");
    }
  }

}

void ZergBot::ManageStructures::OnUnitDestroyed(const Unit &unit) {
  // TODO: If an important building was destroyed, rebuild immediatly
  // If not too important, leave to be automatically built.
}


bool ZergBot::ManageStructures::TryBuildStructureNear(ABILITY_ID structure,
                                                      const Point2D target,
                                                      float tolerance) {
  uint64_t droneTag;
  if (FindNearestUnit(target, GetFreeDrones(), &droneTag)) {
    auto queryCopy = query;
    auto canBuildStructure =
      [queryCopy, structure, target](const Point2D &target) {
        return queryCopy->Placement(structure, target);
      };
    Point2D buildLoc;
    if (FindRandomPoint(target, &buildLoc, tolerance, canBuildStructure)) {
      action->UnitCommand(droneTag, structure, buildLoc);
      return true;
    } else
      cout << "Unable to build structure near " << target.x << ", " << target.y << endl;
  } else
    cout << "Unable to find drone" << endl;
  return false;
}

Units ZergBot::ManageStructures::GetFreeDrones() {
  auto freeDroneFilter = [](const Unit &unit) {
    if (IsUnit(UNIT_TYPEID::ZERG_DRONE)(unit)) {
      for (auto order : unit.orders) {
        if (not (order.ability_id == ABILITY_ID::INVALID ||
                 order.ability_id == ABILITY_ID::SMART ||
                 order.ability_id == ABILITY_ID::HARVEST_GATHER ||
                 order.ability_id == ABILITY_ID::HARVEST_GATHER_DRONE ||
                 order.ability_id == ABILITY_ID::HARVEST_RETURN ||
                 order.ability_id == ABILITY_ID::HARVEST_RETURN_DRONE ||
                 order.ability_id == ABILITY_ID::HALT))
          return false;
      }
      return true;
    }
    return false;
  };
  return observation->GetUnits(Unit::Alliance::Self, freeDroneFilter);
}
