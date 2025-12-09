#include "World.h"

World::World() : tick_(0), ball_{0.0, 0.0} {}

void World::tick() {
    // Ejemplo simple: mover la bola ligeramente cada tick.
    ball_.x += 0.3;
    ball_.y += 0.1;
    ++tick_;
}
