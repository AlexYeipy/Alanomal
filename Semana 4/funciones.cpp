#include <iostream>
#include <vector>
#include <string>
#include <regex>
#include <optional>
#include <stdexcept>
#include <cmath>
#include <thread>
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
void sendInitialMoveMessage(Player &player,
                            MinimalSocket::udp::Udp<true> &udp_socket,
                            MinimalSocket::Address const &recipient)
{
    struct Posicion { int x; int y; };

    // Formación 4-4-2 (La misma que tenías)
    vector<Posicion> posiciones = {
        {-50, 0},     // 1: Portero
        {-35, 20},   // 2: Lateral derecho
        {-35, -20},     // 3: Lateral izquierdo
        {-35, 5},    // 4: Defensa central der
        {-35, -5},   // 5: Defensa central izq
        {-15, -10},   // 6: Medio derecho
        {-15, 25},    // 7: Extremo derecho
        {-15, 10},    // 8: Medio izquierdo
        {-2, 10},      // 9: Delantero centro uno
        {-2, -10},     // 10: Delantero centro dos
        {-15, -25}      // 11: Extremo izquierdo
    };

    // Aseguramos que el índice sea válido
    if (player.unum > 0 && player.unum <= 11) {
        Posicion myPos = posiciones[player.unum - 1];

        // Guardamos la posición en el jugador
        player.home_x = myPos.x;
        player.home_y = myPos.y;
        // --------------------------------------------------

        auto moveCommand = "(move " + to_string(myPos.x) + " " + to_string(myPos.y) + ")";
        udp_socket.sendTo(moveCommand, recipient);
        cout << "Jugador " << player.unum << " inicializado en: " << moveCommand << endl;
    }
}

optional<GoalInfo> parseGoalOpponent(const std::string& seeMsg, const std::string& mySide)
{
    std::string targetGoal = (mySide == "l") ? "g r" : "g l";

    // Regex: \( * objetivo * \) y los espacios y numeros
    std::regex rgx("\\(\\s*" + targetGoal + "\\s*\\)\\s*([-+]?[0-9]*\\.?[0-9]+)\\s+([-+]?[0-9]*\\.?[0-9]+)");
    std::smatch match;

    if (std::regex_search(seeMsg, match, rgx)) {
        if (match.size() >= 3) {
            try {
                GoalInfo info;
                info.dist = std::stod(match[1].str());
                info.angle = std::stod(match[2].str());
                std::cout << "Porteria rival: Distancia: " << info.dist
                          << " | Angulo: " << info.angle << endl;
                return info;
            } catch (const std::exception& e) {
                std::cerr << "Error al convertir distancia/angulo: " << e.what() << " | "
                          << "token1='" << match[1] << "' token2='" << match[2] << "'\n";
            }
        }
    }

    return std::nullopt;  // Por si no se ve la porteria
}


