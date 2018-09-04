#pragma once

#include <sc2api/sc2_api.h>

class SuggestedAction {
public:
  using IssueFunc = std::function<void()>;
  using RejectFunc = std::function<std::vector<SuggestedAction>()>;

  enum PRIORITY_LEVEL : int {
    LOW = 0,
    MEDIUM = 50,
    HIGH = 100,
    DEFAULT = MEDIUM
  };

private:
  uint32_t kindBits;
  int priority;
  sc2::ABILITY_ID ability_id;
  std::vector<sc2::Tag> sourceTags;

  // TODO: Make this a union.
  sc2::Tag targetTag;
  sc2::Point2D targetPoint;

  struct {
    uint32_t minerals;
    uint32_t vespeneGas;
    uint32_t supply;
  } requiredResources;

  IssueFunc issueActionCallback;

  // TODO
  void ActionToResources();
  bool hasTarget() { return kindBits & 0b001; }
  bool isTargetTag() { return hasTarget() && (kindBits & 0b010); }

public:
  SuggestedAction(sc2::Tag tag, sc2::ABILITY_ID ability_id,
                  int priority = PRIORITY_LEVEL::DEFAULT,
                  IssueFunc issue = []{},
                  RejectFunc reject = []{ return std::vector<SuggestedAction>(); })
      : sourceTags({tag}), ability_id(ability_id), priority(priority),
        issueActionCallback(issue), Reject(reject), kindBits(0b000) {}

  SuggestedAction(sc2::Tag tag, sc2::ABILITY_ID ability_id, sc2::Tag targetTag,
                  int priority = PRIORITY_LEVEL::DEFAULT,
                  IssueFunc issue = []{},
                  RejectFunc reject = []{ return std::vector<SuggestedAction>(); })
      : sourceTags({tag}), ability_id(ability_id), targetTag(targetTag),
        priority(priority), issueActionCallback(issue), Reject(reject),
        kindBits(0b011) {}

  SuggestedAction(sc2::Tag tag, sc2::ABILITY_ID ability_id,
                  sc2::Point2D targetPoint,
                  int priority = PRIORITY_LEVEL::DEFAULT,
                  IssueFunc issue = []{},
                  RejectFunc reject = []{  return std::vector<SuggestedAction>(); })
      : sourceTags({tag}), ability_id(ability_id), targetPoint(targetPoint),
        priority(priority), issueActionCallback(issue), Reject(reject),
        kindBits(0b001) {}

  SuggestedAction(std::vector<sc2::Tag> tags, sc2::ABILITY_ID ability_id,
                  int priority = PRIORITY_LEVEL::DEFAULT,
                  IssueFunc issue = []{},
                  RejectFunc reject = []{ return std::vector<SuggestedAction>(); })
      : sourceTags(tags), ability_id(ability_id), priority(priority),
        issueActionCallback(issue), Reject(reject), kindBits(0b000) {
    assert(tags.size() > 1 && "need more than one source unit");
  }

  SuggestedAction(std::vector<sc2::Tag> tags, sc2::ABILITY_ID ability_id,
                  sc2::Tag targetTag, int priority = PRIORITY_LEVEL::DEFAULT,
                  IssueFunc issue = []{},
                  RejectFunc reject = []{ return std::vector<SuggestedAction>(); })
      : sourceTags(tags), ability_id(ability_id), targetTag(targetTag),
        priority(priority), issueActionCallback(issue), Reject(reject),
        kindBits(0b011) {
    assert(tags.size() > 1 && "need more than one source unit");
  }

  SuggestedAction(std::vector<sc2::Tag> tags, sc2::ABILITY_ID ability_id,
                  sc2::Point2D targetPoint,
                  int priority = PRIORITY_LEVEL::DEFAULT,
                  IssueFunc issue = []{},
                  RejectFunc reject = []{ return std::vector<SuggestedAction>(); })
      : sourceTags(tags), ability_id(ability_id), targetPoint(targetPoint),
        priority(priority), issueActionCallback(issue), Reject(reject),
        kindBits(0b001) {
    assert(tags.size() > 1 && "need more than one source unit");
  }

  void Issue(sc2::ActionInterface *action) {
    if (sourceTags.size() == 1) {
      if (not hasTarget()) {
        action->UnitCommand(sourceTags[0], ability_id);
      } else if (isTargetTag()) {
        action->UnitCommand(sourceTags[0], ability_id, targetTag);
      } else {
        action->UnitCommand(sourceTags[0], ability_id, targetPoint);
      }
    } else {
      if (not hasTarget()) {
        action->UnitCommand(sourceTags, ability_id);
      } else if (isTargetTag()) {
        action->UnitCommand(sourceTags, ability_id, targetTag);
      } else {
        action->UnitCommand(sourceTags, ability_id, targetPoint);
      }
    }
    issueActionCallback();
  }

  RejectFunc Reject;

  bool IsAffordable(const sc2::ObservationInterface *observation) {
    return true;
    // TODO
  }

  // High priority actions come before low priority actions.
  friend bool operator< (const SuggestedAction &a, const SuggestedAction &b) {
    return a.priority > b.priority;
  }
};

using Suggestions = std::vector<SuggestedAction>;

#define VECTOR_COMBINE(vec, genElems) do { \
  auto newVec = genElems; \
  std::move(newVec.begin(), newVec.end(), std::inserter(vec, vec.end())); \
} while (0);
