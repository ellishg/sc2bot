#include "common.h"

using namespace std;
using namespace sc2;

bool FindNearestUnit(const Point2D &start, const Units &units,
                     uint64_t *target, Filter filter) {
  bool found = false;
  float bestDistanceSqrd;
  for (const auto &u : units) {
    if (filter(u)) {
      float dSqrd = DistanceSquared2D(u.pos, start);
      if (!found || dSqrd < bestDistanceSqrd) {
        bestDistanceSqrd = dSqrd;
        *target = u.tag;
      }
      found = true;
    }
  }
  return found;
}

bool FindNearestPoint2D(const Point2D &point, const vector<Point2D> &candidates,
                        Point2D * target, Point2DFilter filter) {
  bool found = false;
  float bestDistanceSqrd;
  for (const auto &c : candidates) {
    if (filter(c)) {
      float distSqrd = DistanceSquared2D(point, c);
      if (!found || distSqrd < bestDistanceSqrd) {
        bestDistanceSqrd = distSqrd;
        *target = c;
      }
      found = true;
    }
  }
  return found;
}

bool IsUnitAbilityAvailable(QueryInterface *query, Tag unitTag,
                            ABILITY_ID abilityID) {
  auto abilities = query->GetAbilitiesForUnit(unitTag);
  for (auto &a : abilities.abilities) {
    if (a.ability_id == abilityID) {
      return true;
    }
  }
  return false;
}

vector<Tag> UnitsWithAbility(QueryInterface *query, vector<Tag> unitTags,
                             ABILITY_ID abilityID) {
  vector<Tag> capableUnits;
  auto abilityLists = query->GetAbilitiesForUnits(unitTags);
  for (auto &availableAbilities : abilityLists) {
    for (auto &ability : availableAbilities.abilities) {
      if (ability.ability_id == abilityID) {
        capableUnits.push_back(availableAbilities.unit_tag);
        break;
      }
    }
  }
  return capableUnits;
}

bool FindRandomPoint(const Point2D& origin, Point2D* target, float radius,
                     Point2DFilter filter) {
  const int numTrials = 100;
  for (size_t i = 0; i < numTrials; i++) {
    float r = radius * GetRandomScalar(), theta = GetRandomScalar();
    auto testPoint = origin + Point2D(r * cos(theta), r * sin(theta));
    if (filter(testPoint)) {
      *target = testPoint;
      return true;
    }
  }
  return false;
}
