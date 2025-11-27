#!/bin/bash

# Solicita al usuario que ingrese el nombre del equipo
echo "Ingrese el nombre del equipo: "
read TEAM_NAME

# Define la ruta del ejecutable y el puerto base
PLAYER_EXECUTABLE="./player"
BASE_PORT=8111  # Puerto base a partir del cual se asignarán puertos a los jugadores

# Verifica que el ejecutable del jugador exista antes de continuar
if [ ! -f "$PLAYER_EXECUTABLE" ]; then
    echo "ERROR: Compila primero con: cmake .. && make"
    exit 1  # Termina el script con código de error si no encuentra el ejecutable
fi

# Muestra mensaje de inicio con el nombre del equipo
echo "Iniciando equipo: $TEAM_NAME"

# Bucle que ejecuta los 11 jugadores del equipo
for i in {1..11}; do
    # Calcula el puerto para este jugador (base + número de jugador)
    PORT=$((BASE_PORT + i))
    
    # Muestra información sobre qué jugador se está iniciando
    echo "Jugador $i -> puerto $PORT"
    
    # Ejecuta el jugador en segundo plano con los parámetros:
    # - Nombre del equipo
    # - Puerto único para este jugador
    ./player "$TEAM_NAME" "$PORT" &
    
    # Pequeña pausa para evitar congestión al iniciar múltiples procesos
    sleep 0.3
done

# El script termina aquí, pero los jugadores continúan ejecutándose en segundo plano
