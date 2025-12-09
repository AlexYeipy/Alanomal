# Alanomal — Refactor Semana 3

Este repositorio contiene una refactorización mínima del código de la Semana 3 para acercarlo a la organización típica de proyectos tipo wrighteaglebase: carpetas `include/`, `src/`, `bin/` y `Makefile`.

Estructura propuesta
- include/ : headers (Agent.h, Team.h, World.h)
- src/     : implementaciones (Agent.cpp, Team.cpp, World.cpp, main.cpp)
- bin/     : ejecutable (generado por Makefile)
- Makefile : para compilar con `make`

Cómo compilar
1. make
2. ./bin/alanomal

Qué contiene
- Agent: clase que contiene estado y comportamiento por tick. Aquí es donde debes mover la lógica de `funciones.cpp` y `structs.h`.
- World: estado global (por ejemplo, posición de la bola).
- Team: contenedor de agentes y responsable de iterar sus updates.
- main: bucle principal de la simulación (ticks) y ejecución.

Sugerencias para integrar tu código existente
- Abre `Semana 3/funciones.cpp` y `Semana 3/funciones.h`, localiza las funciones que actualizan la posición y decisión de los agentes y muévelas a `Agent::update()` o a métodos privados de `Agent`.
- Mapea los structs de `Semana 3/structs.h` a `Semana 4/include/Agent.h` o crea `Semana 4/include/Structs.h`.
- Conserva la lógica de lectura/entrada si tienes interacción por consola; adapta los I/O al nuevo main.

Próximos pasos recomendados
- Reescribir las funciones de decisión de agentes dentro de Agent::update.
- Crear una clase InputManager si tu proyecto lee configuraciones/archivos.
- Añadir tests o modos de ejecución (por ejemplo: `./bin/alanomal --ticks 100`).
