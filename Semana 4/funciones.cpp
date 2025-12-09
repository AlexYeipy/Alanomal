#include <iostream>
#include <vector>
#include <string>
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
        {-35, -20},   // 2: Defensa izquierdo
        {-35, 0},     // 3: Defensa central izq
        {-35, 20},    // 4: Defensa central der
        {-35, -20},   // 5: Defensa derecho
        {-15, -20},   // 6: Medio derecho
        {-5, -30},    // 7: Delantero derecho
        {-15, 20},    // 8: Medio izquierdo
        {-5, 0},      // 9: Delantero centro
        {-10, 0},     // 10: Medio centro ofensivo
        {-5, 30}      // 11: Delantero izquierdo
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

// Encontrar informacion en la funcion see_message
void store_data_see(vector<string> &see_message, Player &player, Ball &ball, Goal &own_goal, Goal &opponent_goal, Field &field)
{
    vector<string> ball_coords;
    bool found_ball = false;
    player.flags_seen = 0;

    player.see_ball = false;
    player.seeing_zone = false;

    // // Field flags not seen
    // field.flag_center = {999, 999};
    // field.flag_center_top = {999, 999};
    // field.flag_center_bottom = {999, 999};
    // field.flag_left_top = {999, 999};
    // field.flag_left_bottom = {999, 999};
    // field.flag_right_top = {999, 999};
    // field.flag_right_bottom = {999, 999};
    // field.flag_penalty_left_top = {999, 999};
    // field.flag_penalty_left_center = {999, 999};
    // field.flag_penalty_left_bottom = {999, 999};
    // field.flag_penalty_right_top = {999, 999};
    // field.flag_penalty_right_center = {999, 999};
    // field.flag_penalty_right_bottom = {999, 999};
    // field.flag_goal_left_top = {999, 999};
    // field.flag_goal_left_bottom = {999, 999};
    // field.flag_goal_right_top = {999, 999};
    // field.flag_goal_right_bottom = {999, 999};

    // // Boundary flags not seen
    // boundaries.left_top = {999, 999};
    // boundaries.left_bot = {999, 999};
    // boundaries.right_top = {999, 999};
    // boundaries.right_bot = {999, 999};
    // boundaries.top_left_50 = {999, 999};
    // boundaries.top_left_40 = {999, 999};
    // boundaries.top_left_30 = {999, 999};
    // boundaries.top_left_20 = {999, 999};
    // boundaries.top_left_10 = {999, 999};
    // boundaries.top_0 = {999, 999};
    // boundaries.top_right_10 = {999, 999};
    // boundaries.top_right_20 = {999, 999};
    // boundaries.top_right_30 = {999, 999};
    // boundaries.top_right_40 = {999, 999};
    // boundaries.top_right_50 = {999, 999};
    // boundaries.bot_left_50 = {999, 999};
    // boundaries.bot_left_40 = {999, 999};
    // boundaries.bot_left_30 = {999, 999};
    // boundaries.bot_left_20 = {999, 999};
    // boundaries.bot_left_10 = {999, 999};
    // boundaries.bot_0 = {999, 999};
    // boundaries.bot_right_10 = {999, 999};
    // boundaries.bot_right_20 = {999, 999};
    // boundaries.bot_right_30 = {999, 999};
    // boundaries.bot_right_40 = {999, 999};
    // boundaries.bot_right_50 = {999, 999};


    if (own_goal.side == "l")
    {
        player.see_own_goal = false;
    }
    else
    {
        player.see_opponent_goal = false;
    }
    if (own_goal.side == "r")
    {
        player.see_own_goal = false;
    }
    else
    {
        player.see_opponent_goal = false;
    }

    for (size_t i = 0; i < see_message.size(); i++)
    {
        // Search for the ball
        if (see_message[i].find("(b)") != string::npos)
        {
            player.see_ball = true;
            ball_coords = separate_string_separator(see_message[i], " ");
            ball.x = ball_coords[1];
            ball.y = ball_coords[2];
            cout << "Ball coordinates: " << ball.x << " " << ball.y << endl;
            // Calculate the distance to the ball
            double distance = sqrt(pow(stod(ball.x), 2) + pow(stod(ball.y), 2));
            ball.distance = distance;
            // Calculate the angle to the ball
            double angle = atan2(stod(ball.y), stod(ball.x));
            ball.angle = angle * 180 / M_PI;
            found_ball = true;
        }

        // Search for the right goal
        if (see_message[i].find("(g r)") != string::npos)
        {
            cout << "The player sees the right goal" << endl;
            vector<string> goal_coords = separate_string_separator(see_message[i], " ");

            if (own_goal.side == "r")
            {
                own_goal.x = goal_coords[2];
                own_goal.y = goal_coords[3];
                cout << "Own goal coordinates: " << own_goal.x << " " << own_goal.y << endl;
                player.see_own_goal = true;
            }
            else
            {
                opponent_goal.x = goal_coords[2];
                opponent_goal.y = goal_coords[3];
                opponent_goal.angle = atan2(stof(goal_coords[3]), stof(goal_coords[2])) * 180 / M_PI;
                cout << "Opponent goal coordinates: " << opponent_goal.x << " " << opponent_goal.y << endl;
                player.see_opponent_goal = true;
            }
        }

        // Search for the left goal
        if (see_message[i].find("(g l)") != string::npos)
        {
            cout << "The player sees the left goal" << endl;
            vector<string> goal_coords = separate_string_separator(see_message[i], " ");

            if (own_goal.side == "l")
            {
                own_goal.x = goal_coords[2];
                own_goal.y = goal_coords[3];
                cout << "Own goal coordinates: " << own_goal.x << " " << own_goal.y << endl;
                player.see_own_goal = true;
            }
            else
            {
                opponent_goal.x = goal_coords[2];
                opponent_goal.y = goal_coords[3];
                opponent_goal.angle = atan2(stof(goal_coords[3]), stof(goal_coords[2])) * 180 / M_PI;
                cout << "Opponent goal coordinates: " << opponent_goal.x << " " << opponent_goal.y << endl;
                player.see_opponent_goal = true;
            }
        }

        // Search for the flags
        // Search for the center flag
        if (see_message[i].find("(f c)") != string::npos)
        {
            player.flags_seen++;
            vector<string> center_coords = separate_string_separator(see_message[i], " ");
            field.flag_center = {stof(center_coords[2]), stof(center_coords[3])};
            field.flag_center_distance = sqrt(pow(stof(center_coords[2]),2) + pow(stof(center_coords[3]), 2));
            cout << "Center flag coordinates: " << field.flag_center[0] << " " << field.flag_center[1] << endl;
        }
        if (player.zone_name == "(f c)")
        {
            player.seeing_zone = true;
        }

        // Search for the center top flag
        if (see_message[i].find("(f c t)") != string::npos)
        {
            vector<string> center_top_coords = separate_string_separator(see_message[i], " ");
            field.flag_center_top = {stof(center_top_coords[3]), stof(center_top_coords[4])};
            field.flag_center_top_distance = sqrt(pow(stof(center_top_coords[3]),2) + pow(stof(center_top_coords[4]), 2));
            player.flags_seen++;
            if (player.zone_name == "(f c t)")
            {
                player.seeing_zone = true;
            }
        }

        // Search for the center bottom flag
        if (see_message[i].find("(f c b)") != string::npos)
        {
            vector<string> center_bottom_coords = separate_string_separator(see_message[i], " ");
            field.flag_center_bottom = {stof(center_bottom_coords[3]), stof(center_bottom_coords[4])};
            field.flag_center_bottom_distance = sqrt(pow(stof(center_bottom_coords[3]),2) + pow(stof(center_bottom_coords[4]), 2));
            player.flags_seen++;
            if (player.zone_name == "(f c b)")
            {
                player.seeing_zone = true;
            }
        }

        // Search for the left top flag
        if (see_message[i].find("(f l t)") != string::npos)
        {
            vector<string> left_top_coords = separate_string_separator(see_message[i], " ");
            field.flag_left_top = {stof(left_top_coords[3]), stof(left_top_coords[4])};
            field.flag_left_top_distance = sqrt(pow(stof(left_top_coords[3]),2) + pow(stof(left_top_coords[4]), 2));
            field.flag_left_top_angle = atan2(stof(left_top_coords[4]), stof(left_top_coords[3])) * 180 / M_PI;
            player.flags_seen++;
            if (player.zone_name == "(f l t)")
            {
                player.seeing_zone = true;
            }
        }

        // Search for the left bottom flag
        if (see_message[i].find("(f l b)") != string::npos)
        {
            vector<string> left_bottom_coords = separate_string_separator(see_message[i], " ");
            field.flag_left_bottom = {stof(left_bottom_coords[3]), stof(left_bottom_coords[4])};
            player.flags_seen++;
            if (player.zone_name == "(f l b)")
            {
                player.seeing_zone = true;
            }
        }

        // Search for the right top flag
        if (see_message[i].find("(f r t)") != string::npos)
        {
            vector<string> right_top_coords = separate_string_separator(see_message[i], " ");
            field.flag_right_top = {stof(right_top_coords[3]), stof(right_top_coords[4])};
            player.flags_seen++;
            if (player.zone_name == "(f r t)")
            {
                player.seeing_zone = true;
            }
        }

        // Search for the right bottom flag
        if (see_message[i].find("(f r b)") != string::npos)
        {
            vector<string> right_bottom_coords = separate_string_separator(see_message[i], " ");
            field.flag_right_bottom = {stof(right_bottom_coords[3]), stof(right_bottom_coords[4])};
            player.flags_seen++;
            if (player.zone_name == "(f r b)")
            {
                player.seeing_zone = true;
            }
        }

        // Search for the penalty area left top flag
        if (see_message[i].find("(f p l t)") != string::npos)
        {
            vector<string> penalty_left_top_coords = separate_string_separator(see_message[i], " ");
            field.flag_penalty_left_top = {stof(penalty_left_top_coords[4]), stof(penalty_left_top_coords[5])};
            player.flags_seen++;
            if (player.zone_name == "(f p l t)")
            {
                player.seeing_zone = true;
            }
        }

        // Search for the penalty area left center flag
        if (see_message[i].find("(f p l c)") != string::npos)
        {
            vector<string> penalty_left_center_coords = separate_string_separator(see_message[i], " ");
            field.flag_penalty_left_center = {stof(penalty_left_center_coords[4]), stof(penalty_left_center_coords[5])};
            player.flags_seen++;
            if (player.zone_name == "(f p l c)")
            {
                player.seeing_zone = true;
            }
        }

        // Search for the penalty area left bottom flag
        if (see_message[i].find("(f p l b)") != string::npos)
        {
            vector<string> penalty_left_bottom_coords = separate_string_separator(see_message[i], " ");
            field.flag_penalty_left_bottom = {stof(penalty_left_bottom_coords[4]), stof(penalty_left_bottom_coords[5])};
            player.flags_seen++;
            if (player.zone_name == "(f p l b)")
            {
                player.seeing_zone = true;
            }
        }

        // Search for the penalty area right top flag
        if (see_message[i].find("(f p r t)") != string::npos)
        {
            vector<string> penalty_right_top_coords = separate_string_separator(see_message[i], " ");
            field.flag_penalty_right_top = {stof(penalty_right_top_coords[4]), stof(penalty_right_top_coords[5])};
            player.flags_seen++;
            if (player.zone_name == "(f p r t)")
            {
                player.seeing_zone = true;
            }
        }

        // Search for the penalty area right center flag
        if (see_message[i].find("(f p r c)") != string::npos)
        {
            vector<string> penalty_right_center_coords = separate_string_separator(see_message[i], " ");
            field.flag_penalty_right_center = {stof(penalty_right_center_coords[4]), stof(penalty_right_center_coords[5])};
            player.flags_seen++;
            if (player.zone_name == "(f p r c)")
            {
                player.seeing_zone = true;
            }
        }

        // Search for the penalty area right bottom flag
        if (see_message[i].find("(f p r b)") != string::npos)
        {
            vector<string> penalty_right_bottom_coords = separate_string_separator(see_message[i], " ");
            field.flag_penalty_right_bottom = {stof(penalty_right_bottom_coords[4]), stof(penalty_right_bottom_coords[5])};
            player.flags_seen++;
            if (player.zone_name == "(f p r b)")
            {
                player.seeing_zone = true;
            }
        }

        // Search for the goal left top flag
        if (see_message[i].find("(f g l t)") != string::npos)
        {
            vector<string> goal_left_top_coords = separate_string_separator(see_message[i], " ");
            field.flag_goal_left_top = {stof(goal_left_top_coords[4]), stof(goal_left_top_coords[5])};
            player.flags_seen++;
            if (player.zone_name == "(f g l t)")
            {
                player.seeing_zone = true;
            }
        }

        // Search for the goal left bottom flag
        if (see_message[i].find("(f g l b)") != string::npos)
        {
            vector<string> goal_left_bottom_coords = separate_string_separator(see_message[i], " ");
            field.flag_goal_left_bottom = {stof(goal_left_bottom_coords[4]), stof(goal_left_bottom_coords[5])};
            player.flags_seen++;
            if (player.zone_name == "(f g l b)")
            {
                player.seeing_zone = true;
            }
        }

        // Search for the goal right top flag
        if (see_message[i].find("(f g r t)") != string::npos)
        {
            vector<string> goal_right_top_coords = separate_string_separator(see_message[i], " ");
            field.flag_goal_right_top = {stof(goal_right_top_coords[4]), stof(goal_right_top_coords[5])};
            player.flags_seen++;
            if (player.zone_name == "(f g r t)")
            {
                player.seeing_zone = true;
            }
        }

        // Search for the goal right bottom flag
        if (see_message[i].find("(f g r b)") != string::npos)
        {
            vector<string> goal_right_bottom_coords = separate_string_separator(see_message[i], " ");
            field.flag_goal_right_bottom = {stof(goal_right_bottom_coords[4]), stof(goal_right_bottom_coords[5])};
            player.flags_seen++;
            if (player.zone_name == "(f g r b)")
            {
                player.seeing_zone = true;
            }
        }
        // // BOUNDARIES
        // // Search for the boundary left top flag
        // if (see_message[i].find("(l t 30)") != string::npos)
        // {
        //     vector<string> goal_right_bottom_coords = separate_string_separator(see_message[i], " ");
        //     boundaries.left_top = {stof(goal_right_bottom_coords[3]), stof(goal_right_bottom_coords[4])};
        // }
        // // Search for the boundary left bot flag
        // if (see_message[i].find("(l b 30)") != string::npos)
        // {
        //     vector<string> goal_right_bottom_coords = separate_string_separator(see_message[i], " ");
        //     boundaries.left_bot = {stof(goal_right_bottom_coords[3]), stof(goal_right_bottom_coords[4])};
        // }
        // // Search for the boundary right top flag
        // if (see_message[i].find("(r t 30)") != string::npos)
        // {
        //     vector<string> goal_right_bottom_coords = separate_string_separator(see_message[i], " ");
        //     boundaries.right_top = {stof(goal_right_bottom_coords[3]), stof(goal_right_bottom_coords[4])};
        // }
        // // Search for the boundary right bot flag
        // if (see_message[i].find("(r b 30)") != string::npos)
        // {
        //     vector<string> goal_right_bottom_coords = separate_string_separator(see_message[i], " ");
        //     boundaries.right_bot = {stof(goal_right_bottom_coords[3]), stof(goal_right_bottom_coords[4])};
        // }
        // // Search for the boundary top left 50 flag
        // if (see_message[i].find("(t l 50)") != string::npos)
        // {
        //     vector<string> goal_right_bottom_coords = separate_string_separator(see_message[i], " ");
        //     boundaries.top_left_50 = {stof(goal_right_bottom_coords[3]), stof(goal_right_bottom_coords[4])};
        // }
        // // Search for the boundary top left 40 flag
        // if (see_message[i].find("(t l 40)") != string::npos)
        // {
        //     vector<string> goal_right_bottom_coords = separate_string_separator(see_message[i], " ");
        //     boundaries.top_left_40 = {stof(goal_right_bottom_coords[3]), stof(goal_right_bottom_coords[4])};
        // }
        // // Search for the boundary top left 30 flag
        // if (see_message[i].find("(t l 30)") != string::npos)
        // {
        //     vector<string> goal_right_bottom_coords = separate_string_separator(see_message[i], " ");
        //     boundaries.top_left_30 = {stof(goal_right_bottom_coords[3]), stof(goal_right_bottom_coords[4])};
        // }
        // // Search for the boundary top left 20 flag
        // if (see_message[i].find("(t l 20)") != string::npos)
        // {
        //     vector<string> goal_right_bottom_coords = separate_string_separator(see_message[i], " ");
        //     boundaries.top_left_20 = {stof(goal_right_bottom_coords[3]), stof(goal_right_bottom_coords[4])};
        // }
        // // Search for the boundary top left 10 flag
        // if (see_message[i].find("(t l 10)") != string::npos)
        // {
        //     vector<string> goal_right_bottom_coords = separate_string_separator(see_message[i], " ");
        //     boundaries.top_left_10 = {stof(goal_right_bottom_coords[3]), stof(goal_right_bottom_coords[4])};
        // }
        // // Search for the boundary top 0 flag
        // if (see_message[i].find("(t 0)") != string::npos)
        // {
        //     vector<string> goal_right_bottom_coords = separate_string_separator(see_message[i], " ");
        //     boundaries.top_0 = {stof(goal_right_bottom_coords[2]), stof(goal_right_bottom_coords[3])};
        // }
        // // Search for the boundary top right 10 flag
        // if (see_message[i].find("(t r 10)") != string::npos)
        // {
        //     vector<string> goal_right_bottom_coords = separate_string_separator(see_message[i], " ");
        //     boundaries.top_right_10 = {stof(goal_right_bottom_coords[3]), stof(goal_right_bottom_coords[4])};
        // }
        // // Search for the boundary top right 20 flag
        // if (see_message[i].find("(t r 20)") != string::npos)
        // {
        //     vector<string> goal_right_bottom_coords = separate_string_separator(see_message[i], " ");
        //     boundaries.top_right_20 = {stof(goal_right_bottom_coords[3]), stof(goal_right_bottom_coords[4])};
        // }
        // // Search for the boundary top right 30 flag
        // if (see_message[i].find("(t r 30)") != string::npos)
        // {
        //     vector<string> goal_right_bottom_coords = separate_string_separator(see_message[i], " ");
        //     boundaries.top_right_30 = {stof(goal_right_bottom_coords[3]), stof(goal_right_bottom_coords[4])};
        // }
        // // Search for the boundary top right 40 flag
        // if (see_message[i].find("(t r 40)") != string::npos)
        // {
        //     vector<string> goal_right_bottom_coords = separate_string_separator(see_message[i], " ");
        //     boundaries.top_right_40 = {stof(goal_right_bottom_coords[3]), stof(goal_right_bottom_coords[4])};
        // }
        // // Search for the boundary top right 50 flag
        // if (see_message[i].find("(t r 50)") != string::npos)
        // {
        //     vector<string> goal_right_bottom_coords = separate_string_separator(see_message[i], " ");
        //     boundaries.top_right_50 = {stof(goal_right_bottom_coords[3]), stof(goal_right_bottom_coords[4])};
        // }
        // // Search for the boundary bottom left 50 flag
        // if (see_message[i].find("(b l 50)") != string::npos)
        // {
        //     vector<string> goal_right_bottom_coords = separate_string_separator(see_message[i], " ");
        //     boundaries.bot_left_50 = {stof(goal_right_bottom_coords[3]), stof(goal_right_bottom_coords[4])};
        // }
        // // Search for the boundary bottom left 40 flag
        // if (see_message[i].find("(b l 40)") != string::npos)
        // {
        //     vector<string> goal_right_bottom_coords = separate_string_separator(see_message[i], " ");
        //     boundaries.bot_left_40 = {stof(goal_right_bottom_coords[3]), stof(goal_right_bottom_coords[4])};
        // }
        // // Search for the boundary bottom left 30 flag
        // if (see_message[i].find("(b l 30)") != string::npos)
        // {
        //     vector<string> goal_right_bottom_coords = separate_string_separator(see_message[i], " ");
        //     boundaries.bot_left_30 = {stof(goal_right_bottom_coords[3]), stof(goal_right_bottom_coords[4])};
        // }
        // // Search for the boundary bottom left 20 flag
        // if (see_message[i].find("(b l 20)") != string::npos)
        // {
        //     vector<string> goal_right_bottom_coords = separate_string_separator(see_message[i], " ");
        //     boundaries.bot_left_20 = {stof(goal_right_bottom_coords[3]), stof(goal_right_bottom_coords[4])};
        // }
        // // Search for the boundary bottom left 10 flag
        // if (see_message[i].find("(b l 10)") != string::npos)
        // {
        //     vector<string> goal_right_bottom_coords = separate_string_separator(see_message[i], " ");
        //     boundaries.bot_left_10 = {stof(goal_right_bottom_coords[3]), stof(goal_right_bottom_coords[4])};
        // }
        // // Search for the boundary bottom 0 flag
        // if (see_message[i].find("(b 0)") != string::npos)
        // {
        //     vector<string> goal_right_bottom_coords = separate_string_separator(see_message[i], " ");
        //     boundaries.bot_0 = {stof(goal_right_bottom_coords[2]), stof(goal_right_bottom_coords[3])};
        // }
        // // Search for the boundary bottom right 10 flag
        // if (see_message[i].find("(b r 10)") != string::npos)
        // {
        //     vector<string> goal_right_bottom_coords = separate_string_separator(see_message[i], " ");
        //     boundaries.bot_right_10 = {stof(goal_right_bottom_coords[3]), stof(goal_right_bottom_coords[4])};
        // }
        // // Search for the boundary bottom right 20 flag
        // if (see_message[i].find("(b r 20)") != string::npos)
        // {
        //     vector<string> goal_right_bottom_coords = separate_string_separator(see_message[i], " ");
        //     boundaries.bot_right_20 = {stof(goal_right_bottom_coords[3]), stof(goal_right_bottom_coords[4])};
        // }
        // // Search for the boundary bottom right 30 flag
        // if (see_message[i].find("(b r 30)") != string::npos)
        // {
        //     vector<string> goal_right_bottom_coords = separate_string_separator(see_message[i], " ");
        //     boundaries.bot_right_30 = {stof(goal_right_bottom_coords[3]), stof(goal_right_bottom_coords[4])};
        // }
        // // Search for the boundary bottom right 40 flag
        // if (see_message[i].find("(b r 40)") != string::npos)
        // {
        //     vector<string> goal_right_bottom_coords = separate_string_separator(see_message[i], " ");
        //     boundaries.bot_right_40 = {stof(goal_right_bottom_coords[3]), stof(goal_right_bottom_coords[4])};
        // }
        // // Search for the boundary bottom right 50 flag
        // if (see_message[i].find("(b r 50)") != string::npos)
        // {
        //     vector<string> goal_right_bottom_coords = separate_string_separator(see_message[i], " ");
        //     boundaries.bot_right_50 = {stof(goal_right_bottom_coords[3]), stof(goal_right_bottom_coords[4])};
        // }
    }
}

/**
 * Verifica si un jugador está en su posición asignada en el campo
 * Versión simplificada para evitar condiciones demasiado restrictivas
 *
 * @param field Información del campo y distancias a banderas
 * @param player Jugador a verificar
 * @param own_goal Portería propia del jugador
 * @param opponent_goal Portería contraria
 * @return true si el jugador está en su posición, false en caso contrario
 */
bool estasentusitio(const Field &field, const Player &player, const Goal &own_goal, const Goal &opponent_goal)
{
    // Si el campo no tiene datos válidos, considerar que no está en posición
    if (field.flag_center_distance <= 0.1) {
        return false;
    }

    // Lado derecho (equipo que juega de derecha a izquierda, portería en -52.5)
    if (player.side == "r")
    {
        switch (player.unum)
        {
        case 1: // Portero (lado derecho)
            // Portero debe estar cerca de su portería derecha
            // Condiciones más realistas y menos restrictivas
            if (field.flag_goal_right_top_distance < 15 ||
                field.flag_goal_right_bottom_distance < 15 ||
                field.flag_penalty_right_center_distance < 20)
            {
                return true;
            }
            // Alternativa: si está cerca del centro del área
            if (field.flag_center_distance > 40 && field.flag_center_distance < 55)
            {
                return true;
            }
            break;

        case 2: // Defensa derecho (equipo derecho)
            // Zona defensiva derecha
            if (field.flag_right_top_distance < 40 &&
                field.flag_center_distance > 25)
            {
                return true;
            }
            break;

        case 3: // Defensa central derecho
            // Centro de la defensa, lado derecho
            if (field.flag_center_distance > 20 &&
                field.flag_center_distance < 40 &&
                abs(field.flag_center_top_angle) < 45)
            {
                return true;
            }
            break;

        case 4: // Defensa central izquierdo
            // Centro de la defensa, lado izquierdo
            if (field.flag_center_distance > 20 &&
                field.flag_center_distance < 40 &&
                abs(field.flag_center_top_angle) < 45)
            {
                return true;
            }
            break;

        case 5: // Defensa izquierdo
            // Zona defensiva izquierda
            if (field.flag_left_top_distance < 40 &&
                field.flag_center_distance > 25)
            {
                return true;
            }
            break;

        case 6: // Mediocampista derecho
            // Mediocampista zona derecha
            if (field.flag_right_top_distance > 20 &&
                field.flag_right_top_distance < 50 &&
                field.flag_center_distance > 30)
            {
                return true;
            }
            break;

        case 7: // Delantero derecho
            // Delantero ala derecha
            if (field.flag_right_top_distance > 30 &&
                field.flag_right_top_distance < 60 &&
                field.flag_center_distance > 35)
            {
                return true;
            }
            break;

        case 8: // Mediocampista izquierdo
            // Mediocampista zona izquierda
            if (field.flag_left_top_distance > 20 &&
                field.flag_left_top_distance < 50 &&
                field.flag_center_distance > 30)
            {
                return true;
            }
            break;

        case 9: // Delantero centro
            // Delantero central
            if (field.flag_center_distance > 35 &&
                field.flag_center_distance < 55 &&
                abs(field.flag_center_top_angle) < 30)
            {
                return true;
            }
            break;

        case 10: // Mediocampista central ofensivo
            // Mediocampista ofensivo
            if (field.flag_center_distance > 25 &&
                field.flag_center_distance < 45 &&
                abs(field.flag_center_top_angle) < 35)
            {
                return true;
            }
            break;

        case 11: // Delantero izquierdo
            // Delantero ala izquierda
            if (field.flag_left_top_distance > 30 &&
                field.flag_left_top_distance < 60 &&
                field.flag_center_distance > 35)
            {
                return true;
            }
            break;

        default:
            // Para cualquier otro caso (no debería ocurrir)
            if (field.flag_center_distance > 15 && field.flag_center_distance < 60)
            {
                return true;
            }
            break;
        }
    }
    // Lado izquierdo (equipo que juega de izquierda a derecha, portería en 52.5)
    else if (player.side == "l")
    {
        switch (player.unum)
        {
        case 1: // Portero (lado izquierdo)
            // Portero debe estar cerca de su portería izquierda
            if (field.flag_goal_left_top_distance < 15 ||
                field.flag_goal_left_bottom_distance < 15 ||
                field.flag_penalty_left_center_distance < 20)
            {
                return true;
            }
            // Alternativa: si está cerca del centro del área
            if (field.flag_center_distance > 40 && field.flag_center_distance < 55)
            {
                return true;
            }
            break;

        case 2: // Defensa izquierdo (equipo izquierdo)
            // Zona defensiva izquierda
            if (field.flag_left_top_distance < 40 &&
                field.flag_center_distance > 25)
            {
                return true;
            }
            break;

        case 3: // Defensa central izquierdo
            // Centro de la defensa, lado izquierdo
            if (field.flag_center_distance > 20 &&
                field.flag_center_distance < 40 &&
                abs(field.flag_center_top_angle) < 45)
            {
                return true;
            }
            break;

        case 4: // Defensa central derecho
            // Centro de la defensa, lado derecho
            if (field.flag_center_distance > 20 &&
                field.flag_center_distance < 40 &&
                abs(field.flag_center_top_angle) < 45)
            {
                return true;
            }
            break;

        case 5: // Defensa derecho
            // Zona defensiva derecha
            if (field.flag_right_top_distance < 40 &&
                field.flag_center_distance > 25)
            {
                return true;
            }
            break;

        case 6: // Mediocampista izquierdo
            // Mediocampista zona izquierda
            if (field.flag_left_top_distance > 20 &&
                field.flag_left_top_distance < 50 &&
                field.flag_center_distance > 30)
            {
                return true;
            }
            break;

        case 7: // Delantero izquierdo
            // Delantero ala izquierda
            if (field.flag_left_top_distance > 30 &&
                field.flag_left_top_distance < 60 &&
                field.flag_center_distance > 35)
            {
                return true;
            }
            break;

        case 8: // Mediocampista derecho
            // Mediocampista zona derecha
            if (field.flag_right_top_distance > 20 &&
                field.flag_right_top_distance < 50 &&
                field.flag_center_distance > 30)
            {
                return true;
            }
            break;

        case 9: // Delantero centro
            // Delantero central
            if (field.flag_center_distance > 35 &&
                field.flag_center_distance < 55 &&
                abs(field.flag_center_top_angle) < 30)
            {
                return true;
            }
            break;

        case 10: // Mediocampista central ofensivo
            // Mediocampista ofensivo
            if (field.flag_center_distance > 25 &&
                field.flag_center_distance < 45 &&
                abs(field.flag_center_top_angle) < 35)
            {
                return true;
            }
            break;

        case 11: // Delantero derecho
            // Delantero ala derecha
            if (field.flag_right_top_distance > 30 &&
                field.flag_right_top_distance < 60 &&
                field.flag_center_distance > 35)
            {
                return true;
            }
            break;

        default:
            // Para cualquier otro caso
            if (field.flag_center_distance > 15 && field.flag_center_distance < 60)
            {
                return true;
            }
            break;
        }
    }

    // CONDICIÓN DE FALBACK - Si no coincide con ningún caso específico
    // Para evitar que los jugadores nunca se consideren "en posición"
    // Condición más genérica basada en distancia al centro
    if (field.flag_center_distance > 10 && field.flag_center_distance < 65)
    {
        return true;
    }

    // Si no cumple ninguna condición, no está en posición
    return false;
}

string dash(double power, double angle)
{
    std::string dash_command = "(dash " + to_string(power) + " " + to_string(angle) + ")";
    return dash_command;
}

void store_data_hear(string &hear_message, Player &player, MinimalSocket::udp::Udp<true> &udp_socket, MinimalSocket::Address const &server_udp, Ball &ball)
{
    vector<string> aux_hear_message = separate_string(hear_message); // hear 0 referee kick_off_l
    vector<string> valores_mensaje_Hear;
    for (size_t i = 0; i < aux_hear_message.size(); i++)
    {
        if (aux_hear_message[i].find("hear") != string::npos)
        {
            valores_mensaje_Hear = separate_string_separator(aux_hear_message[i], " ");
            cout << "TIME: " << valores_mensaje_Hear[1] << endl;
            cout << "REFEREE: " << valores_mensaje_Hear[2] << endl;
            cout << "MODO: " << valores_mensaje_Hear[3] << endl;
            player.playmode_prev = player.playmode; //metemos en player el modo de juego anterior
            std::cout << "MODO ANTERIRO: " << player.playmode_prev << std::endl;
            player.playmode = valores_mensaje_Hear[3]; //metemos en player el modo de juego
            std::cout << "MODO ACTUAL: " << player.playmode << std::endl;

            if (valores_mensaje_Hear[3].find("goal") != string::npos)
            {
                // Paso 1: Encontrar la última posición del '_'
                size_t lastUnderscorePos_L = valores_mensaje_Hear[3].rfind('_');

                std::string numberStr_L = valores_mensaje_Hear[3].substr(lastUnderscorePos_L + 1);
                // std::string numberStr_R = valores_mensaje_Hear[3].substr(lastUnderscorePos_R + 1);
                player.jugadorMarcaGol = numberStr_L;
                //player.jugadorMarcaGol = numberStr_R;
                std::cout << "Número extraído: " << player.jugadorMarcaGol << std::endl;
            }

            //funcion_modos_juego(player.playmode, player, udp_socket, server_udp, ball);
        }
    }
}

void chutarPorteria(Player &player, Ball &ball, Goal &opponent_goal, MinimalSocket::udp::Udp<true> &udp_socket, MinimalSocket::Address const &server_udp)
{
    if (ball.distance < 1)
    {
        float angle = opponent_goal.angle; // Ángulo hacia la portería contraria
        int power = 100;                   // Potencia del chute
        std::string kick_command = "(kick " + to_string(power) + " " + to_string(angle) + ")";
        udp_socket.sendTo(kick_command, server_udp); // Enviar comando de chute}
    }
    else
    {
        int i = 0;
        if (abs(ball.angle) >= 10)
        {
            int division = 1;
            if (ball.distance < 6)
            {
                division = 50;
            }
            else
            {
                division = 5;
            }
            // Rotate the player to the ball
            std::string rotate_command = "(turn " + to_string(ball.angle / division) + ")";
            udp_socket.sendTo(rotate_command, server_udp);
        }

        else
        {
            int power = 100;
            if (ball.distance < 3)
            {
                power = 60;
            }
            else if (ball.distance < 7)
            {
                power = 80;
            }
            // In this moment, the player should be looking to the ball
            // Create the dash command
            std::string dash_command = "(dash " + to_string(power) + " 0)";
            udp_socket.sendTo(dash_command, server_udp);
        }
    }
}

void pase(Player &player, Ball &ball,JugadorCercano &jugador, MinimalSocket::udp::Udp<true> &udp_socket, MinimalSocket::Address const &server_udp)
{
    if (ball.distance < 1)
    {
        float angle = jugador.angle; // Ángulo hacia la portería contraria
        int power = 50;                   // Potencia del chute
        std::string kick_command = "(kick " + to_string(power) + " " + to_string(angle) + ")";
        udp_socket.sendTo(kick_command, server_udp); // Enviar comando de chute}
    }
    else
    {
        int i = 0;
        if (abs(ball.angle) >= 10)
        {
            int division = 1;
            if (ball.distance < 6)
            {
                division = 50;
            }
            else
            {
                division = 5;
            }
            // Rotate the player to the ball
            std::string rotate_command = "(turn " + to_string(ball.angle / division) + ")";
            udp_socket.sendTo(rotate_command, server_udp);
        }

        else
        {
            int power = 100;
            if (ball.distance < 3)
            {
                power = 60;
            }
            else if (ball.distance < 7)
            {
                power = 80;
            }
            // In this moment, the player should be looking to the ball
            // Create the dash command
            std::string dash_command = "(dash " + to_string(power) + " 0)";
            udp_socket.sendTo(dash_command, server_udp);
        }
    }
}

/*
void configurePlayer(Player &player) // la  mitad de los jugadores de la derecha no estan en zona al iniciar
{
    vector<Posicion>
        posiciones = {{-50, 0},
                      {-40, -10},
                      {-35, -28},
                      {-40, 10},
                      {-35, 28},
                      {-25, 11},
                      {-8, 20},
                      {-25, -11},
                      {-5, 0},
                      {-15, 0},
                      {-8, -20}};

    const std::vector<Posicion> flags_config =
        {
            {0, 0},         // Center of the field
            {0, -33.5},     // Top center
            {0, 33.5},      // Bottom center
            {-52.5, -33.5}, // Corner top-left
            {-52.5, 33.5},  // Corner bottom-left
            {52.5, -33.5},  // Corner top-right
            {52.5, 33.5},   // Corner bottom-right
            {-36, -20},     // Penalty top-left 7
            {-36, 0},       // Penalty center-left
            {-36, 20},      // Penalty bottom-left 9
            {36, -20},      // Penalty top-right
            {36, 0},        // Penalty center-right
            {36, 20},       // Penalty bottom-right
            {-52.5, -7.32}, // Goal top-left
            {-52.5, 7.32},  // Goal bottom-left
            {52.5, -7.32},  // Goal top-right 15
            {52.5, 7.32}    // Goal bottom-right 16
        };
    if (player.unum == 1)
    {
        if (player.side == "r")
        {
            player.rol = "PORTERO";
            player.range = 10;
            player.zone = {50, 0};
            player.zone_name = "g r";
        }
        else
        {
            player.rol = "PORTERO";
            player.range = 10;
            player.zone = posiciones[player.unum - 1];
            player.zone_name = "g l";
        }
    }
    else if (player.unum == 3)
    {
        if (player.side == "r")
        {
            player.rol = "DEFENSA";
            player.range = 20;
            player.zone = {50, 20};
            player.zone_name = "(f " + player.side + " b)";
        }
        else
        {
            player.rol = "DEFENSA";
            player.range = 20;
            player.zone = {-50,-20};
            player.zone_name = "(f " + player.side + " t)";
        }
    }
    else if (player.unum == 5)
    {
        if (player.side == "r")
        {
            player.rol = "DEFENSA";
            player.range = 20;
            player.zone = {50, -20};
            player.zone_name = "(f " + player.side + " t)";
        }
        else
        {
            player.rol = "DEFENSA";
            player.range = 20;
            player.zone = {-30,30};
            player.zone_name = "(f " + player.side + " b)";
        }
    }
    else if (player.unum == 2)
    {
        if (player.side == "r")
        {
            player.rol = "DEFENSA";
            player.range = 20;
            player.zone = {50, 10};
            player.zone_name = "(f p" + player.side + " b)";
        }
        else
        {
            player.rol = "DEFENSA";
            player.range = 20;
            player.zone = {-50,-10};
            player.zone_name = "(f p " + player.side + " t)";
        }
    }
    else if (player.unum == 4)
    {
        if (player.side == "r")
        {
            player.rol = "DEFENSA";
            player.range = 20;
            player.zone = {50, -10};
            player.zone_name = "(f p " + player.side + " t)";
        }
        else
        {
            player.rol = "DEFENSA";
            player.range = 20;
            player.zone = {-35,20};
            player.zone_name = "(f p " + player.side + " b)";
        }
    }
    else if (player.unum == 8)
    {
        if (player.side == "r")
        {
            player.rol = "DEFENSA";
            player.range = 30;
            player.zone = {15, 25};
            player.zone_name = "(f p " + player.side + " b)";
        }
        else
        {
            player.rol = "DEFENSA";
            player.range = 30;
            player.zone = {-30, -20};
            player.zone_name = "(f p " + player.side + " t)";
        }
    }
    else if (player.unum == 6)
    {
        if (player.side == "r")
        {
            player.rol = "DEFENSA";
            player.range = 30;
            player.zone = {30, -10};
            player.zone_name = "(f p " + player.side + " t)";
        }
        else
        {
            player.rol = "DEFENSA";
            player.range = 30;
            player.zone = {-15,20};
            player.zone_name = "(f p " + player.side + " b)";
        }
    }
    else if (player.unum == 9)
    {
        if (player.side == "r")
        {
            player.rol = "DELANTERO";
            player.range = 20;
            player.zone = {10, 0};
            player.zone_name = "(f p r c)";
        }
        else
        {
            player.rol = "DELANTERO";
            player.range = 20;
            player.zone = {-10, 0};
            player.zone_name = "(f p l c)";
        }
    }
    else if (player.unum == 10)
    {
        if (player.side == "r")
        {
            player.rol = "DELANTERO";
            player.range = 15;
            player.zone = {20, 0};
            player.zone_name = "(f c)";
        }
        else
        {
            player.rol = "DELANTERO";
            player.range = 15;
            player.zone = posiciones[player.unum - 1];
            player.zone_name = "(f c)";
        }
    }
    else if (player.unum == 7)
    {
        if (player.side == "r")
        {
            player.rol = "DELANTERO";
            player.range = 30;
            player.zone = {10, -20};
            player.zone_name = "(f c t)";
        }
        else
        {
            player.rol = "DELANTERO";
            player.range = 30;
            player.zone = {-15,20};
            player.zone_name = "(f c b)";
        }
    }
    else if (player.unum == 11)
    {
        if (player.side == "r")
        {
            player.rol = "DELANTERO";
            player.range = 30;
            player.zone = {10, 25};
            player.zone_name = "(f c b)";
        }
        else
        {
            player.rol = "DELANTERO";
            player.range = 30;
            player.zone = {-10,-20};
            player.zone_name = "(f c t)";
        }
    }
    else
    {
        player.rol = "DELANTERO";
        player.range = 65;
        player.zone = flags_config[11];
        player.zone_name = "(f c)";
    }
}

*/

string returnToZone(Player const &player)
{
    // cout << "Not in zone" << endl; esto es mentira
    if (player.seeing_zone == false)
    {
        std::string rotate_command = "(turn " + to_string(15) + ")";
        return rotate_command;
    }
    else
    {
        if (player.flags_seen <= 3)
        {
            std::string dash_command = "(turn 15)";
            return dash_command;
        }
        else
        {
            std::string dash_command = "(dash 100 0)";
            return dash_command;
        }
    }
    // el portero no sabe volver porque no tiene flag
}

/*
void imInZone(Player &player)
{
    player.distancia_a_zona = sqrt(pow(player.x - player.zone.x, 2) + pow(player.y - player.zone.y, 2));
    if (player.distancia_a_zona<player.range&&player.flags_seen>=3)
    {
        player.in_zone = true;
    }
    else
    {
        player.in_zone = false;
    }
}
*/

void store_data_senseBody(string &senseBody_message, Player &player) //(sense_body 313 (view_mode high normal) (stamina 7985 1 126795))
{
    vector<string> parse_message = separate_string(senseBody_message); // sense_body 313 (view_mode high normal) (stamina 7985 1 126795)
    vector<string> valores;
    if (parse_message[0].find("sense_body") != string::npos)
    {
        vector<string> senseBodu_message = separate_string(parse_message[0]);

        for (size_t i = 0; i < senseBodu_message.size(); i++)
        {
            if (senseBodu_message[1].find("stamina") != string::npos)
            {
                valores = separate_string_separator(senseBodu_message[1], " ");
                player.stamina = stof(valores[1]);
            }
        }
    }
}

void store_data_hear(string &hear_message, Player &player, MinimalSocket::udp::Udp<true> &udp_socket, MinimalSocket::Address const &server_udp)
{
    vector<string> aux_hear_message = separate_string(hear_message); // hear 0 referee kick_off_l
    vector<string> valores_mensaje_Hear;
    for (size_t i = 0; i < aux_hear_message.size(); i++)
    {
        if (aux_hear_message[i].find("hear") != string::npos)
        {
            valores_mensaje_Hear = separate_string_separator(aux_hear_message[i], " ");
            cout << "TIME: " << valores_mensaje_Hear[1] << endl;
            cout << "REFEREE: " << valores_mensaje_Hear[2] << endl;
            cout << "MODO: " << valores_mensaje_Hear[3] << endl;
            player.playmode_prev = player.playmode; //metemos en player el modo de juego anterior
            std::cout << "MODO ANTERIRO: " << player.playmode_prev << std::endl;
            player.playmode = valores_mensaje_Hear[3]; //metemos en player el modo de juego
            std::cout << "MODO ACTUAL: " << player.playmode << std::endl;

            if (valores_mensaje_Hear[3].find("goal") != string::npos)
            {
                // Paso 1: Encontrar la última posición del '_'
                size_t lastUnderscorePos_L = valores_mensaje_Hear[3].rfind('_');

                std::string numberStr_L = valores_mensaje_Hear[3].substr(lastUnderscorePos_L + 1);
                // std::string numberStr_R = valores_mensaje_Hear[3].substr(lastUnderscorePos_R + 1);
                player.jugadorMarcaGol = numberStr_L;
                //player.jugadorMarcaGol = numberStr_R;
                std::cout << "Número extraído: " << player.jugadorMarcaGol << std::endl;
            }

            //funcion_modos_juego(player.playmode, player, udp_socket, server_udp, ball);
        }
    }
}


JugadorCercano procesarJugadoresVisibles(const vector<string> &see_message, const Player &player)
{
    JugadorCercano jugador_mas_cercano;
    float menor_distancia = std::numeric_limits<float>::max();

    for (auto &obj : see_message)
    {
        if (obj.find("(p") != string::npos && obj.find("(p)") == string::npos)
        {
            string obj_copy = obj; // Hacemos una copia local del string
            vector<string> player_info = separate_string_separator(obj_copy, " ");
            if (player_info.size() < 5) continue; // Asegurar que hay suficientes elementos

            JugadorCercano jugador;
            jugador.nombreEquipo = player_info[1];
            jugador.dorsal = player_info[2];
            jugador.distancia = player_info[3];
            jugador.angle = stof(player_info[4]); // Convertir el ángulo a float
            jugador.distance = stof(jugador.distancia); // Convertir la distancia a float

            // Eliminar el último carácter si es un paréntesis ')'
            if (!jugador.dorsal.empty() && jugador.dorsal.back() == ')')
            {
                jugador.dorsal.pop_back();

                // Comprobar si el jugador es del mismo equipo
                if (jugador.nombreEquipo.find(player.team_name) != string::npos)
                {
                    // Verificar si es el jugador más cercano
                    if (jugador.distance < menor_distancia)
                    {
                        menor_distancia = jugador.distance;
                        jugador_mas_cercano = jugador;
                    }
                }
            }
        }
    }

    return jugador_mas_cercano;
}

void mostrarJugadorMasCercano(const JugadorCercano &jugador_mas_cercano)
{
    if (jugador_mas_cercano.dorsal.empty()) {
        std::cout << "No se encontraron jugadores cercanos." << std::endl;
        return;
    }

    std::cout << " " << std::endl;
    std::cout << "Jugador más cercano:" << std::endl;
    std::cout << "Nombre Equipo: " << jugador_mas_cercano.nombreEquipo << std::endl;
    std::cout << "Dorsal: " << jugador_mas_cercano.dorsal << std::endl;
    std::cout << "Distancia: " << jugador_mas_cercano.distancia << std::endl;
    std::cout << "Ángulo: " << jugador_mas_cercano.angle << std::endl;
    std::cout << " " << std::endl;
}

/*
//PENDIENTE
void sacar_balon(Player &player, MinimalSocket::udp::Udp<true> &udp_socket, MinimalSocket::Address const &server_udp, Ball const &ball, Boundaries const &boundaries)
{
    player.posicion_sacar = false;
    if (player.see_ball)
    {
        vector<vector<double>> boundaries_rel = {boundaries.left_top, boundaries.left_bot, boundaries.right_top, boundaries.right_bot, boundaries.top_left_50, boundaries.top_left_40, boundaries.top_left_30, boundaries.top_left_20, boundaries.top_left_10, boundaries.top_0, boundaries.top_right_10, boundaries.top_right_20, boundaries.top_right_30, boundaries.top_right_40, boundaries.top_right_50, boundaries.bot_left_50, boundaries.bot_left_40, boundaries.bot_left_30, boundaries.bot_left_20, boundaries.bot_left_10, boundaries.bot_0, boundaries.bot_right_10, boundaries.bot_right_20, boundaries.bot_right_30, boundaries.bot_right_40, boundaries.bot_right_50};
        vector<double> dest = boundaries_rel[0];
        double angle_aux = 999;
        for (auto elem : boundaries_rel)
        {
            if (elem[0] == 999) // The player is not seeing the boundary
            {
                continue;
            }
            else
            {
                double angle = atan2(elem[1], elem[0]) * 180 / M_PI;
                double distance = sqrt(pow(elem[0], 2) + pow(elem[1], 2));
                if (abs(ball.angle-angle) < angle_aux)
                {
                    angle_aux = abs(ball.angle-angle);
                    dest = elem;
                }
            }
        }
        double angle = atan2(dest[1], dest[0]) * 180 / M_PI;
        double distance = sqrt(pow(dest[0], 2) + pow(dest[1], 2));
        if (distance > 1.0)
        {
            if (angle > 10)
            {
                std::string rotate_command = "(turn 10)";
                udp_socket.sendTo(rotate_command, server_udp);
            }
            else
            {
                std::string dash_command = "(dash 100 0)";
                udp_socket.sendTo(dash_command, server_udp);
            }
        }
        else
        {
            player.posicion_sacar = true;
        }
    }
    else
    {
        // Rotate to find the ball
        if (player.y < 0)
        {
            std::string rotate_command = "(turn " + to_string(-80) + ")";
            udp_socket.sendTo(rotate_command, server_udp);
        }
        else
        {
            std::string rotate_command = "(turn " + to_string(80) + ")";
            udp_socket.sendTo(rotate_command, server_udp);
        }

    }

}
*/

/*
void funcion_modos_juego(const string &modo, Player &player, MinimalSocket::udp::Udp<true> &udp_socket, MinimalSocket::Address const &server_udp, Ball &ball, Goal &opponent_goal, Boundaries &boundaries)
{
    if ((player.playmode == "goal_l_" + player.jugadorMarcaGol) || (player.playmode == "goal_r_" + player.jugadorMarcaGol) || player.playmode == "half_time") //movemos a los jugadores a su posicion inicial
    {
        sendInitialMoveMessage(player, udp_socket, server_udp);
        configurePlayer(player);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    if ((player.playmode == "corner_kick_l" && player.side == "l") || (player.playmode == "corner_kick_r"  && player.side == "r"))
    {
        if (player.unum==9)
        {
            if (player.posicion_sacar==false)
            {
                sacar_balon(player, udp_socket, server_udp, ball, boundaries);
            }
            else
            {
                if (player.see_ball)
                {
                    chutarPorteria(player, ball, opponent_goal, udp_socket, server_udp);
                }
                else
                {
                    std::string rotate_command = "(turn " + to_string(80) + ")";
                    udp_socket.sendTo(rotate_command, server_udp);
                }
            }
        }
        else{
            udp_socket.sendTo(returnToZone(player), server_udp);
        }
    }




    if ((player.playmode == "kick_in_l" && player.side == "l") || (player.playmode == "kick_in_r"  && player.side == "r"))
    {
        if (player.unum==9)
        {
            if (player.posicion_sacar==false)
            {
                sacar_balon(player, udp_socket, server_udp, ball, boundaries);
            }
            else
            {
                if (player.see_ball)
                {
                    chutarPorteria(player, ball, opponent_goal, udp_socket, server_udp);
                }
                else
                {
                    std::string rotate_command = "(turn " + to_string(80) + ")";
                    udp_socket.sendTo(rotate_command, server_udp);
                }
            }
        }
        else
        {
            udp_socket.sendTo(returnToZone(player), server_udp);
        }
    }
}
*/

void logica_portero(Player &player, MinimalSocket::udp::Udp<true> &udp_socket, MinimalSocket::Address const &server_udp, Ball &ball, Field &field)
{
    if (player.see_ball)
    {
        if (ball.distance < 1.0)
        {
            if (ball.angle < 10 && ball.angle > -10)
            {
                std::string catch_command = "(catch " + to_string(ball.angle) + ")";
                udp_socket.sendTo(catch_command, server_udp);
            }
        }
        else
        {
            // Mantener la misma coordenada en y que la pelota
            if (stod(ball.y) > 0 && (field.flag_goal_left_top[1] < 0 || field.flag_goal_left_bottom[1] < 0))
            {
                std::string dash_command = "(dash 100 90)";
                udp_socket.sendTo(dash_command, server_udp);
            }
            else if (stod(ball.y) < 0 && (field.flag_goal_right_top[1] > 0 || field.flag_goal_right_bottom[1] > 0))
            {
                std::string dash_command = "(dash 100 -90)";
                udp_socket.sendTo(dash_command, server_udp);
            }
        }
    }
}
