#pragma once

#include <sc2lib/sc2_lib.h>
#include <sc2api/sc2_api.h>

#include "BotBase.h"
#include "FoundationBuilding.h"
#include "common.h"

namespace sc2 {

class ZergBot : public BotBase {
  StrategyList strategies;

  StrategyList PopulateStrategies() {
    strategies.emplace_back(new ManageDrones(10));
    strategies.emplace_back(new ManageStructures(100));
    strategies.emplace_back(new ManageSupply(10));
    strategies.emplace_back(new ManageQueens(100));
    strategies.emplace_back(new ExpandCreep(100));
    return strategies;
  }

public:
  ZergBot() : BotBase(PopulateStrategies()) {}
  ~ZergBot() { strategies.clear(); }

  // TODO: Use `OnGame` methods to initialize and destroy strategies.

private:

  class ManageStructures : public StrategyBase {
    /*
     *  Responsible for producing all buildings.
     *  Is aware of resources
     */
    struct Structure {
      using BuildConditionDecl =
            std::function<bool(const sc2::ObservationInterface *)>;
      using GetTargetTagDecl =
            std::function<sc2::Tag(const sc2::ObservationInterface *,
                                   sc2::QueryInterface *query)>;
      using GetTargetLocDecl =
            std::function<sc2::Point2D(const sc2::ObservationInterface *,
                                       sc2::QueryInterface *query)>;
      sc2::ABILITY_ID ability;
      BuildConditionDecl BuildCondition;
      GetTargetLocDecl GetTargetLocation;
      GetTargetTagDecl GetTargetTag;
      float tolerance;
      Structure(sc2::ABILITY_ID _ability, BuildConditionDecl _BuildCond,
                GetTargetLocDecl _GetTargetLoc, float _tolerance = 3)
            : ability(_ability), BuildCondition(_BuildCond),
              GetTargetLocation(_GetTargetLoc), GetTargetTag(nullptr),
              tolerance(_tolerance) {}
      Structure(sc2::ABILITY_ID _ability, BuildConditionDecl _BuildCond,
                GetTargetTagDecl _GetTargetTag)
            : ability(_ability), BuildCondition(_BuildCond),
              GetTargetLocation(nullptr), GetTargetTag(_GetTargetTag),
              tolerance(0) {}
    };
  public:
    ManageStructures(size_t period) : StrategyBase(period) {}

    void OnPeriod();
    void OnUnitDestroyed(const Unit &unit);

  private:
    bool TryBuildStructureNear(sc2::ABILITY_ID structure,
                               const sc2::Point2D target, float tolerance = 3);
    sc2::Units GetFreeDrones();
  };

  class ManageSupply : public StrategyBase {
  public:
    ManageSupply(size_t period) : StrategyBase(period), pendingSupply(0) {}

    void OnPeriod();
    void OnUnitCreated(const Unit &unit);
    void OnUnitDestroyed(const Unit &unit);

  private:
    size_t pendingSupply;
    const size_t OverlordSupply = 10;
  };

  class ManageDrones : public StrategyBase {
    std::map<sc2::Tag, size_t> pendingDronesForHive;
  public:
    ManageDrones(size_t period) : StrategyBase(period) {}

    void OnGameStart();
    void OnPeriod();
    void OnUnitCreated(const Unit &unit);
    void OnUnitIdle(const Unit &unit);

  private:
    void AssignWork(const Unit &drone);
    void MineMinerals(const Unit &drone);
    void MineVespeneGas(const Unit &drone);

    inline bool IsDrone(const Unit &unit) {
      return unit.unit_type == UNIT_TYPEID::ZERG_DRONE;
    }

    // class DroneList {
    //   std::map<sc2::Tag, std::tuple<size_t, std::vector<sc2::Tag> > > list;
    // public:
    //   std::vector<sc2::Tag> GetDrones(sc2::Tag hive) {
    //     return std::get<1>(list[hive]);
    //   }
    //   size_t GetPendingDrones(sc2::Tag hive) { return std::get<0>(list[hive]);}
    //   void AddPendingDrone(sc2::Tag hive) {
    //     list[hive] = std::make_tuple(GetPendingDrones(hive)+1, GetDrones(hive));
    //   }
    //   // FIXME: Reduce pending drones when larva is killed or when drone gets
    //   //        assigned to wrong hive.
    //   void AssignDroneToHive(sc2::Tag drone, sc2::Tag hive) {
    //     std::vector<sc2::Tag> drones;
    //     size_t pendingDrones = 0;
    //     if (list.find(hive) != list.end()) {
    //       drones = GetDrones(hive);
    //       pendingDrones = (GetPendingDrones(hive) == 0)
    //                     ? 0 : GetPendingDrones(hive) - 1;
    //     }
    //     drones.push_back(drone);
    //     list[hive] = std::make_tuple(pendingDrones, drones);
    //   }
    //   void RemoveDrone(sc2::Tag drone) {
    //     for (auto hiveDrones : list) {
    //       auto drones = GetDrones(hiveDrones.first);
    //       for (auto it = drones.begin(); it != drones.end(); it++) {
    //         if (*it == drone) {
    //           drones.erase(it);
    //         }
    //       }
    //     }
    //   }
    // } droneList;
  };

  class ManageQueens : public StrategyBase {
    struct Queen { Tag tag, hiveTag; };
    using Queens = std::vector<Queen>;
    Queens queens;
  public:
    ManageQueens(size_t period) : StrategyBase(period) {}

    void OnPeriod();
    void OnUnitCreated(const Unit &unit);
    void OnUnitDestroyed(const Unit &unit);

  private:
    void TrainQueens();
    void SpawnCreep(Queen queen);
    void InjectLarva(Queen queen);
    void Attack(Queen queen);

    bool IsQueen(const Unit &unit) {
      return unit.unit_type == UNIT_TYPEID::ZERG_QUEEN;
    }
  };

  class ExpandCreep : public StrategyBase {
  public:
    ExpandCreep(size_t period) : StrategyBase(period) {}

    void OnPeriod();

  private:
  };

  // No thanks, I don't want hives.
  static const sc2::Units GetHives(const ObservationInterface *observation) {
    auto hiveFilter = [](const Unit &unit) {
      return IsUnit(UNIT_TYPEID::ZERG_HIVE)(unit)
          || IsUnit(UNIT_TYPEID::ZERG_LAIR)(unit)
          || IsUnit(UNIT_TYPEID::ZERG_HATCHERY)(unit);
    };
    return observation->GetUnits(Unit::Alliance::Self, hiveFilter);
  }
  static const sc2::Units GetMineralFields(const ObservationInterface *observation) {
    auto mineralFieldFilter = [](const Unit &unit) {
      return IsUnit(UNIT_TYPEID::NEUTRAL_LABMINERALFIELD)(unit)
          || IsUnit(UNIT_TYPEID::NEUTRAL_LABMINERALFIELD750)(unit)
          || IsUnit(UNIT_TYPEID::NEUTRAL_MINERALFIELD)(unit)
          || IsUnit(UNIT_TYPEID::NEUTRAL_MINERALFIELD750)(unit)
          || IsUnit(UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD)(unit)
          || IsUnit(UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD750)(unit);
    };
    return observation->GetUnits(Unit::Alliance::Neutral, mineralFieldFilter);
  }
};

// class ZergBot : public Agent {
//
// private:
//   std::vector<FoundationBuilding> foundationBuildings;
//   std::vector<Point2D> expansions;
//   std::vector<Tag> workerTags;
//   int32_t pendingSupply;
//
//   void TryExpand();
//
//   void TryExpandCreep();
//
//   void WorkEconomy();
//
//   bool NeedSupply();
//
//   void TryGetSupply();
//
//   void TryBuildStructure(ABILITY_ID);
//
// public:
//   ZergBot() : foundationBuildings(), expansions(), workerTags(),
//           pendingSupply(0) {}
//
//   virtual void OnGameStart() final;
//
//   virtual void OnStep() final;
//
//   virtual void OnUnitCreated(const Unit &);
//
//   virtual void OnUnitDestroyed(const Unit &);
//
//   virtual void OnUnitIdle(const sc2::Unit &) final;
// };
}
