#pragma once

#include <sc2api/sc2_api.h>
#include <iostream>
#include <math.h>

using Point2DFilter = std::function<bool(const sc2::Point2D&)>;
const auto FilterNone = [](const auto&){return true;};

bool FindNearestUnit(const sc2::Point2D &, const sc2::Units &, uint64_t *,
                     sc2::Filter = FilterNone);

bool FindNearestPoint2D(const sc2::Point2D &, const std::vector<sc2::Point2D> &,
                        sc2::Point2D *, Point2DFilter = FilterNone);

bool IsUnitAbilityAvailable(sc2::QueryInterface *, sc2::Tag, sc2::ABILITY_ID);

std::vector<sc2::Tag> UnitsWithAbility(sc2::QueryInterface *,
                                       std::vector<sc2::Tag>, sc2::ABILITY_ID);

// TODO: Use batched version of sc2::QueryInterface::Placement
bool FindRandomPoint(const sc2::Point2D &, sc2::Point2D *, float,
                     Point2DFilter = FilterNone);
