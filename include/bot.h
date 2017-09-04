#pragma once

#include <sc2lib/sc2_lib.h>
#include <sc2api/sc2_api.h>

#include "foundation_building.h"
#include "common.h"

class Bot : public sc2::Agent {

private:

  std::vector<FoundationBuilding> _foundationBuildings;
  std::vector<sc2::Point2D> _expansions;
  std::vector<sc2::Tag> _workerTags;
  int32_t _pendingSupply;

  void TryExpand();

  void TryExpandCreep();

  void WorkEconomy();

  bool NeedSupply();

  void TryGetSupply();

  void TryBuildStructure(sc2::ABILITY_ID buildStructureAbility);

public:

  Bot() : _foundationBuildings(), _expansions(), _workerTags(), _pendingSupply(0) {}

  virtual void OnGameStart() final;

  virtual void OnStep() final;

  virtual void OnUnitCreated(const sc2::Unit& unit);

  virtual void OnUnitDestroyed(const sc2::Unit & unit);

  virtual void OnUnitIdle(const sc2::Unit& unit) final;
};
