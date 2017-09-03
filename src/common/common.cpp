#include "common.h"

using namespace std;
using namespace sc2;

bool FindNearestUnit(const Point2D& start, const Units& units, uint64_t * target, Filter filter) {
  if (units.size() == 0) {
    return false;
  }
  float bestDistanceSqrd = std::numeric_limits<float>::max();
  for (const auto& u : units) {
    if (filter(u)) {
      float dSqrd = DistanceSquared2D(u.pos, start);
      if (dSqrd < bestDistanceSqrd) {
        bestDistanceSqrd = dSqrd;
        *target = u.tag;
      }
    }
  }
  if (bestDistanceSqrd == numeric_limits<float>::max()) {
    return false;
  }
  return true;
}

bool FindNearestPoint2D(const Point2D& point, const vector<Point2D>& candidates, Point2D * target, function<bool(const Point2D&)> filter) {
  if (candidates.size() == 0) {
    return false;
  }
  float bestDistanceSqrd = numeric_limits<float>::max();
  for (auto & c : candidates) {
    if (filter(c)) {
      float distSqrd = DistanceSquared2D(point, c);
      if (distSqrd < bestDistanceSqrd) {
        bestDistanceSqrd = distSqrd;
        *target = c;
      }
    }
  }
  if (bestDistanceSqrd == numeric_limits<float>::max()) {
    return false;
  }
  return true;
}

bool IsUnitAbilityAvailable(QueryInterface* query, Tag unitTag, ABILITY_ID abilityID) {
  AvailableAbilities abilities = query->GetAbilitiesForUnit(unitTag);
  for (const auto& a : abilities.abilities) {
    if (a.ability_id == abilityID) {
      return true;
    }
  }
  return false;
}

bool FindRandomPoint(const Point2D& origin, Point2D* target, float radius, function<bool(const Point2D&)> filter) {
  const int numTests = 100;
  for (size_t i = 0; i < numTests; i++) {
    Point2D testPoint = Point2D(GetRandomScalar(), GetRandomScalar());
    Normalize2D(testPoint);
    testPoint = origin + (testPoint * radius);
    if (filter(testPoint)) {
      *target = testPoint;
      return true;
    }
  }
  return false;
}
