#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <MinimalSocket/udp/UdpSocket.h>

using namespace std;

// Include headers
#include "funciones.h"
#include "structs.h"

/**
 * Parsea el mensaje inicial del servidor para extraer información del jugador
 * Formato del mensaje: "init <side> <unum> <playmode>"
 *
 * @param message Mensaje recibido del servidor
 * @param player Referencia al objeto Player a actualizar
 * @return Player actualizado con la información parseada
 */
Player parseInitialMessage(std::string &message, Player &player)
{
    std::string delimiter = " ";
    size_t pos = 0;
    std::string token;
    int i = 0;

    // Divide el mensaje por espacios y extrae la información
    while ((pos = message.find(delimiter)) != std::string::npos)
    {
        token = message.substr(0, pos);

        // Asigna cada token a la propiedad correspondiente del jugador
        if (i == 1)
        {
            player.side = token;  // Segundo token: lado del campo
        }
        else if (i == 2)
        {
            player.unum = std::stoi(token);  // Tercer token: número de jugador
        }
        else if (i == 3)
        {
            player.playmode = token;  // Cuarto token: modo de juego
        }

        // Elimina el token procesado del mensaje
        message.erase(0, pos + delimiter.length());
        i++;
    }

    cout << "Player - Lado: " << player.side
         << " | Número: " << player.unum
         << " | Modo: " << player.playmode << endl;

    return player;
}

/**
 * Separa un string en múltiples substrings usando un separador específico
 *
 * @param s String original a separar
 * @param separator Carácter o string usado como separador
 * @return Vector de strings resultantes de la separación
 */
vector<string> separate_string_separator(string &s, string separator)
{
    vector<string> v;
    int pos = 0;

    // Encuentra todas las ocurrencias del separador
    while ((pos = s.find(separator)) != string::npos)
    {
        // Añade el substring antes del separador al vector
        v.push_back(s.substr(0, pos));
        // Elimina el substring procesado
        s.erase(0, pos + separator.length());
    }

    // Añade el último substring (después del último separador)
    v.push_back(s);
    return v;
}

/**
 * Separa un string en substrings basándose en paréntesis anidados
 * Ejemplo: "((hola) (soy) (dani)) (que tal) (estas)" -> {"(hola) (soy) (dani)", "que tal", "estas"}
 *
 * @param s String original con estructura de paréntesis
 * @return Vector de strings separados por niveles de paréntesis
 */
vector<string> separate_string(string &s)
{
    vector<string> v;
    int pos = 0;
    int level = 0;  // Nivel de anidamiento de paréntesis
    string temp;    // String temporal para acumular contenido

    // Recorre cada carácter del string
    while (pos < s.size())
    {
        // Incrementa nivel al encontrar '('
        if (s[pos] == '(')
        {
            level++;
        }
        // Decrementa nivel al encontrar ')'
        else if (s[pos] == ')')
        {
            level--;
        }

        // Lógica de separación basada en niveles de paréntesis
        if (s[pos] == '(' && level == 1)
        {
            // Inicia nuevo string temporal al encontrar primer '(' del nivel 1
            temp = "";
        }
        else if (s[pos] == ')' && level == 0)
        {
            // Finaliza y guarda el string temporal al cerrar paréntesis del nivel 0
            v.push_back(temp);
        }
        else
        {
            // Añade carácter al string temporal (excepto paréntesis del nivel 0)
            temp += s[pos];
        }
        pos++;
    }

    // Verificación de integridad: todos los paréntesis deben estar balanceados
    if (level != 0)
    {
        throw std::runtime_error("Error: Paréntesis desbalanceados - no coinciden aperturas y cierres");
    }
    else
    {
        return v;
    }
}

/**
 * Envía el comando de movimiento inicial para posicionar al jugador en el campo
 * Asigna posiciones específicas basadas en una formación 4-4-2
 *
 * @param player Objeto Player con información del jugador
 * @param udp_socket Socket UDP para comunicación
 * @param recipient Dirección del servidor destino
 */
void sendInitialMoveMessage(const Player &player,
                            MinimalSocket::udp::Udp<true> &udp_socket,
                            MinimalSocket::Address const &recipient)
{
    // Estructura para coordenadas de posición
    struct Posicion
    {
        int x;
        int y;
    };

    // Formación 4-4-2 con posiciones iniciales para cada número de jugador
    vector<Posicion> posiciones = {
        {-50, 0},    // 1: Portero
        {-40, -10},  // 2: Defensa izquierdo
        {-35, -28},  // 3: Defensa central izquierdo
        {-40, 10},   // 4: Defensa central derecho
        {-35, 28},   // 5: Defensa derecho
        {-25, 11},   // 6: Medio derecho
        {-8, 20},    // 7: Delantero derecho
        {-25, -11},  // 8: Medio izquierdo
        {-5, 0},     // 9: Delantero centro
        {-15, 0},    // 10: Medio centro ofensivo
        {-8, -20}    // 11: Delantero izquierdo
    };

    // Obtiene la posición correspondiente al número del jugador
    Posicion myPos = posiciones[player.unum - 1];

    // Construye y envía el comando move
    auto moveCommand = "(move " + to_string(myPos.x) + " " + to_string(myPos.y) + ")";

    // Envía el comando al servidor a través del socket UDP
    udp_socket.sendTo(moveCommand, recipient);

    cout << "Comando de movimiento enviado - Posición: " << moveCommand << endl;
}
