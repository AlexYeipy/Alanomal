#include <iostream>

using namespace std;

#include <MinimalSocket/udp/UdpSocket.h>
#include <chrono>
#include <thread>
#include <vector>

#include "funciones.h"
#include "structs.h"

// Función principal que recibe dos argumentos: nombre del equipo y puerto
int main(int argc, char *argv[])
{
    // Verifica que se hayan proporcionado los argumentos correctos
    if (argc != 3)
    {
        cout << "Usage: " << argv[0] << " <team-name> <this-port>" << endl;
        return 1;
    }

    // Extrae los argumentos de la línea de comandos
    string team_name = argv[1];
    MinimalSocket::Port this_socket_port = std::stoi(argv[2]);

    cout << "Creating a UDP socket" << endl;

    // Crea un socket UDP IPv6
    MinimalSocket::udp::Udp<true> udp_socket(this_socket_port, MinimalSocket::AddressFamily::IP_V6);

    cout << "Socket created" << endl;

    // Intenta abrir el socket
    bool success = udp_socket.open();

    if (!success)
    {
        cout << "Error opening socket" << endl;
        return 1;
    }

    // Define la dirección del servidor (localhost puerto 6000)
    MinimalSocket::Address other_recipient_udp = MinimalSocket::Address{"127.0.0.1", 6000};
    cout << "(init " + team_name + "(version 19))";

    // Construye y envía el mensaje de inicialización al servidor
    udp_socket.sendTo("(init " + team_name + "(version 19))", other_recipient_udp);
    cout << "Init Message sent" << endl;

    // Espera la respuesta del servidor
    std::size_t message_max_size = 1000;
    cout << "Waiting for a message" << endl;
    auto received_message = udp_socket.receive(message_max_size);
    std::string received_message_content = received_message->received_message;

    // Actualiza la dirección del servidor con el puerto que respondió
    MinimalSocket::Address other_sender_udp = received_message->sender;
    MinimalSocket::Address server_udp = MinimalSocket::Address{"127.0.0.1", other_sender_udp.getPort()};

    // Crea y configura el objeto jugador
    Player player{team_name, "", "", 0, 0, 0};
    
    // Parsea el mensaje inicial para obtener lado, número y modo de juego
    player = parseInitialMessage(received_message_content, player);

    cout << player.unum << endl;
    
    // Envía el comando de movimiento inicial
    sendInitialMoveMessage(player, udp_socket, server_udp);

    // Bucle infinito para mantener el programa ejecutándose
    while (true){
        std::this_thread::sleep_for(std::chrono::seconds(30));
    }

    return 0;
}
