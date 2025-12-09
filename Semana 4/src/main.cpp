#include <iostream>
#include "Team.h"
#include "World.h"

int main(int argc, char** argv) {
    const int NUM_AGENTS = 5;
    const int TICKS = 20;

    World world;
    Team team(NUM_AGENTS, world);

    std::cout << "Alanomal - simulación inicializada\n";
    for (int t = 0; t < TICKS; ++t) {
        std::cout << "=== Tick " << t << " ===\n";
        world.tick();
        team.update();
        team.printStatus();
    }

    std::cout << "Simulación finalizada\n";
    return 0;
}
