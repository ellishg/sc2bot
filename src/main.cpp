#include "bot.h"

using namespace sc2;
using namespace std;

int main(int argc, char* argv[]) {
  Coordinator coordinator;
  coordinator.LoadSettings(argc, argv);
  //  coordinator.SetRealtime(true);

  Bot bot;
  coordinator.SetParticipants({
    CreateParticipant(Race::Zerg, &bot),
    CreateComputer(Race::Terran)
  });

  coordinator.LaunchStarcraft();
  coordinator.StartGame(kMapBelShirVestigeLE);

  while (coordinator.Update()) {
  }

  return 0;
}
