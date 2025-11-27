#include <iostream>

using namespace std;

#include <MinimalSocket/udp/UdpSocket.h>
#include <chrono>
#include <thread>
#include <vector>

// Estructura que representa un jugador con sus propiedades básicas
struct Player{
    std::string team_name;  // Nombre del equipo
    std::string playmode;   // Modo de juego (before_kick_off, play_on, etc.)
    std::string side;       // Lado del campo (l = izquierda, r = derecha)
    int unum;               // Número del jugador (1-11)
    double x;               // Posición X actual (no utilizada en este código)
    double y;               // Posición Y actual (no utilizada en este código)
};

// Función que parsea el mensaje inicial del servidor para extraer información del jugador
Player parseInitialMessage(std::string &message, Player &player){
    std::string delimiter = " ";
    size_t pos = 0;
    std::string token;
    int i = 0;
    
    // Divide el mensaje por espacios y extrae la información
    while ((pos = message.find(delimiter)) != std::string::npos){
        token = message.substr(0, pos);
        if (i == 1){
            player.side = token;  // Segundo token: lado del campo
        }
        else if (i == 2) {
            player.unum = std::stoi(token);  // Tercer token: número de jugador
        }
        else if (i == 3) {
            player.playmode = token;  // Cuarto token: modo de juego
        }
        message.erase(0, pos + delimiter.length());
        i++;
    }
    cout << "Player: " << player.side << " " << player.unum << " " << player.playmode << endl;
    return player;
}

// Función que envía el comando de movimiento inicial al servidor
void sendInitialMoveMessage(const Player &player,
                            MinimalSocket::udp::Udp<true> &udp_socket,
                            MinimalSocket::Address const &recipient) {
    struct Posicion
    {
        int x;
        int y;
    };

    // Vector con las posiciones iniciales para el lado izquierdo (formación 4-4-2)
    // Las posiciones están hardcodeadas para cada número de jugador
    vector<Posicion> posiciones = {
        {-50, 0},    // 1: Portero
        {-35, -20},  // 2: Defensa izquierda
        {-35, 20},   // 3: Defensa derecha
        {-20, -25},  // 4: Medio izquierdo
        {-18, -9},   // 5: Medio izquierdo-central
        {-18, 5},    // 6: Medio derecho-central
        {-20, 20},   // 7: Medio derecho
        {-2, -18},   // 8: Delantero izquierdo
        {-28, -18},  // 9: Delantero izquierdo-extremo
        {-35, 11},   // 10: Delantero derecho-extremo
        {-5, 0}      // 11: Delantero centro
    };

    // Obtiene la posición correspondiente al número del jugador
    Posicion myPos = posiciones[player.unum - 1];
    
    // Construye el comando move en formato S-Expression
    auto moveCommand = "(move " + to_string(myPos.x) + " " + to_string(myPos.y) + ")";

    // Envía el comando al servidor a través del socket UDP
    udp_socket.sendTo(moveCommand, recipient);

    cout << "Move command sent: " << moveCommand << endl;
}

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
