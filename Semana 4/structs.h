#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

#include <string>
#include <iostream>
#include <vector>

using namespace std;

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
    string team_name;   // Nombre del equipo al que pertenece
    string playmode;    // Modo de juego actual (before_kick_off, play_on, etc.)
    string side;        // Lado del campo: "l" (izquierda) o "r" (derecha)
    bool see_ball;      // Indica si el jugador puede ver la pelota
    bool see_goal;      // Indica si el jugador puede ver la portería
    int unum;           // Número único del jugador (1-11)
    double x;           // Coordenada X de la posición del jugador
    double y;           // Coordenada Y de la posición del jugador
    double home_x;      // posición base X
    double home_y;      // posición base Y
};

/**
 * Estructura que representa la pelota del juego
 * Almacena información de posición y velocidad
 */
struct Ball
{
    string x;   // Coordenada X de la posición de la pelota
    string y;   // Coordenada Y de la posición de la pelota
    string distancia;    // Coordenada X de la posición de la pelota
    string angulo;    // Coordenada Y de la posición de la pelota
    string vx;   // Componente X de la velocidad de la pelota
    string xy;   // Componente Y de la velocidad de la pelota
};


struct Goal
{
    string x;
    string y;
    string side;
    float distance;
    double angle;
};

struct JugadorCercano
{
    string x;
    string y;
    string distancia;
    string unum;
    string nombreEquipo;
    string dorsal;
    float distance;
    float angle;
};

struct JugadoresAlrededor
{
    vector<int> x;
    vector<int> y;
    vector<int> distancia;
    vector<int> unum;
    float distance;
};

#endif // ESTRUCTURAS_H


