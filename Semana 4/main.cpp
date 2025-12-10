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
