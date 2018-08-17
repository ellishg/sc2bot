#pragma once

#include <iostream>
#include <sc2lib/sc2_lib.h>
#include <sc2api/sc2_api.h>

#include "StrategyBase.h"

using StrategyList = std::vector<StrategyBase *>;

class BotBase : public sc2::Agent {

  StrategyList strategies;

protected:
  BotBase(StrategyList _strategies) : strategies(_strategies) {
    // TODO: If running in realtime I need to call SendActions()
  }

  // TODO: Attempt fancy metaprogramming here :)

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
    for (auto strategy : strategies)
      strategy->OnTick(Actions(), Observation(), Query());
  }

  // Called whenever one of the player's units has been destroyed.
  virtual void OnUnitDestroyed(const sc2::Unit& unit) {
    for (auto strategy : strategies)
      strategy->OnUnitDestroyed(Actions(), Observation(), Query(), unit);
  }

  // Called when a Unit has been created by the player.
  virtual void OnUnitCreated(const sc2::Unit& unit) {
    for (auto strategy : strategies)
      strategy->OnUnitCreated(Actions(), Observation(), Query(), unit);
  }

  // Called when a unit becomes idle, this will only occur as an event so
  // will only be called when the unit becomes idle and not a second time.
  // Being idle is defined by having orders in the previous step and not
  // currently having orders or if it did not exist in the previous step
  // and now does, a unit being created, for instance, will call both
  // OnUnitCreated and OnUnitIdle if it does not have a rally set.
  virtual void OnUnitIdle(const sc2::Unit& unit) {
    for (auto strategy : strategies)
      strategy->OnUnitIdle(Actions(), Observation(), Query(), unit);
  }

  // Called when an upgrade is finished, warp gate, ground weapons, baneling
  // speed, etc.
  virtual void OnUpgradeCompleted(sc2::UpgradeID id) {
    for (auto strategy : strategies)
      strategy->OnUpgradeCompleted(Actions(), Observation(), Query(), id);
  }

  // Called when the unit in the previous step had a build progress less than
  // 1.0 but is greater than or equal to 1.0 in the current step.
  virtual void OnBuildingConstructionComplete(const sc2::Unit& unit) {
    for (auto strategy : strategies)
      strategy->OnBuildingConstructionComplete(Actions(), Observation(),
                                               Query(), unit);
  }

  // Called when a nydus is placed.
  virtual void OnNydusDetected() {
    for (auto strategy : strategies)
      strategy->OnNydusDetected(Actions(), Observation(), Query());
  }

  // Called when a nuclear launch is detected.
  virtual void OnNuclearLaunchDetected() {
    for (auto strategy : strategies)
      strategy->OnNuclearLaunchDetected(Actions(), Observation(), Query());
  }

  // Called when an enemy unit enters vision from out of fog of war.
  virtual void OnUnitEnterVision(const sc2::Unit& unit) {
    for (auto strategy : strategies)
      strategy->OnUnitEnterVision(Actions(), Observation(), Query(), unit);
  }

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
