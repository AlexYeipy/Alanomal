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
    if (argc != 3)
    {
        cout << "Uso: " << argv[0] << " <nombre-equipo> <puerto>" << endl;
        return 1;
    }

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

    // Envío del mensaje de inicialización al servidor
    // Formato: (init <team_name> (version 15))
    udp_socket.sendTo("(init " + team_name + "(version 19))", other_recipient_udp);
    cout << "Mensaje de inicialización enviado" << endl;

    // Espera y recepción de la respuesta del servidor
    std::size_t message_max_size = 1000000;
    cout << "Esperando mensaje del servidor..." << endl;
    auto received_message = udp_socket.receive(message_max_size);
    std::string received_message_content = received_message->received_message;

    // Actualización de la dirección del servidor con el puerto que respondió
    MinimalSocket::Address other_sender_udp = received_message->sender;
    MinimalSocket::Address server_udp = MinimalSocket::Address{"127.0.0.1", other_sender_udp.getPort()};

    // Creación de objetos para almacenar estado del jugador y la pelota
    Player player{team_name, "", "", false, 0, 0, 0};
    Ball ball{"0", "0", "0", "0"};

    // Parseo del mensaje inicial para obtener: lado, número de jugador y modo de juego
    player = parseInitialMessage(received_message_content, player);
    cout << "Número de jugador asignado: " << player.unum << endl;

    // Envío del comando de movimiento inicial a la posición asignada
    sendInitialMoveMessage(player, udp_socket, server_udp);

    /**
     * Bucle principal de ejecución del jugador
     * Procesa continuamente los mensajes del servidor y toma decisiones
     */
    while (true)
    {
        // Pequeña pausa para evitar saturación de CPU
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        // Recepción de mensaje del servidor
        auto received_message = udp_socket.receive(message_max_size);
        std::string received_message_content = received_message->received_message;

        // Separación del mensaje en componentes individuales
        vector<string> parsed_message = separate_string(received_message_content);

        // Procesamiento de mensajes de tipo "see" (información visual)
        if (parsed_message[0].find("see") <= 5)
        {
            // Parseo adicional del mensaje visual
            vector<string> see_message = separate_string(parsed_message[0]);

            // Búsqueda de la pelota en la información visual
            size_t ball_pos = 0;
            player.see_ball = false;
            for (size_t i = 0; i < see_message.size(); i++)
            {
                // Detección del objeto pelota "(b)" en el mensaje
                if (see_message[i].find("(b)") <= 5)
                {
                    ball_pos = i;
                    player.see_ball = true;
                    break;
                }
            }

            // Comportamiento cuando se detecta la pelota
            if (player.see_ball)
            {
                // Extracción de datos de la pelota
                vector<string> ball_coords = separate_string_separator(see_message[ball_pos], " ");
                ball.distancia = ball_coords[1]; // aquí guardamos la distancia como string
                ball.angulo = ball_coords[2]; // aquí guardamos el ángulo como string

                // Distancia y angulo reales (en double)
                double distance = std::stod(ball.distancia);  // distancia al balón
                double angle = std::stod(ball.angulo);  // ángulo relativo en grados

                cout << "Pelota - distancia: " << distance << " | ángulo: " << angle << " grados" << endl;

                // Lógica de toma de decisiones basada en distancia y ángulo
                if (distance < 1.5)
                {
                    // CHUTAR: Cuando está suficientemente cerca de la pelota
                    int power = 100;
                    std::string kick_command = "(kick " + to_string(power) + " 0)";
                    udp_socket.sendTo(kick_command, server_udp);
                    cout << "Comando de chute enviado: " << kick_command << endl;
                }
                else
                {
                    // GIRAR: Cuando el ángulo es demasiado grande
                    if (abs(angle) >= 10)
                    {
                        int division = 1;
                        // Ajuste fino del giro según la distancia
                        if (distance < 6)
                        {
                            division = 20;  // Giro más suave cuando está cerca
                        }
                        else
                        {
                            division = 5;   // Giro más brusco cuando está lejos
                        }
                        std::string rotate_command = "(turn " + to_string(angle / division) + ")";
                        udp_socket.sendTo(rotate_command, server_udp);
                        cout << "Girando hacia la pelota: " << rotate_command << endl;
                    }
                    else
                    {
                        // CORRER: Cuando está bien alineado con la pelota
                        int power = 100;
                        // Ajuste de potencia según distancia
                        if (distance < 3)
                        {
                            power = 60;   // Dash suave cuando está muy cerca
                        }
                        else if (distance < 7)
                        {
                            power = 80;   // Dash medio en distancias intermedias
                        }
                        std::string dash_command = "(dash " + to_string(power) + " 0)";
                        udp_socket.sendTo(dash_command, server_udp);
                        cout << "Corriendo hacia la pelota: " << dash_command << endl;
                    }
                }
            }
            else
            {
                // BÚSQUEDA: Comportamiento cuando no se ve la pelota
                // Gira en diferentes direcciones según su posición en el campo
                if (player.y < 0)
                {
                    std::string rotate_command = "(turn " + to_string(-80) + ")";
                    udp_socket.sendTo(rotate_command, server_udp);
                    cout << "Buscando pelota (giro izquierda): " << rotate_command << endl;
                }
                else
                {
                    std::string rotate_command = "(turn " + to_string(80) + ")";
                    udp_socket.sendTo(rotate_command, server_udp);
                    cout << "Buscando pelota (giro derecha): " << rotate_command << endl;
                }
            }
        }
    }

    return 0;
}


