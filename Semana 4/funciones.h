#pragma once
#include<iostream>
#include<vector>
#include<string>
#include <cmath>
#include"structs.h"
#include <MinimalSocket/udp/UdpSocket.h>
using namespace std;

Player parseInitialMessage(std::string &message, Player &player);

vector<string> separate_string(string & s);

vector<string> separate_string_separator(string & s, string separator);

void sendInitialMoveMessage(Player &player, MinimalSocket::udp::Udp<true> &udp_socket, MinimalSocket::Address const &recipient);

void store_data_see(vector<string> &see_message, Player &player, Ball &ball, Goal &own_goal, Goal &opponent_goal, Field &field);

bool estasentusitio (const Field &field, const Player &player, const Goal &own_goal, const Goal &opponent_goal);

string dash(double power, double angle);

void store_data_hear(string &hear_message, Player &player, MinimalSocket::udp::Udp<true> &udp_socket, MinimalSocket::Address const &server_udp, Ball &ball);

void chutarPorteria(Player &player, Ball &ball, Goal &opponent_goal, MinimalSocket::udp::Udp<true> &udp_socket, MinimalSocket::Address const &server_udp);

JugadorCercano procesarJugadoresVisibles(const vector<string> &see_message, const Player &player);

void mostrarJugadorMasCercano(const JugadorCercano &jugador_mas_cercano);

void configurePlayer(Player &player);

string returnToZone(Player const &player);

void imInZone(Player &player);

void pase(Player &player, Ball &ball,JugadorCercano &jugador, MinimalSocket::udp::Udp<true> &udp_socket, MinimalSocket::Address const &server_udp);

void store_data_senseBody(string &senseBody_message, Player &player);

// Función para procesar mensajes 'hear'
void store_data_hear(string &hear_message, Player &player, MinimalSocket::udp::Udp<true> &udp_socket, MinimalSocket::Address const &server_udp);

void funcion_modos_juego(const string &modo, Player &player, MinimalSocket::udp::Udp<true> &udp_socket, MinimalSocket::Address const &server_udp, Ball &ball, Goal &opponent_goal, Boundaries &boundaries);
