#ifndef TEAM_H
#define TEAM_H

#include <vector>
#include "Agent.h"
#include "World.h"

class Team {
public:
    Team(int numAgents, World& worldRef);

    void update();       // actualizar todos los agentes (un tick)
    void printStatus() const;

private:
    World& world_;
    std::vector<Agent> agents_;
};

#endif // TEAM_H
