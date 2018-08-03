#pragma once

#include <sc2lib/sc2_lib.h>
#include <sc2api/sc2_api.h>

#include "FoundationBuilding.h"
#include "common.h"

class ZergBot : public sc2::Agent {

private:
  std::vector<FoundationBuilding> foundationBuildings;
  std::vector<sc2::Point2D> expansions;
  std::vector<sc2::Tag> workerTags;
  int32_t pendingSupply;

  void TryExpand();

  void TryExpandCreep();

  void WorkEconomy();

  bool NeedSupply();

  void TryGetSupply();

  void TryBuildStructure(sc2::ABILITY_ID buildStructureAbility);

public:
  ZergBot() : foundationBuildings(), expansions(), workerTags(),
          pendingSupply(0) {}

  virtual void OnGameStart() final;

  virtual void OnStep() final;

  virtual void OnUnitCreated(const sc2::Unit& unit);

  virtual void OnUnitDestroyed(const sc2::Unit & unit);

  virtual void OnUnitIdle(const sc2::Unit& unit) final;
};
