#pragma once

#include <sc2api/sc2_api.h>
#include <iostream>

bool FindNearestUnit(const sc2::Point2D& start, const sc2::Units& units, uint64_t * target, sc2::Filter filter = std::function<bool(const sc2::Unit&)>([](const sc2::Unit&){return true;}));

bool FindNearestPoint2D(const sc2::Point2D& point, const std::vector<sc2::Point2D>& candidates, sc2::Point2D * target, std::function<bool(const sc2::Point2D&)> filter = std::function<bool(const sc2::Point2D&)>([](const sc2::Point2D&){return true;}));

bool IsUnitAbilityAvailable(sc2::QueryInterface* query, sc2::Tag unitTag, sc2::ABILITY_ID abilityID);

std::vector<sc2::Tag> UnitsWithAbility(sc2::QueryInterface* query, std::vector<sc2::Tag> unitTags, sc2::ABILITY_ID abilityID);

// TODO: Use batched version of sc2::QueryInterface::Placement
bool FindRandomPoint(const sc2::Point2D& origin, sc2::Point2D* target, float radius, std::function<bool(const sc2::Point2D&)> filter = [](const sc2::Point2D&){return true;});
