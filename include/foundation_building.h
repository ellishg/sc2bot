#pragma once

#include <iostream>

#include "common.h"

class FoundationBuilding {
private:
  size_t numPendingWorkers;
  size_t numPendingQueens;

  bool NeedsMineralHarvesters(const sc2::ObservationInterface* observation);

  bool NeedsQueen();

public:
  sc2::Tag buildingTag;
  std::vector<sc2::Tag> queenTags;

  FoundationBuilding(sc2::Tag _buildingTag) : buildingTag(_buildingTag), queenTags(), numPendingWorkers(0), numPendingQueens(0) {}

  bool OnQueenCreated(sc2::ActionInterface* action, sc2::Tag queenTag);

  bool OnQueenDestroyed(sc2::Tag queenTag);

  void TryTrainWorker(sc2::ActionInterface* action, const sc2::ObservationInterface* observation, sc2::QueryInterface* query);

  bool TryAssignWorker(sc2::ActionInterface* actions, const sc2::ObservationInterface* observation, const sc2::Units& mineralFields, const sc2::Unit& worker);

  void TryTrainQueen(sc2::ActionInterface* action, const sc2::ObservationInterface* observation, sc2::QueryInterface* query);

  void TryExpand();
};
