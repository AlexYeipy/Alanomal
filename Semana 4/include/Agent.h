#ifndef AGENT_H
#define AGENT_H

#include <string>

struct Position {
    double x = 0.0;
    double y = 0.0;
};

class World; // forward

class Agent {
public:
    Agent(int id);

    // Actualiza el agente por un tick (usa información del world si es necesario)
    void update(const World* world = nullptr);

    // Imprime estado (para debugging / salida)
    void printStatus() const;

    int getId() const { return id_; }

private:
    int id_;
    Position pos_;
    double energy_;

    // Métodos privados que pueden mapear a tus funciones de Semana 3
    void moveSimple();
    void consumeEnergy();
};

#endif // AGENT_H
