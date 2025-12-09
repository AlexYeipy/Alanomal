#include "Agent.h"
#include "World.h"
#include <iostream>

Agent::Agent(int id) : id_(id), pos_{0.0, 0.0}, energy_(100.0) {}

// Actualización simple por tick. Aquí puedes reemplazar la lógica por la de tus funciones.cpp
void Agent::update(const World* /*world*/) {
    moveSimple();
    consumeEnergy();
}

void Agent::moveSimple() {
    // Movimiento de ejemplo: cada agente se desplaza de forma determinista
    pos_.x += 0.6;
    pos_.y += 0.2 * (id_ % 3);
}

void Agent::consumeEnergy() {
    energy_ -= 0.8;
    if (energy_ < 0.0) energy_ = 0.0;
}

void Agent::printStatus() const {
    std::cout << "Agent[" << id_ << "] pos(" << pos_.x << "," << pos_.y
              << ") energy=" << energy_ << "\n";
}
