#pragma once

#include <sc2api/sc2_api.h>

#include "SuggestedAction.h"

class StrategyBase {
  const size_t period;
  size_t ticks;

protected:
  const sc2::ObservationInterface * observation;
  sc2::QueryInterface * query;

public:
  StrategyBase(size_t _period, bool staggered = true)
        : period(_period), ticks(staggered ? rand() % 20 : 0) {
    assert(period > 0 && "must specify a positive period");
  }

  virtual Suggestions OnPeriod() { return {}; }

  virtual void OnGameFullStart() {}
  virtual void OnGameStart() {}
  virtual void OnGameEnd() {}

  // Fancy macro metaprogramming :)
  #define CAPTURE_METHOD(NAME) \
  virtual Suggestions NAME() { return {}; } \
  virtual Suggestions NAME(const sc2::ObservationInterface *_observation, \
                           sc2::QueryInterface *_query) { \
    observation = _observation; query = _query; return NAME(); \
  }

  #define CAPTURE_METHOD_WITH_ARG(NAME, ARGTYPE) \
  virtual Suggestions NAME(ARGTYPE) { return {}; } \
  virtual Suggestions NAME(const sc2::ObservationInterface *_observation, \
                           sc2::QueryInterface *_query, ARGTYPE arg) { \
    observation = _observation; query = _query; return NAME(arg); \
  }

  CAPTURE_METHOD_WITH_ARG(OnUnitCreated, const sc2::Unit&)
  CAPTURE_METHOD_WITH_ARG(OnUnitDestroyed, const sc2::Unit&)
  CAPTURE_METHOD_WITH_ARG(OnUnitIdle, const sc2::Unit&)
  CAPTURE_METHOD_WITH_ARG(OnUpgradeCompleted, sc2::UpgradeID)
  CAPTURE_METHOD_WITH_ARG(OnBuildingConstructionComplete, const sc2::Unit&)
  CAPTURE_METHOD_WITH_ARG(OnUnitEnterVision, const sc2::Unit&)
  CAPTURE_METHOD(OnNydusDetected)
  CAPTURE_METHOD(OnNuclearLaunchDetected)

  #undef CAPTURE_METHOD_WITH_ARG
  #undef CAPTURE_METHOD

  Suggestions OnTick(const sc2::ObservationInterface *_observation,
                     sc2::QueryInterface *_query) {
    if (++ticks >= period) {
      observation = _observation;
      query = _query;
      ticks = 0;
      return OnPeriod();
    }
    return {};
  }
};
