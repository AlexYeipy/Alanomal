#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

#include <string>
#include <iostream>
#include <vector>

/**
 * Archivo de cabecera que define las estructuras de datos utilizadas
 * en el sistema de fútbol robótico para representar entidades del juego
 */

/**
 * Estructura que representa un jugador en el campo
 * Contiene toda la información de estado y percepción del jugador
 */
struct Player
{
    std::string team_name;    // Nombre del equipo al que pertenece
    std::string playmode;     // Modo de juego actual (before_kick_off, play_on, etc.)
    std::string side;         // Lado del campo: "l" (izquierda) o "r" (derecha)
    bool see_ball;           // Indica si el jugador puede ver la pelota
    bool see_goal;           // Indica si el jugador puede ver la portería
    int unum;                // Número único del jugador (1-11)
    double x;                // Coordenada X de la posición del jugador
    double y;                // Coordenada Y de la posición del jugador
};

/**
* Estructura que representa la pelota
* Almacena información de la posición y la velocidad
*/
struct Ball
{
    std::string distancia;    // Coordenada X de la posición de la pelota
    std::string angulo;    // Coordenada Y de la posición de la pelota
    std::string vx;   // Componente X de la velocidad de la pelota
    std::string xy;   // Componente Y de la velocidad de la pelota
};

#endif // ESTRUCTURAS_H

