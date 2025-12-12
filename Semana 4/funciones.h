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

vector<string> parse_message_hear(const std::string &s);

void sendInitialKickOffMessage(Player &player, MinimalSocket::udp::Udp<true> &udp_socket, MinimalSocket::Address const &recipient);

optional<GoalInfo> parseGoalOpponent(const std::string& seeMsg, const std::string& mySide);

