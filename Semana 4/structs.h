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
    string team_name;       // Nombre del equipo al que pertenece
    string playmode;        // Modo de juego actual (before_kick_off, play_on, etc.)
    string side;            // Lado del campo: "l" (izquierda) o "r" (derecha)
    bool see_ball;          // Indica si el jugador puede ver la pelota
    bool see_goal;          // Indica si el jugador puede ver la portería
    int unum;               // Número único del jugador (1-11)
    double x;               // Coordenada X de la posición del jugador
    double y;               // Coordenada Y de la posición del jugador
    double home_x;          // Posición X inicial (su zona)
    double home_y;          // Posición Y inicial (su zona)
    int flags_seen;
    string zone_name;
    bool seeing_zone;
    double range;
    bool see_own_goal;
    bool see_opponent_goal;
    string rol;
    bool see_left_top;
    float stamina;
    bool in_zone;
    bool OutOfField;
    double distancia_a_zona;
    double angle;
    string playmode_prev;
    string jugadorMarcaGol;
    bool posicion_sacar;
};

/**
 * Estructura que representa la pelota del juego
 * Almacena información de posición y velocidad
 */
struct Ball
{
    string x;   // Coordenada X de la posición de la pelota
    string y;   // Coordenada Y de la posición de la pelota
    string vx;  // Componente X de la velocidad de la pelota
    string vy;  // Componente Y de la velocidad de la pelota
    double distance;
    double angle;
};


struct Goal
{
    std::string x;
    std::string y;
    std::string side;
    float distance;
    double angle;
};

struct JugadorCercano
{
    std::string x;
    std::string y;
    std::string distancia;
    std::string unum;
    std::string nombreEquipo;
    std::string dorsal;
    float distance;
    float angle;
};

struct JugadoresAlrededor
{
    std::vector<int> x;
    std::vector<int> y;
    std::vector<int> distancia;
    std::vector<int> unum;
    float distance;
};

struct Field
{
    vector<float> flag_center;
    float flag_center_distance;
    vector<float> flag_center_top;
    float flag_center_top_distance;
    float flag_center_top_angle;
    vector<float> flag_center_bottom;
    float flag_center_bottom_distance;
    vector<float> flag_left_top;
    float flag_left_top_distance;
    float flag_left_top_angle;
    vector<float> flag_left_bottom;
    float flag_left_bottom_distance;
    vector<float> flag_right_top;
    float flag_right_top_distance;
    vector<float> flag_right_bottom;
    float flag_right_bottom_distance;

    vector<float> flag_penalty_left_top;
    float flag_penalty_left_top_distance;
    vector<float> flag_penalty_left_center;
    float flag_penalty_left_center_distance;
    vector<float> flag_penalty_left_bottom;
    float flag_penalty_left_bottom_distance;
    vector<float> flag_penalty_right_top;
    float flag_penalty_right_top_distance;
    vector<float> flag_penalty_right_center;
    float flag_penalty_right_center_distance;
    vector<float> flag_penalty_right_bottom;
    float flag_penalty_right_bottom_distance;

    vector<float> flag_goal_left_top;
    float flag_goal_left_top_distance;
    vector<float> flag_goal_left_bottom;
    float flag_goal_left_bottom_distance;
    vector<float> flag_goal_right_top;
    float flag_goal_right_top_distance;
    vector<float> flag_goal_right_bottom;
    float flag_goal_right_bottom_distance;
};

struct Boundaries
{
    // Goal side boundaries
    vector<double> left_top;
    vector<double> left_bot;
    vector<double> right_top;
    vector<double> right_bot;

    // Field boundaries
    vector<double> top_left_50;
    vector<double> top_left_40;
    vector<double> top_left_30;
    vector<double> top_left_20;
    vector<double> top_left_10;
    vector<double> top_0;
    vector<double> top_right_10;
    vector<double> top_right_20;
    vector<double> top_right_30;
    vector<double> top_right_40;
    vector<double> top_right_50;
    vector<double> bot_left_50;
    vector<double> bot_left_40;
    vector<double> bot_left_30;
    vector<double> bot_left_20;
    vector<double> bot_left_10;
    vector<double> bot_0;
    vector<double> bot_right_10;
    vector<double> bot_right_20;
    vector<double> bot_right_30;
    vector<double> bot_right_40;
    vector<double> bot_right_50;
};


#endif // ESTRUCTURAS_H
