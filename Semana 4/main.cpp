#include <iostream>
#include <MinimalSocket/udp/UdpSocket.h>
#include <chrono>
#include <thread>
#include <vector>
#include <sstream>
#include <cmath>

#include "funciones.h"
#include "structs.h"

using namespace std;

/**
 * Programa principal para el cliente de jugador de fútbol robótico
 * Este programa se conecta al servidor de simulador RCSS y controla un jugador
 */
int main(int argc, char *argv[])
{
    // Verificación de argumentos de línea de comandos
    if (argc != 3 && argc != 4)
    {
        cout << "Uso: " << argv[0] << " <nombre-equipo> <puerto> [goalie]" << endl;
        return 1;
    }

    bool soy_portero = (argc == 4 && string(argv[3]) == "goalie");

    // Obtención de parámetros: nombre del equipo y puerto para este jugador
    string team_name = argv[1];
    MinimalSocket::Port this_socket_port = std::stoi(argv[2]);

    cout << "Creando socket UDP..." << endl;

    // Creación del socket UDP IPv6 para comunicación con el servidor
    MinimalSocket::udp::Udp<true> udp_socket(this_socket_port, MinimalSocket::AddressFamily::IP_V6);
    cout << "Socket creado correctamente" << endl;

    // Apertura del socket
    bool success = udp_socket.open();
    if (!success)
    {
        cout << "Error al abrir el socket" << endl;
        return 1;
    }

    // Configuración de dirección del servidor (localhost puerto 6000)
    MinimalSocket::Address other_recipient_udp = MinimalSocket::Address{"127.0.0.1", 6000};

    string init_msg;
    if (soy_portero)
        init_msg = "(init " + team_name + " (version 19) (goalie))";
    else
        init_msg = "(init " + team_name + " (version 19))";

    udp_socket.sendTo(init_msg, other_recipient_udp);
    cout << "Mensaje de inicialización enviado: " << init_msg << endl;

    // Espera y recepción de la respuesta del servidor
    std::size_t message_max_size = 1000000;
    cout << "Esperando mensaje del servidor..." << endl;
    auto received_message = udp_socket.receive(message_max_size);
    std::string received_message_content = received_message->received_message;

    // Actualización de la dirección del servidor con el puerto que respondió
    MinimalSocket::Address other_sender_udp = received_message->sender;
    MinimalSocket::Address server_udp = MinimalSocket::Address{"127.0.0.1", other_sender_udp.getPort()};

    // Creación de objetos para almacenar estados
    Player player{team_name, "before_kick_off", "", false, 0, 0, 0};
    Ball ball{"0", "0", "0", "0"};
    Goal own_goal{"0","0","init",0};
    Goal opponent_goal{"0","0","init",0};
    bool goalie_has_ball = false;
    // Field field;

    // Parseo del mensaje inicial para obtener: lado, número de jugador y modo de juego
    player = parseInitialMessage(received_message_content, player);
    cout << "Número de jugador asignado: " << player.unum << endl;

    // Envío del comando de movimiento inicial a la posición asignada
    sendInitialMoveMessage(player, udp_socket, server_udp);

    // Configurar las porterias
    if (player.side == "r")
    {
        opponent_goal.side = "l";
        own_goal.side = "r";
    }
    else
    {
        opponent_goal.side = "r";
        own_goal.side = "l";
    }

    /**
     * Bucle principal de ejecución del jugador
     * Procesa continuamente los mensajes del servidor y toma decisiones
     */
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        auto received_message = udp_socket.receive(message_max_size);
        std::string received_message_content = received_message->received_message;

        vector<string> parsed_message = separate_string(received_message_content);
        string mySide = player.side;

        
        // ======================================
        // ACTUALIZAR PLAYMODE A PARTIR DE HEAR
        // ======================================
        for (const auto &block : parsed_message)
        {
            // block será algo tipo: "hear 15 referee before_kick_off"
            if (block.rfind("hear ", 0) == 0)  // empieza por "hear "
            {
                vector<string> hear_parts = separate_string_separator(const_cast<string&>(block), " ");

                if (hear_parts.size() >= 4 && hear_parts[2] == "referee")
                {
                    std::string new_mode = hear_parts[3];

                    // Quitar posible ')' final: "before_kick_off)"
                    if (!new_mode.empty() && new_mode.back() == ')')
                        new_mode.pop_back();

                    if (new_mode != player.playmode)
                    {
                        cout << "[REFEREE] playmode cambia de "
                             << player.playmode << " a " << new_mode << endl;
                        player.playmode = new_mode;
                    }
                }
            }
        }

        // ======================================================
        //   FASE INICIAL: before_kick_off
        // ======================================================
        if (player.playmode == "before_kick_off")
        {
            cout << "Partido sin empezar" << endl;
            continue;
        }
    

        // Solo procesamos mensajes de tipo "see"
        if (parsed_message[0].find("see") <= 5)
        {
            vector<string> see_message = separate_string(parsed_message[0]);

            // ========================================
            //   DETECTAR BALÓN
            // ========================================
            size_t ball_pos = 0;
            player.see_ball = false;

            for (size_t i = 0; i < see_message.size(); i++)
            {
                if (see_message[i].find("(b)") <= 5)
                {
                    ball_pos = i;
                    player.see_ball = true;
                    break;
                }
            }

            // ========================================
            //   DETECTAR PORTERÍA (USANDO FLAGS)
            // ========================================
            bool veo_porteria = false;
            double ang_flag_porteria = 0.0;

            for (size_t i = 0; i < see_message.size(); i++)
            {
                if (player.side == "l" && see_message[i].find("(g l)") <= 5)
                {
                    vector<string> f = separate_string_separator(see_message[i], " ");
                    // f = ["(g", "l)", dist, ang, ...]
                    ang_flag_porteria = stod(f[3]); // ÁNGULO YA EN GRADOS
                    veo_porteria = true;
                    break;
                }
                else if (player.side == "r" && see_message[i].find("(g r)") <= 5)
                {
                    vector<string> f = separate_string_separator(see_message[i], " ");
                    ang_flag_porteria = stod(f[3]);
                    veo_porteria = true;
                    break;
                }
            }

            // =======================================
            //   LÓGICA DEL PORTERO (UNUM == 1)
            // =======================================
            if (player.unum == 1)
            {
                const double radio_despeje      = 1.5;  // muy cerca → despejo
                const double radio_salir        = 10.0; // balón cerca del área → voy a por él
                const double margen_orientacion = 5.0;  // grados

                if (player.see_ball)
                {
                    // Si ya tenemos el balón, hacemos un saque sencillo hacia delante
                    if (goalie_has_ball)
                    {
                        int power_saque = 100;  // ajustable
                        std::string kick_cmd = "(kick " + to_string(power_saque) + " 0)";
                        udp_socket.sendTo(kick_cmd, server_udp);
                        std::cout << "PORTERO → saca de puerta: " << kick_cmd << std::endl;

                        // Ya no tenemos el balón tras el saque
                        goalie_has_ball = false;
                        continue;
                    }

                    // Datos del balón (SOLO si lo veo)
                    vector<string> ball_coords = separate_string_separator(see_message[ball_pos], " ");
                    ball.distancia = ball_coords[1];  // distancia en string
                    ball.angulo    = ball_coords[2];  // ángulo en string

                    double distance = std::stod(ball.distancia);  // distancia al balón
                    double angle    = std::stod(ball.angulo);     // ángulo relativo en grados

                    cout << "Pelota - distancia: " << distance << " | ángulo: " << angle << " grados" << endl;

                    // --- 1. SI EL BALÓN ESTÁ MUY CERCA → INTENTAR CATCH ---
                    if (distance <= radio_despeje)
                    {
                        // Intentar un catch cuando estemos cerca
                        std::string catch_cmd = "(catch 0)";
                        udp_socket.sendTo(catch_cmd, server_udp);
                        cout << "PORTERO → intentando CATCH: " << catch_cmd << endl;

                        goalie_has_ball = true;

                        continue; // no hacemos más cosas en este ciclo
                    }

                    // --- 2. SI EL BALÓN ESTÁ A DISTANCIA DE SALIR (~10m) → IR A POR ÉL ---
                    if (distance <= radio_salir)
                    {
                        // primero orientarse hacia el balón
                        if (std::abs(angle) > margen_orientacion)
                        {
                            std::string turn_cmd = "(turn " + std::to_string(angle / 5.0) + ")";
                            udp_socket.sendTo(turn_cmd, server_udp);
                            cout << "PORTERO → girando hacia balón: " << turn_cmd << endl;
                        }
                        else
                        {
                            // avanzar hacia balón
                            int power = 70;
                            std::string dash_cmd = "(dash " + std::to_string(power) + " 0)";
                            udp_socket.sendTo(dash_cmd, server_udp);
                            cout << "PORTERO → saliendo a por balón: " << dash_cmd << endl;
                        }
                        continue; // no seguimos con más lógica de este ciclo
                    }

                    // --- 3. SI EL BALÓN ESTÁ LEJOS → VOLVER A PORTERÍA ---
                    if (veo_porteria)
                    {
                        // ang_flag_porteria es el ángulo relativo a la portería, en grados
                        if (std::abs(ang_flag_porteria) > margen_orientacion)
                        {
                            // me oriento hacia la portería
                            std::string turn_cmd = "(turn " + std::to_string(ang_flag_porteria / 5.0) + ")";
                            udp_socket.sendTo(turn_cmd, server_udp);
                            cout << "PORTERO → orientándose a portería: " << turn_cmd << endl;
                        }
                        else
                        {
                            // ya estoy bastante orientado → camino hacia ella
                            int power = 40;
                            std::string dash_cmd = "(dash " + std::to_string(power) + " 0)";
                            udp_socket.sendTo(dash_cmd, server_udp);
                            cout << "PORTERO → volviendo a portería: " << dash_cmd << endl;
                        }
                    }
                    else
                    {
                        // No veo la portería:
                        //  → giro para buscarla
                        //  → y además doy un pequeño paso hacia atrás
                        std::string turn_cmd = "(turn 40)";
                        udp_socket.sendTo(turn_cmd, server_udp);
                        cout << "PORTERO → buscando portería: " << turn_cmd << endl;

                        std::string dash_cmd = "(dash 30 180)";
                        udp_socket.sendTo(dash_cmd, server_udp);
                        cout << "PORTERO → retrocediendo mientras la busca: " << dash_cmd << endl;
                    }

                    continue; // MUY IMPORTANTE: el portero NO entra en la lógica normal
                }
                else
                {
                    // El portero NO ve el balón:
                    //  → se centra en volver a portería / buscarla
                    if (veo_porteria)
                    {
                        if (std::abs(ang_flag_porteria) > margen_orientacion)
                        {
                            std::string turn_cmd = "(turn " + std::to_string(ang_flag_porteria / 5.0) + ")";
                            udp_socket.sendTo(turn_cmd, server_udp);
                            cout << "PORTERO (sin ver balón) → orientándose a portería: " << turn_cmd << endl;
                        }
                        else
                        {
                            std::string dash_cmd = "(dash 40 0)";
                            udp_socket.sendTo(dash_cmd, server_udp);
                            cout << "PORTERO (sin ver balón) → acercándose a portería: " << dash_cmd << endl;
                        }
                    }
                    else
                    {
                        std::string turn_cmd = "(turn 40)";
                        udp_socket.sendTo(turn_cmd, server_udp);
                        cout << "PORTERO (sin ver balón) → buscando portería: " << turn_cmd << endl;
                    }

                    continue; // igual: portero no entra en lógica normal
                }
            }


            // ==================================================
            //       LÓGICA DE JUGADORES NORMALES
            // ==================================================
            double distancia_retorno = 25.0;   // si el balón está más lejos que esto → volver a posición base
            bool es_delantero = (player.unum == 7 || player.unum == 9 || player.unum == 11);
            if (player.see_ball)
            {
                vector<string> ball_coords = separate_string_separator(see_message[ball_pos], " ");
                ball.distancia = ball_coords[1];
                ball.angulo    = ball_coords[2];

                double distance = stod(ball.distancia);
                double angle = stod(ball.angulo);

                auto goal = parseGoalOpponent(received_message_content, mySide);


                // ============================================
                // Solo delanteros persiguen SIEMPRE el balón
                // ============================================
                if (!es_delantero && distance > distancia_retorno)
                {
                    // balón muy lejos → volver a base
                    double ang = (player.home_y > 0 ? 60 : -60);
                    std::string tcmd = "(turn " + std::to_string(ang) + ")";
                    udp_socket.sendTo(tcmd, server_udp);

                    std::string dcmd = "(dash 40 0)";
                    udp_socket.sendTo(dcmd, server_udp);

                    cout << "Jugador " << player.unum << " NO persigue balón, retorna a base." << endl;
                    continue;
                }

                // ==> si delantero, o si distancia < umbral → persecución normal
                if (distance < 1.5)
                {
                    std::string kick_command = "(kick 100 " + to_string(goal->angle) + ")";
                    udp_socket.sendTo(kick_command, server_udp);
                    cout << "Chutando balón." << endl;
                }
                else
                {
                    if (abs(angle) >= 10)
                    {
                        int division = (distance < 6 ? 20 : 5);
                        std::string rotate_command = "(turn " + to_string(angle / division) + ")";
                        udp_socket.sendTo(rotate_command, server_udp);
                        cout << "Girando hacia pelota." << endl;
                    }
                    else
                    {
                        int power = (distance < 3 ? 60 : distance < 7 ? 80 : 100);
                        std::string dash_command = "(dash " + to_string(power) + " 0)";
                        udp_socket.sendTo(dash_command, server_udp);
                        cout << "Corriendo hacia pelota." << endl;
                    }
                }
            }

            else  // no veo el balón
            {
                if (!es_delantero)  // defensas y medios
                {
                    // ===== Volver a posición base =====
                    double dx = player.home_x;   // ojo: estas coordenadas están en el sistema absoluto
                    double dy = player.home_y;

                    // no tienes posición absoluta del jugador, así que usamos flags/ángulos
                    // SOLUCIÓN SIMPLE: orientar hacia donde *debería* estar la home position
                    double objetivo_y = dy;
                    double objetivo_x = dx;

                    // Si el jugador está en el lado derecho se invierte el campo
                    if (player.side == "r") {
                        objetivo_x = -objetivo_x;
                        objetivo_y = -objetivo_y;
                    }

                    // girar hacia la posición base (aproximación usando solo eje Y)
                    double ang = (objetivo_y > 0 ? 60 : -60);
                    std::string turn_cmd = "(turn " + std::to_string(ang) + ")";
                    udp_socket.sendTo(turn_cmd, server_udp);

                    // avanzar lentamente hacia home
                    std::string dash_cmd = "(dash 40 0)";
                    udp_socket.sendTo(dash_cmd, server_udp);

                    cout << "Jugador " << player.unum << " volviendo a home position." << endl;
                }
                else
                {
                    // comportamiento original de los delanteros
                    std::string rotate_command = (player.y < 0 ? "(turn -80)" : "(turn 80)");
                    udp_socket.sendTo(rotate_command, server_udp);
                    cout << "Delantero buscando pelota." << endl;
                }
            }

        }
    }

    return 0;
}
