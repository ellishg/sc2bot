#pragma once

#include <iostream>

#include "common.h"

class FoundationBuilding {
private:
  size_t numPendingWorkers;
  size_t numPendingQueens;

  bool NeedsMineralHarvesters(const sc2::ObservationInterface *);

  bool NeedsQueen();

public:
  sc2::Tag buildingTag;
  std::vector<sc2::Tag> queenTags;

  FoundationBuilding(sc2::Tag _buildingTag)
      : buildingTag(_buildingTag), queenTags(),
        numPendingWorkers(0), numPendingQueens(0) {}

  bool OnQueenCreated(sc2::Tag);

  bool OnQueenDestroyed(sc2::Tag);

  void TryTrainWorker(sc2::ActionInterface *, const sc2::ObservationInterface *,
                      sc2::QueryInterface*);

  bool TryAssignWorker(sc2::ActionInterface *, const sc2::ObservationInterface *,
                       const sc2::Units &, const sc2::Unit &);

  void TryTrainQueen(sc2::ActionInterface *, sc2::QueryInterface *);

  void TryInjectLarva(sc2::ActionInterface *, sc2::QueryInterface *);

  void TryBuildCreepTumor(sc2::ActionInterface *,
                          const sc2::ObservationInterface *,
                          sc2::QueryInterface *);
};
