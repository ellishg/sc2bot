#pragma once

#include <sc2lib/sc2_lib.h>
#include <sc2api/sc2_api.h>

#include "common.h"

class Bot : public sc2::Agent {

private:

  sc2::Units foundationBuildings;
  std::vector<sc2::Point2D> _expansions;
  sc2::Units _workers;
  sc2::Units _queens;

public:

  virtual void OnGameStart() final;

  virtual void OnStep() final;

  virtual void OnUnitCreated(const sc2::Unit& unit);

  virtual void OnUnitDestroyed(const sc2::Unit & unit);

  virtual void OnUnitIdle(const sc2::Unit& unit) final;

  bool NeedSupply();

  bool NeedsMineralHarvesters(const sc2::Unit * foundationBuilding);

  bool NeedsQueen(const sc2::Unit& foundationBuilding);

  void TryExpand();

  void WorkEconomy();
};
