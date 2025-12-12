/**
 * @file Funciones.cpp
 * @brief Implementación principal de lógica del agente de fútbol robótico
 *
 * Contiene las funciones para procesamiento de percepciones, toma de decisiones
 * y generación de comandos de movimiento para el simulador RCSS.
 */

#include <MinimalSocket/udp/UdpSocket.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <cmath>
#include "structs.h"
#include "utilities.h"
#include "datos_area.h"
#include "Funciones.h"

/**
 * @brief Procesa mensaje inicial y posiciona al jugador
 *
 * @param str Mensaje de inicialización del servidor
 * @param udp_socket Socket UDP para comunicación
 * @param recep Dirección del servidor
 * @param datos Estructura de datos del juego
 */
void sendInitialMoveMessage(const string &str, MinimalSocket::udp::Udp<true> &udp_socket,
                     MinimalSocket::Address const &recep, Game_data &datos)
{
    struct Posicion { int x; int y; };
    Posicion pos;
    vector<string> player_ = separate_string_separator(str, ' '); // Ejemplo: "(init l 2 ...)"

    if (player_.at(0) != "(init")
        throw runtime_error("Invalid initial message");

    datos.jugador.lado_campo = player_.at(1);
    datos.jugador.jugador_numero = player_[2];

    // Asigna posiciones según número de jugador (formación 4-3-3)
    switch (stoi(player_[2]) - 1)
    {
    case 0:  pos.x = -50; pos.y = 0; break;   // Portero
    case 1:  pos.x = -35; pos.y = 20; break;  // Lateral derecho
    case 2:  pos.x = -35; pos.y = 5; break;   // Central derecho
    case 3:  pos.x = -35; pos.y = -5; break;    // Central izquierdo
    case 4:  pos.x = -35; pos.y = -20; break;   // Lateral izquierdo
    case 5:  pos.x = -15; pos.y = -10; break;   // Mediocentro derecho
    case 6:  pos.x = -15; pos.y = 10; break;    // Mediocentro izquierdo
    case 7:  pos.x = -12; pos.y = -2; break;   // Mediocentro central
    case 8:  pos.x = -1;  pos.y = 15; break;   // Delantero derecho
    case 9:  pos.x = -1;  pos.y = -15; break;  // Delantero izquierdo
    case 10: pos.x = -5;  pos.y = 10; break;   // Delantero centro
    }

    auto moveCommand = "(move " + to_string(pos.x) + " " + to_string(pos.y) + ")";
    udp_socket.sendTo(moveCommand, recep);
}

/**
 * @brief Devuelve comando para posición inicial del jugador
 *
 * @param datos Estructura de datos del juego
 * @return Comando de movimiento inicial
 */
string posicion_inicial(Game_data &datos)
{
    struct Posicion { int x; int y; };
    Posicion pos;

    switch (stoi(datos.jugador.jugador_numero) - 1)
    {
    case 0:  pos.x = -50; pos.y = 0; break;   // Portero
    case 1:  pos.x = -35; pos.y = 20; break;  // Lateral derecho
    case 2:  pos.x = -35; pos.y = 5; break;   // Central derecho
    case 3:  pos.x = -35; pos.y = -5; break;    // Central izquierdo
    case 4:  pos.x = -35; pos.y = -20; break;   // Lateral izquierdo
    case 5:  pos.x = -15; pos.y = -10; break;   // Mediocentro derecho
    case 6:  pos.x = -15; pos.y = 10; break;    // Mediocentro izquierdo
    case 7:  pos.x = -12; pos.y = -2; break;   // Mediocentro central
    case 8:  pos.x = -1;  pos.y = 15; break;   // Delantero derecho
    case 9:  pos.x = -1;  pos.y = -15; break;  // Delantero izquierdo
    case 10: pos.x = -5;  pos.y = 10; break;   // Delantero centro
    }

    return "(move " + to_string(pos.x) + " " + to_string(pos.y) + ")";
}

/**
 * @brief Procesa información del balón desde mensajes "see"
 *
 * @param message Mensaje de percepción
 * @param datos Estructura de datos del juego
 * @return Game_data actualizada con información del balón
 */
Game_data handle_ball(string const &message, Game_data &datos)
{
    vector<string> vector_mensaje = separate_string(message);
    datos.ball.veo_balon = false;

    for (auto const &v : vector_mensaje)
    {
        if (v.find("(b)") != -1)
        {
            vector<string> vec = separate_string_separator(v, ' ');
            if (vec.size() >= 3)
            {
                datos.ball.balon_distancia = vec.at(1);
                datos.ball.balon_direccion = vec.at(2);
                datos.ball.veo_balon = true;
            }
        }
    }

    if (!datos.ball.veo_balon)
    {
        datos.ball.balon_distancia = "999.0";
        datos.ball.balon_direccion = "999.0";
    }

    return datos;
}

/**
 * @brief Procesa información de porterías desde mensajes "see"
 *
 * @param message Mensaje de percepción
 * @param datos Estructura de datos del juego
 * @return Game_data actualizada con información de porterías
 */
Game_data handle_porteria(string const &message, Game_data &datos)
{
    // Inicializa valores por defecto
    datos.porteria.veo_porteria_contraria = false;
    datos.porteria.veo_porteria_propia = false;
    datos.porteria.palo_abajo_distancia = "999.0";
    datos.porteria.palo_abajo_direccion = "999.0";
    datos.porteria.centro_distancia = "999.0";
    datos.porteria.centro_direccion = "999.0";

    vector<string> vector_mensaje = separate_string(message);

    for (auto const &v : vector_mensaje)
    {
        // Portería derecha (contraria si jugamos izquierda, propia si jugamos derecha)
        if ((v.find("(f g r b)") != -1) && datos.jugador.lado_campo == "l")
        {
            vector<string> vector_palo = separate_string_separator(v, ' ');
            if (vector_palo.size() >= 6)
            {
                datos.porteria.palo_abajo_distancia = vector_palo.at(4);
                datos.porteria.palo_abajo_direccion = vector_palo.at(5);
            }
        }
        else if ((v.find("(f g r b)") != -1) && datos.jugador.lado_campo == "r")
        {
            datos.porteria.veo_porteria_propia = true;
        }

        if ((v.find("(g r)") != -1) && datos.jugador.lado_campo == "l")
        {
            datos.porteria.veo_porteria_contraria = true;
            vector<string> vector_porteria = separate_string_separator(v, ' ');
            if (vector_porteria.size() >= 4)
            {
                datos.porteria.centro_distancia = vector_porteria.at(2);
                datos.porteria.centro_direccion = vector_porteria.at(3);
            }
        }

        // Portería izquierda (contraria si jugamos derecha, propia si jugamos izquierda)
        if ((v.find("(f g l b)") != -1) && datos.jugador.lado_campo == "r")
        {
            vector<string> vector_palo = separate_string_separator(v, ' ');
            if (vector_palo.size() >= 6)
            {
                datos.porteria.palo_arriba_distancia = vector_palo.at(4);
                datos.porteria.palo_arriba_direccion = vector_palo.at(5);
            }
        }
    }

    return datos;
}

/**
 * @brief Procesa información de jugadores visibles
 *
 * @param message Mensaje de percepción
 * @param datos Estructura de datos del juego
 * @return Game_data actualizada con información de jugadores
 */
Game_data handle_jugadores_vistos(string const &message, Game_data &datos)
{
    datos.jugadores_vistos.veo_equipo = false;
    datos.jugadores_vistos.jugadores.clear();

    vector<string> vector_mensaje = separate_string(message);
    vector<vector<string>> vector_jugadores_cerca;
    vector<vector<string>> vector_contrarios;

    string encontrar_jugador_nuestro_equipo = "(p \"" + datos.nombre_equipo + "\" ";

    for (auto const &v : vector_mensaje)
    {
        if (v.find(encontrar_jugador_nuestro_equipo) != -1)
        {
            vector<string> vector_jugador = separate_string_separator(v, ' ');
            if (vector_jugador.size() >= 5)
            {
                vector_jugador.at(2).pop_back();
                vector_jugadores_cerca.push_back({vector_jugador.at(2), vector_jugador.at(3), vector_jugador.at(4)});
                datos.jugadores_vistos.veo_equipo = true;
            }
        }
        else if (v.find("(p \"") != -1)
        {
            vector<string> contrarios = separate_string_separator(v, ' ');
            if (contrarios.at(1) != datos.jugador.nombre_equipo && contrarios.size() >= 5)
            {
                contrarios.at(2).pop_back();
                if (contrarios.at(2) < "12" && contrarios.at(2) > "0")
                {
                    vector_contrarios.push_back({contrarios.at(2), contrarios.at(3), contrarios.at(4)});
                    datos.jugadores_vistos.veo_equipo_contrario = true;
                }
            }
        }
    }

    if (datos.jugadores_vistos.veo_equipo)
        datos.jugadores_vistos.jugadores = vector_jugadores_cerca;
    if (datos.jugadores_vistos.veo_equipo_contrario)
        datos.jugadores_vistos.jugadores_contrario = vector_contrarios;

    return datos;
}

/**
 * @brief Verifica si el jugador tiene posesión del balón
 *
 * @param datos Estructura de datos del juego
 * @return true si tiene el balón, false en caso contrario
 */
bool tengo_balon(Game_data const &datos)
{
    return (stod(datos.ball.balon_distancia) <= 1 && datos.ball.balon_distancia != "999.0");
}

/**
 * @brief Determina si el jugador debe ir hacia el balón
 *
 * @param datos Estructura de datos del juego
 * @return true si debe ir al balón, false en caso contrario
 */
bool voy_balon(Game_data &datos)
{
    // Devolver true si tenemos que ir a por el balon, falso si no

    // Variables de jugador a balon
    auto db = stod(datos.ball.balon_distancia);
    auto thetab = -(M_PI / 180) * stod(datos.ball.balon_direccion);

    vector<vector<string>> jugadores = datos.jugadores_vistos.jugadores;

    int contador{0};
    if (jugadores.size() > 0)
    {
        for (auto const jugador : jugadores)
        {
            // variables 3º jugador a balon
            auto dp = stod(jugador.at(1));
            auto thetap = -(M_PI / 180) * stod(jugador.at(2));

            auto d = sqrt(db * db + dp * dp - 2 * db * dp * cos(thetab - thetap));

            // Si mi distancia al balon es mayor devolvemos false
            // Si mi distancia al balon es menor comparada con 3º jugador devolvemos true
            // Hacemos return si encontramos algun true

            if (db > d)
            {
                contador++;
            }
        }
    }
    if ((datos.jugador.jugador_numero == "4" || datos.jugador.jugador_numero == "2" || datos.jugador.jugador_numero == "5") && stof(datos.ball.balon_distancia) < 10)
    {
        return true;
    }

    if (contador > (int)(jugadores.size() / 2))
        return false;
    // Si no vemos a nadie cerca, tambien voy
    if (jugadores.size() == 0)
    {
        // Si no vemos a jugadores cerca deberiamos de ir a por el balon, por ello debemos devolver true
        // pero los jugadores tienden a ir demasiado al balon, por lo que agregamos una probabilidad de no ir
        bool random_boolean = rand() % 100 < 80;
        return random_boolean;
    }
    return true;
}

/**
 * @brief Detecta si hay enemigos cercanos (<10m)
 *
 * @param datos Estructura de datos del juego
 * @return true si hay enemigos cercanos, false en caso contrario
 */
bool cerca_enemigo(Game_data &datos)
{
    float distancia{1000};
    for (auto const &jugador : datos.jugadores_vistos.jugadores_contrario)
    {
        if (datos.jugadores_vistos.veo_equipo_contrario)
        {
            if (stof(jugador.at(1)) < distancia)
                distancia = stof(jugador.at(1));
        }
    }
    if (distancia < 10)
        return true;
    else
        return false;
}

/**
 * @brief Encuentra el mejor compañero para pasar
 *
 * @param datos Estructura de datos del juego
 */
void jugador_cerca_pase(Game_data &datos)
{
    // Guardar el numero, distancia y direccion del jugador mas cercano a ti y con numero mayor
    // Se guarda en estructura nueva Jugador Cercano
    if (!cerca_enemigo(datos))
    {
        vector<vector<string>> jugadoresCerca = datos.jugadores_vistos.jugadores;
        vector<vector<string>> jugadoresNumeroMayor;

        // Evaluamos todos los jugadores
        for (auto &jugador : jugadoresCerca)
        {
            // aceptamos pases de menor numero si somos delanteros a centrocampistas
            bool somosDelanteros = (datos.jugador.jugador_numero == "11" || datos.jugador.jugador_numero == "10" ||
                                    datos.jugador.jugador_numero == "9");

            if (somosDelanteros && stod(jugador.at(0)) >= 5)
            {
                jugadoresNumeroMayor.push_back(jugador);
            }
            else if (stod(jugador.at(0)) > stod(datos.jugador.jugador_numero) && stod(jugador.at(1)) > 8)
            {
                jugadoresNumeroMayor.push_back(jugador);
            }
        }

        if (jugadoresNumeroMayor.size() != 0)
        {
            // Ordenamos el vector juagdoresNumeroMayor por menor distancia
            sort(jugadoresNumeroMayor.begin(), jugadoresNumeroMayor.end(), [](const vector<string> &a, const vector<string> &b)
                 { return stod(a.at(1)) > stod(b.at(1)); });//cambio <
            if (datos.jugador.jugador_numero == "1" || datos.jugador.jugador_numero == "2" || datos.jugador.jugador_numero == "3" ||
                datos.jugador.jugador_numero == "4" || datos.jugador.jugador_numero == "5")
            {
                sort(jugadoresNumeroMayor.begin(), jugadoresNumeroMayor.end(), [](const vector<string> &a, const vector<string> &b)
                     { return stod(a.at(1)) > stod(b.at(1)); });
            }
            if(datos.jugador.jugador_numero == "6" || datos.jugador.jugador_numero == "7" || datos.jugador.jugador_numero == "8")
            {
                sort(jugadoresNumeroMayor.begin(), jugadoresNumeroMayor.end(), [](const vector<string> &a, const vector<string> &b)
                     { return stod(a.at(0)) > stod(b.at(0)); });
            }
            // Si hay jugadores con numero mayor

            datos.jugadorCerca.hayJugador = true;
            datos.jugadorCerca.distancia = jugadoresNumeroMayor.at(0).at(1);
            datos.jugadorCerca.direccion = jugadoresNumeroMayor.at(0).at(2);
            datos.jugadorCerca.numero_jugador = jugadoresNumeroMayor.at(0).at(0);
        }
        else
        {
            datos.jugadorCerca.hayJugador = false;
        }
    }
    else
    {
        vector<vector<string>> jugadoresCerca = datos.jugadores_vistos.jugadores;
        vector<vector<string>> jugadoresNumeroMayor;

        // Evaluamos todos los jugadores
        for (auto &jugador : jugadoresCerca)
        {
            // aceptamos pases de menor numero si somos delanteros a centrocampistas
            bool somosDelanteros = (datos.jugador.jugador_numero == "11" || datos.jugador.jugador_numero == "10" ||
                                    datos.jugador.jugador_numero == "9");

            if (somosDelanteros && stod(jugador.at(0)) >= 5) // jugador distancia en mayor de 11
            {
                jugadoresNumeroMayor.push_back(jugador);
            }
            else if (stod(jugador.at(0)) > stod(datos.jugador.jugador_numero) && stod(jugador.at(1)) > 8)
            {
                jugadoresNumeroMayor.push_back(jugador);
            }
        }

        if (jugadoresNumeroMayor.size() != 0)
        {
            // Ordenamos el vector juagdoresNumeroMayor por mayor distancia
            sort(jugadoresNumeroMayor.begin(), jugadoresNumeroMayor.end(), [](const vector<string> &a, const vector<string> &b)
                 { return stod(a.at(1)) > stod(b.at(1)); });
            if (datos.jugador.jugador_numero == "1" || datos.jugador.jugador_numero == "2" || datos.jugador.jugador_numero == "3" ||
                datos.jugador.jugador_numero == "4" || datos.jugador.jugador_numero == "5")
            {
                sort(jugadoresNumeroMayor.begin(), jugadoresNumeroMayor.end(), [](const vector<string> &a, const vector<string> &b)
                     { return stod(a.at(1)) > stod(b.at(1)); });
            }
            if(datos.jugador.jugador_numero == "6" || datos.jugador.jugador_numero == "7" || datos.jugador.jugador_numero == "8")
            {
                sort(jugadoresNumeroMayor.begin(), jugadoresNumeroMayor.end(), [](const vector<string> &a, const vector<string> &b)
                     { return stod(a.at(0)) > stod(b.at(0)); });
            }
            // Si hay jugadores con numero mayor
            // Guardamos el jugador con mayor numero
            // for (auto &j : jugadoresNumeroMayor)
            //     cout << "-> " << j.at(1) << endl;
            // cout << "fin" << endl;
            datos.jugadorCerca.hayJugador = true;
            datos.jugadorCerca.distancia = jugadoresNumeroMayor.at(0).at(1);
            datos.jugadorCerca.direccion = jugadoresNumeroMayor.at(0).at(2);
            datos.jugadorCerca.numero_jugador = jugadoresNumeroMayor.at(0).at(0);
        }
        else
        {
            datos.jugadorCerca.hayJugador = false;
        }
    }
}

/**
 * @brief Genera comando de pase al mejor compañero disponible
 *
 * @param datos Estructura de datos del juego
 * @return Comando de pase o patada segura
 */
string pase(Game_data &datos)
{
    jugador_cerca_pase(datos);
    // Si vemos a un jugador cerca con un numero mayor al nuestro
    bool hayJugadorMasCerca = datos.jugadorCerca.hayJugador;

    if (hayJugadorMasCerca)
    {
        int potencia;
        // Hay jugador cerca, calculamos potencia necesaria del pase
        if (stof(datos.jugadorCerca.distancia) >= 20)
            potencia = static_cast<int>(stof(datos.jugadorCerca.distancia) * 2.7);
        else if (stof(datos.jugadorCerca.distancia) < 20 && stof(datos.jugadorCerca.distancia) > 10)
            potencia = static_cast<int>(stof(datos.jugadorCerca.distancia) * 2.4);
        else
        {
            potencia = static_cast<int>(stof(datos.jugadorCerca.distancia) * 2.3);
        }

        if (potencia > 100)
        {
            return "(kick 100 " + datos.jugadorCerca.direccion + ")";
        }
        else
        {
            return "(kick " + to_string(potencia) + " " + datos.jugadorCerca.direccion + ")";
        }
    }
    else
    {
        // No hay nadie cerca, pase corto hacia atras para girar la cabeza
        return "(kick 12 130)";
    }
}

/**
 * @brief Procesa mensajes del servidor y actualiza estado del juego
 *
 * @param mensaje Mensaje recibido del servidor
 * @param datos Estructura de datos del juego
 */
void send_message_funtion(string const &mensaje, Game_data &datos)
{
    if (mensaje.find("hear") != -1)
    {
        vector<string> vector_mensaje_1 = separate_string(mensaje); // ("init r 1 before_kick_off")
        vector_mensaje_1 = separate_string_separator(vector_mensaje_1.at(0), ' ');
        if (vector_mensaje_1.at(0) != "hear")
        {
            throw runtime_error("error en el mensaje hear");
            return;
        }
        datos.evento_anterior = datos.evento;
        datos.evento = vector_mensaje_1.at(3);
    }

    if (mensaje.find("see") != -1)
    {
        vector<string> vector_mensaje = separate_string(mensaje); // ("see ((b) 12 12)")
        handle_ball(vector_mensaje.at(0), datos);
        handle_porteria(vector_mensaje.at(0), datos);
        handle_jugadores_vistos(vector_mensaje.at(0), datos);
    }
}

/**
 * @brief Genera comportamiento ofensivo según distancia al balón
 *
 * @param datos Estructura de datos del juego
 * @return Comando de ataque o "0" si no aplica
 */
string ataque(Game_data &datos)
{
    string resultado;

    // Si somos el jugador que va al balon, y su distancia es < 25
    if ((datos.jugador.jugador_numero == "10" || datos.jugador.jugador_numero == "11" || datos.jugador.jugador_numero == "9") &&
        stod(datos.ball.balon_distancia) <= 30 && stod(datos.ball.balon_distancia) > 1)
    {
        return "(dash 100 " + datos.ball.balon_direccion + ")";
    }

    // Si somos el jugador que va al balon, y su distancia es < 25
    if (voy_balon(datos) && stod(datos.ball.balon_distancia) <= 20 && stod(datos.ball.balon_distancia) > 1)
    {
        return "(dash 100 " + datos.ball.balon_direccion + ")";
    }

    // Si somos el 11, hay corner y la distancia al balon > 1
    bool corner = (datos.evento.find("kick_off_l") == -1 && datos.jugador.jugador_numero == "11" && datos.jugador.lado_campo == "l") ||
                  (datos.evento.find("kick_off_r") == -1 && datos.jugador.jugador_numero == "11" && datos.jugador.lado_campo == "r");

    if (datos.jugador.jugador_numero == "11" && stod(datos.ball.balon_distancia) > 1 && corner)
    {
        return "(dash 100 " + datos.ball.balon_direccion + ")";
    }

    // Somos cualquier jugador y la distancia al balon >25
    if (datos.jugador.jugador_numero != "1" && stod(datos.ball.balon_distancia) > 20 && stod(datos.ball.balon_distancia) < 40)
    {
        return "(dash 35 " + datos.ball.balon_direccion + ")";
    }

    // Somos cualquier jugador y la distancia al balon >25
    if (datos.jugador.jugador_numero != "1" && stod(datos.ball.balon_distancia) >= 40)
    {
        return "(dash 55 " + datos.ball.balon_direccion + ")";
    }

    // Somos portero y nos atacan con el balon, salimos
    if (datos.jugador.jugador_numero == "1" && stod(datos.ball.balon_distancia) > 2 && stod(datos.ball.balon_distancia) <= 4)
    {
        return "(dash 10 " + datos.ball.balon_direccion + ")";
    }
    return "0";
}

/**
 * @brief Genera comando de disparo a portería
 *
 * @param datos Estructura de datos del juego
 * @return Comando de disparo con ángulo aleatorio
 */
string disparo(Game_data &datos)
{
    float angulo = stof(datos.porteria.centro_direccion);

    // Añade variación aleatoria al disparo
    if (rand() % 2 == 0)
        angulo += (rand() % 11);  // Desvío a la derecha
    else
        angulo -= (rand() % 11);  // Desvío a la izquierda

    return "(kick 100 " + to_string(angulo) + ")";
}

/**
 * @brief Función principal de toma de decisiones
 *
 * Coordina todas las decisiones tácticas del jugador
 *
 * @param datos Estructura de datos del juego
 * @return Comando a enviar al servidor
 */
string sendMessage(Game_data &datos)
{
    jugador_cerca_pase(datos);

    string resultado;

    // 1. POSICIONAMIENTO INICIAL
    // Inicio de posiciones
    bool checkStart = (datos.evento.find("goal_l") != -1 || datos.evento.find("goal_r") != -1 ||
                       datos.evento.find("half_time") != -1 || datos.evento.find("extra_half_time") != -1) ||
                      datos.evento.find("before_kick_off") != -1;

    if (checkStart)
    {
        resultado = posicion_inicial(datos);
        if ((datos.evento.find("goal_l") != -1 && datos.lado_campo == "r") || (datos.evento.find("goal_r") != -1 && datos.lado_campo == "l"))
        {
            datos.flag_kick_off = true;
        }
        else if (datos.evento.find("half_time") != -1 || datos.evento.find("extra_half_time") != -1)
        {
            datos.flag_kick_off = true;
        }
        return resultado;
    }

    // 2. BÚSQUEDA DEL BALÓN
    // Si no vemos balon giramos mucho
    if (!datos.ball.veo_balon)
    {
        return "(turn 70)";
    }

    // si vemos mal el balon giramos poco
    if (abs(stod(datos.ball.balon_direccion)) > 10) // BALON A LA DERECHA, GIRA DERECHA
    {
        return "(turn " + datos.ball.balon_direccion + ")";
    }

    // 3. SAQUES ESPECIALES
    // FALTAS
    // Hay saque de balon nuestro (kisk_off_side_l y somos l)
    if ((datos.evento == "kick_off_l" && datos.jugador.lado_campo == "l") ||
        (datos.evento == "kick_off_r" && datos.jugador.lado_campo == "r"))
    {
        if (tengo_balon(datos))
            return "(kick 100 70)";
    }

    // CORNERS
    bool DelanterosCorner = (datos.jugador.jugador_numero == "11" || datos.jugador.jugador_numero == "10" || datos.jugador.jugador_numero == "9");
    if ((datos.evento.find("indirect_free_kick_l") != -1 && datos.jugador.lado_campo == "l" && DelanterosCorner) ||
        (datos.evento.find("indirect_free_kick_r") != -1 && datos.jugador.lado_campo == "r" && DelanterosCorner) ||
        (datos.evento.find("corner_kick_l") != -1 && datos.jugador.lado_campo == "l" && DelanterosCorner) ||
        (datos.evento.find("corner_kick_r") != -1 && datos.jugador.lado_campo == "r" && DelanterosCorner))
    {
        if (!tengo_balon(datos))
            return "(dash 100 " + datos.ball.balon_direccion + ")";
        else if (tengo_balon(datos))
            if(datos.porteria.veo_porteria_contraria)
            {
                resultado = disparo(datos);
                return resultado;
            }
            else
            {
                resultado = pase(datos);
            }
        if (resultado == "(kick 10 130)")
        {
            return "(kick 70 120)";
        }
        return resultado;
    }

    // Si somos el 11 y hay penaltie
    if (datos.jugador.jugador_numero == "11" && ((datos.evento.find("penalty_kick_l") != -1) && datos.lado_campo == "l" ||
                                                 (datos.evento.find("penalty_kick_r") != -1) && datos.lado_campo == "r"))
    {
        if (tengo_balon(datos))
        {
            resultado = disparo(datos);
            return resultado;
        }
    }

    // Si somos el portero y hay saque de porteria (d<1)
    bool saquePortero = (datos.evento.find("free_kick_") != -1 && datos.jugador.jugador_numero == "1" && stod(datos.ball.balon_distancia) < 1);
    if (saquePortero)
    {
        resultado = pase(datos);
        if (resultado == "(kick 10 130)")
        {
            resultado = "(kick 100 10)";
            return resultado;
        }
        return resultado;
    }

    // 4. COMPORTAMIENTO EN ÁREA ASIGNADA

    if (comprobar_area(datos) || voy_balon(datos))
    {
        resultado = ataque(datos);
        if (resultado != "0")
        {
            return resultado;
        }
    }
    else
    {
        if (datos.jugador.jugador_numero == "1")
        {
            return "";
        }
        else
        {
            // Mejor tener juego agresivo a bugs por ir hacia atras
            return "(dash 20 180)";
        }
    }

    // 5. COMPORTAMIENTO DEL PORTERO

    // if (datos.jugador.saque_puerta && datos.jugador.jugador_numero == "1" && stod(datos.ball.balon_distancia) < 1)
    if(datos.jugador.jugador_numero == "1" && stod(datos.ball.balon_distancia) <= 2 && stod(datos.ball.balon_distancia) >= 0.5)
    {
        return "(catch " + datos.ball.balon_direccion + ")";
    }

    if (datos.jugador.jugador_numero == "1" && stod(datos.ball.balon_distancia) < 0.5)
    {
        // datos.jugador.saque_puerta = false;
        if (datos.jugadorCerca.hayJugador && stof(datos.jugadorCerca.distancia) > 10 && !datos.jugadores_vistos.veo_equipo_contrario)
        {
            resultado = pase(datos);
            if (resultado == "(kick 10 130)")
            {
                if (datos.porteria.veo_porteria_propia)
                {
                    return "(kick 100 120)";
                }
                else
                {
                    return "(kick 100 20)";
                }
            }
            return resultado;
        }
        if (datos.porteria.veo_porteria_propia)
        {
            return "(kick 100 120)";
        }
        else
        {
            return "(kick 100 40)";
        }
    }


    // 6. DISPARO A PORTERÍA
    // Si podemos TIRAR A PORTERIA
    if (stod(datos.ball.balon_distancia) < 1 && stod(datos.porteria.centro_distancia) <= 38)
    {
        resultado = disparo(datos);
        return resultado;
    }

    // 7. PASE CON POSESIÓN
    // Si TENEMOS BALON, valoramos distintas opciones
    if (stod(datos.ball.balon_distancia) < 1)
    {
        jugador_cerca_pase(datos);
        // Si no somos delanteros y vemos a compañero
        bool Delantero = (datos.jugador.jugador_numero == "11" || datos.jugador.jugador_numero == "10");

        // Comportamiento defensas
        bool defensa = (datos.jugador.jugador_numero == "2" || datos.jugador.jugador_numero == "3" || datos.jugador.jugador_numero == "4" || datos.jugador.jugador_numero == "5");
        if (defensa && !datos.porteria.veo_porteria_propia && datos.jugadorCerca.hayJugador && stof(datos.jugadorCerca.distancia) < 20)
        {
            if (datos.porteria.veo_porteria_contraria)
            {
                int direccion;
                if((direccion = rand() % 3) == 0)
                    return "(kick 100 " + datos.porteria.centro_direccion + ")";
                else if( direccion == 1)
                    return "(kick 100 " + datos.porteria.palo_abajo_direccion + ")";
                else
                    return "(kick 100 " + datos.porteria.palo_arriba_direccion + ")";
            }
            else
            {
                return "(kick 100 " + datos.ball.balon_direccion + ")";
            }
        }
        else if (datos.jugadorCerca.hayJugador && stof(datos.jugadorCerca.distancia) > 15)
        {
            resultado = pase(datos);
            return resultado;
        }

        if (!Delantero && datos.jugadorCerca.hayJugador)
        {
            resultado = pase(datos);
            return resultado;
        }
        if (datos.porteria.veo_porteria_contraria && datos.jugador.jugador_numero != "1")
        {
            return "(kick 25 " + datos.porteria.centro_direccion + ")";
        }

        if (datos.jugador.jugador_numero != "1")
        {
            if (datos.evento.find("kick_in_") != -1 || datos.evento.find("corner_kick_") != -1)
            {
                if (datos.jugadorCerca.hayJugador)
                {
                    resultado = pase(datos);
                    return resultado;
                }
                else
                {
                    return ("(kick 60 180)");
                }
            } // REGATE
            if (datos.lado_campo == "l")
                return "(kick 10 -70)";
            else
            {
                return "(kick 10 70)";
            }
        }
    }

    if (datos.jugador.jugador_numero == "1")
    {
        return "";
    }

    // 8. COMPORTAMIENTO POR DEFECTO
    if (tengo_balon(datos))
    {
        return "(kick 20" + datos.porteria.centro_direccion + ")";
    }

    return "(dash 1 " + datos.ball.balon_direccion + ")";
}
