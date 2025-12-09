#include "Team.h"
#include <iostream>

Team::Team(int numAgents, World& worldRef) : world_(worldRef) {
    agents_.reserve(numAgents);
    for (int i = 0; i < numAgents; ++i) {
        agents_.emplace_back(i + 1);
    }
}

void Team::update() {
    for (auto &a : agents_) {
        a.update(&world_);
    }
}

void Team::printStatus() const {
    for (const auto &a : agents_) {
        a.printStatus();
    }
    // Mostrar estado global resumido
    const auto &b = world_.getBall();
    std::cout << "Ball pos(" << b.x << "," << b.y << ")\n";
}
