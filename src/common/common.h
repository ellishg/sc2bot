#pragma once

#include <sc2api/sc2_api.h>

bool FindNearestUnit(const sc2::Point2D& start, const sc2::Units& units, uint64_t * target, sc2::Filter filter = std::function<bool(const sc2::Unit&)>([](const sc2::Unit&){return true;}));

bool FindNearestPoint2D(const sc2::Point2D& point, const std::vector<sc2::Point2D>& candidates, sc2::Point2D * target, std::function<bool(const sc2::Point2D&)> filter = std::function<bool(const sc2::Point2D&)>([](const sc2::Point2D&){return true;}));
