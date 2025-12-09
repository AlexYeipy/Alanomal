#ifndef WORLD_H
#define WORLD_H

struct Position;

class World {
public:
    World();

    void tick(); // avanza la simulación global
    int getTick() const { return tick_; }

    // Ejemplo de estado global que pueden leer los agentes
    struct Ball {
        double x = 0.0;
        double y = 0.0;
    };
    const Ball& getBall() const { return ball_; }

private:
    int tick_;
    Ball ball_;
};

#endif // WORLD_H
