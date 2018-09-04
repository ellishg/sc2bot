#pragma once

#include <iostream>
#include <algorithm>
#include <sc2lib/sc2_lib.h>
#include <sc2api/sc2_api.h>

#include "StrategyBase.h"

using StrategyList = std::vector<StrategyBase *>;

class BotBase : public sc2::Agent {

  StrategyList strategies;
  Suggestions suggestions;

  // There may be many strategies for issuing commands.
  // First idea: Sort by priority and issue commands until you run out of
  //             resources. We stop once we cannot issue one command because
  //             that command has high priority.
  void IssueCommands() {
    // TODO: Also issue any free actions.
    // TODO: Provide a callback function so that when an action is rejected,
    //       we can make a new action.
    std::sort(suggestions.begin(), suggestions.end());

    Suggestions newSuggestions;
    bool isIssuingCommands = true;
    for (auto suggestion : suggestions) {
      // FIXME: Does `Observation()` change? I want to make sure that we don't
      //        think we can afford everything because we aren't keeping track
      //        of resources used!!!!!!
      if (isIssuingCommands && suggestion.IsAffordable(Observation())) {
        suggestion.Issue(Actions());
      } else {
        VECTOR_COMBINE(newSuggestions, suggestion.Reject());
        isIssuingCommands = false;
      }
    }
    suggestions.clear();  // Is this needed?
    suggestions = newSuggestions;
  }

protected:
  BotBase(StrategyList _strategies) : strategies(_strategies) {
    // TODO: If running in realtime I need to call SendActions()
  }

  // Called when a game is started after a load. Fast restarting will not
  // call this.
  virtual void OnGameFullStart() {
    for (auto strategy : strategies)
      strategy->OnGameFullStart();
  }

  // Called when a game is started or restarted.
  virtual void OnGameStart() {
    for (auto strategy : strategies)
      strategy->OnGameStart();
  }

  // Called when a game has ended.
  virtual void OnGameEnd() {
    for (auto strategy : strategies)
      strategy->OnGameEnd();
  }

  // This event will only get called when stepping. It will not get called
  // in a real time game. In a real time game the user will be responsible
  // for calling GetObservation() via the ObservationInterface.
  virtual void OnStep() {
    for (auto strategy : strategies) {
      VECTOR_COMBINE(suggestions, strategy->OnTick(Observation(), Query()));
    }
    IssueCommands();
  }

  // Fancy metaprogramming :)

  #define GENERATE_METHOD_WITH_ARG(NAME, ARGTYPE) \
  virtual void NAME(ARGTYPE a) { \
    for (auto strategy : strategies) \
      VECTOR_COMBINE(suggestions, strategy->NAME(Observation(), Query(), a)); \
    IssueCommands(); \
  }

  #define GENERATE_METHOD(NAME) \
  virtual void NAME() { \
    for (auto strategy : strategies) \
      VECTOR_COMBINE(suggestions, strategy->NAME(Observation(), Query())); \
    IssueCommands(); \
  }


  // Called whenever one of the player's units has been destroyed.
  GENERATE_METHOD_WITH_ARG(OnUnitDestroyed, const sc2::Unit&)

  // Called when a Unit has been created by the player.
  GENERATE_METHOD_WITH_ARG(OnUnitCreated, const sc2::Unit&)

  // Called when a unit becomes idle, this will only occur as an event so
  // will only be called when the unit becomes idle and not a second time.
  // Being idle is defined by having orders in the previous step and not
  // currently having orders or if it did not exist in the previous step
  // and now does, a unit being created, for instance, will call both
  // OnUnitCreated and OnUnitIdle if it does not have a rally set.
  GENERATE_METHOD_WITH_ARG(OnUnitIdle, const sc2::Unit&)

  // Called when an upgrade is finished, warp gate, ground weapons, baneling
  // speed, etc.
  GENERATE_METHOD_WITH_ARG(OnUpgradeCompleted, sc2::UpgradeID)

  // Called when the unit in the previous step had a build progress less than
  // 1.0 but is greater than or equal to 1.0 in the current step.
  GENERATE_METHOD_WITH_ARG(OnBuildingConstructionComplete, const sc2::Unit&)

  // Called when a nydus is placed.
  GENERATE_METHOD(OnNydusDetected)

  // Called when a nuclear launch is detected.
  GENERATE_METHOD(OnNuclearLaunchDetected)

  // Called when an enemy unit enters vision from out of fog of war.
  GENERATE_METHOD_WITH_ARG(OnUnitEnterVision, const sc2::Unit&)

  #undef GENERATE_METHOD_WITH_ARG
  #undef GENERATE_METHOD

  // Called for various errors the library can encounter. See ClientError
  // enum for possible errors.
  void OnError(const std::vector<sc2::ClientError>& client_errors,
               const std::vector<std::string>& protocol_errors = {}) {
    for (auto error : client_errors) {
      switch (error) {
        case sc2::ClientError::ErrorSC2:
          std::cout << "Some error occured\n";
          break;
        case sc2::ClientError::InvalidAbilityRemap:
          std::cout << "An ability was improperly mapped to an ability id that "
                       "doesn't exist\n";
          break;
        case sc2::ClientError::InvalidResponse:
          std::cout << "The response does not contain a field that was "
                       "expected\n";
          break;
        case sc2::ClientError::NoAbilitiesForTag:
          std::cout << "The unit does not have any abilities\n";
          break;
        case sc2::ClientError::ResponseNotConsumed:
          std::cout << "A request was made without consuming the response from "
                       "the previous request, that puts this library in an "
                       "illegal state\n";
          break;
        case sc2::ClientError::ResponseMismatch:
          std::cout << "The response received from SC2 does not match the "
                       "request\n";
          break;
        case sc2::ClientError::ConnectionClosed:
          std::cout << "The websocket connection has prematurely closed, this "
                       "could mean starcraft crashed or a websocket timeout "
                       "has occurred\n";
          break;
        case sc2::ClientError::SC2UnknownStatus:
          std::cout << "An unknown error occured.\n";
          break;
        case sc2::ClientError::SC2AppFailure:
          std::cout << "SC2 has either crashed or been forcibly terminated by "
                       "this library because it was not responding to "
                       "requests\n";
          break;
        case sc2::ClientError::SC2ProtocolError:
          std::cout << "The response from SC2 contains errors, most likely "
                       "meaning the API was not used in a correct way\n";
          break;
        case sc2::ClientError::SC2ProtocolTimeout:
          std::cout << "A request was made and a response was not received in "
                       "the amount of time given by the timeout\n";
          break;
        case sc2::ClientError::WrongGameVersion:
          std::cout << "A replay was attempted to be loaded in the wrong game "
                       "version\n";
          break;
      }
    }
    for (auto error : protocol_errors)
      std::cout << error;
  }
};
