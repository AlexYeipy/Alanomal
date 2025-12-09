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

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        cout << "Uso: " << argv[0] << " <nombre-equipo> <puerto>" << endl;
        return 1;
    }

    string team_name = argv[1];
    MinimalSocket::Port this_socket_port = std::stoi(argv[2]);

    cout << "Creando socket UDP..." << endl;

    MinimalSocket::udp::Udp<true> udp_socket(this_socket_port, MinimalSocket::AddressFamily::IP_V6);
    cout << "Socket creado correctamente" << endl;

    if (!udp_socket.open())
    {
        cout << "Error al abrir el socket" << endl;
        return 1;
    }

    MinimalSocket::Address other_recipient_udp = MinimalSocket::Address{"127.0.0.1", 6000};

    udp_socket.sendTo("(init " + team_name + "(version 19))", other_recipient_udp);
    cout << "Mensaje de inicialización enviado" << endl;

    std::size_t message_max_size = 1000000;
    cout << "Esperando mensaje del servidor..." << endl;

    auto received_message = udp_socket.receive(message_max_size);
    std::string received_message_content = received_message->received_message;

    MinimalSocket::Address other_sender_udp = received_message->sender;
    MinimalSocket::Address server_udp = MinimalSocket::Address{"127.0.0.1", other_sender_udp.getPort()};

    Player player{team_name, "", "", false, 0, 0, 0};
    Ball ball{"0", "0", "0", "0"};

    player = parseInitialMessage(received_message_content, player);
    cout << "Número de jugador asignado: " << player.unum << endl;

    sendInitialMoveMessage(player, udp_socket, server_udp);

    // ========================================================
    //                 BUCLE PRINCIPAL
    // ========================================================
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        auto received_message = udp_socket.receive(message_max_size);
        std::string received_message_content = received_message->received_message;

        vector<string> parsed_message = separate_string(received_message_content);

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
                    // Datos del balón (SOLO si lo veo)
                    vector<string> ball_coords = separate_string_separator(see_message[ball_pos], " ");
                    ball.distancia = ball_coords[1];  // distancia en string
                    ball.angulo    = ball_coords[2];  // ángulo en string

                    double distance = std::stod(ball.distancia);  // distancia al balón
                    double angle    = std::stod(ball.angulo);     // ángulo relativo en grados

                    cout << "Pelota - distancia: " << distance << " | ángulo: " << angle << " grados" << endl;

                    // --- 1. SI EL BALÓN ESTÁ MUY CERCA → CHUTAR ---
                    if (distance <= radio_despeje)
                    {
                        int power = 100;
                        std::string kick_cmd = "(kick " + std::to_string(power) + " 0)";
                        udp_socket.sendTo(kick_cmd, server_udp);
                        cout << "PORTERO → despejando: " << kick_cmd << endl;
                        continue;  // no hacemos nada más en este ciclo
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
            if (player.see_ball)
            {
                vector<string> ball_coords = separate_string_separator(see_message[ball_pos], " ");
                ball.distancia = ball_coords[1];
                ball.angulo    = ball_coords[2];

                double distance = stod(ball.distancia);
                double angle = stod(ball.angulo);

                if (distance < 1.5)
                {
                    std::string kick_command = "(kick 100 0)";
                    udp_socket.sendTo(kick_command, server_udp);
                    cout << "Chutando balón: " << kick_command << endl;
                }
                else
                {
                    if (abs(angle) >= 10)
                    {
                        int division = (distance < 6 ? 20 : 5);
                        std::string rotate_command = "(turn " + to_string(angle / division) + ")";
                        udp_socket.sendTo(rotate_command, server_udp);
                        cout << "Girando hacia la pelota: " << rotate_command << endl;
                    }
                    else
                    {
                        int power = (distance < 3 ? 60 : distance < 7 ? 80 : 100);
                        std::string dash_command = "(dash " + to_string(power) + " 0)";
                        udp_socket.sendTo(dash_command, server_udp);
                        cout << "Corriendo hacia la pelota: " << dash_command << endl;
                    }
                }
            }
            else
            {
                std::string rotate_command = (player.y < 0 ? "(turn -80)" : "(turn 80)");
                udp_socket.sendTo(rotate_command, server_udp);

                cout << "Buscando pelota: " << rotate_command << endl;
            }
        }
    }

    return 0;
}
