#pragma once

#include <sc2api/sc2_api.h>

class StrategyBase {
  const size_t period;
  size_t ticks;

protected:
  sc2::ActionInterface * action;
  const sc2::ObservationInterface * observation;
  sc2::QueryInterface * query;

public:
  StrategyBase(size_t _period, bool staggered = true)
        : period(_period), ticks(staggered ? rand() % 20 : 0) {
    assert(period > 0 && "must specify a positive period");
  }

  virtual void OnPeriod() {}

  virtual void OnGameFullStart() {}
  virtual void OnGameStart() {}
  virtual void OnGameEnd() {}

  // Fancy macro metaprogramming :)
  #define CAPTUREMETHOD(NAME) \
  virtual void NAME() {} \
  virtual void NAME(sc2::ActionInterface *_action, \
                    const sc2::ObservationInterface *_observation, \
                    sc2::QueryInterface *_query) { \
    action = _action; observation = _observation; query = _query; NAME(); \
  }

  #define CAPTUREMETHODWITHARG(NAME, ARGTYPE) \
  virtual void NAME(ARGTYPE) {} \
  virtual void NAME(sc2::ActionInterface *_action, \
                    const sc2::ObservationInterface *_observation, \
                    sc2::QueryInterface *_query, ARGTYPE arg) { \
    action = _action; observation = _observation; query = _query; NAME(arg); \
  }

  CAPTUREMETHODWITHARG(OnUnitCreated, const sc2::Unit&)
  CAPTUREMETHODWITHARG(OnUnitDestroyed, const sc2::Unit&)
  CAPTUREMETHODWITHARG(OnUnitIdle, const sc2::Unit&)
  CAPTUREMETHODWITHARG(OnUpgradeCompleted, sc2::UpgradeID)
  CAPTUREMETHODWITHARG(OnBuildingConstructionComplete, const sc2::Unit&)
  CAPTUREMETHODWITHARG(OnUnitEnterVision, const sc2::Unit&)
  CAPTUREMETHOD(OnNydusDetected)
  CAPTUREMETHOD(OnNuclearLaunchDetected)

  void OnTick(sc2::ActionInterface *_action,
              const sc2::ObservationInterface *_observation,
              sc2::QueryInterface *_query) {
    if (++ticks >= period) {
      action = _action; observation = _observation; query = _query;
      ticks = 0;
      OnPeriod();
    }
  }

  // virtual void OnUnitDestroyed(sc2::ActionInterface *,
  //                              const sc2::ObservationInterface *,
  //                              sc2::QueryInterface *, const sc2::Unit&) {}
  // virtual void OnUnitCreated(sc2::ActionInterface *,
  //                            const sc2::ObservationInterface *,
  //                            sc2::QueryInterface *, const sc2::Unit&){}
  // virtual void OnUnitIdle(sc2::ActionInterface *,
  //                         const sc2::ObservationInterface *,
  //                         sc2::QueryInterface *, const sc2::Unit&) {}
  // virtual void OnUpgradeCompleted(sc2::ActionInterface *,
  //                                 const sc2::ObservationInterface *,
  //                                 sc2::QueryInterface *, sc2::UpgradeID) {}
  // virtual void OnBuildingConstructionComplete(sc2::ActionInterface *,
  //                                             const sc2::ObservationInterface *,
  //                                             sc2::QueryInterface *,
  //                                             const sc2::Unit&) {}
  // virtual void OnNydusDetected(sc2::ActionInterface *,
  //                              const sc2::ObservationInterface *,
  //                              sc2::QueryInterface *) {}
  // virtual void OnNuclearLaunchDetected(sc2::ActionInterface *,
  //                                      const sc2::ObservationInterface *,
  //                                      sc2::QueryInterface *) {}
  // virtual void OnUnitEnterVision(sc2::ActionInterface *,
  //                                const sc2::ObservationInterface *,
  //                                sc2::QueryInterface *, const sc2::Unit&) {}
};
