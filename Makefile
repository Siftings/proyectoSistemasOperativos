#################################################################################
#            Pontificia Universidad Javeriana
#Autores: Juliana Aguirre Ballesteros
#Juan Carlos Santamaria Orjuela
#Juan David Daza
#Materia: Sistemas Operativos
#Proyecto de Sistemas Operativos 2025-30
#
#Descripcion:
#-Este archivo es el makefile que compila el servidor y el agente del sistema
#-Organiza directorios fuentes cabeceras y ejecutables
#-Define reglas para compilar enlazar limpiar y ejecutar ejemplos
#-Permite compilar todo el proyecto con un solo comando
#-Tambien crea el directorio bin si no existe
#
#Objetivo:
#-Automatizar la compilacion del servidor y del agente
#-Evitar compilar archivos uno por uno
#-Ofrecer comandos faciles para ejecutar y limpiar el proyecto
###############################################################################

CC = gcc
CFLAGS = -Wall -pthread -Iinclude

# Archivos objeto en build/
SERVER_OBJS = build/servidor.o build/agenda_reservas.o build/reloj_simulacion.o build/ipc_fifo.o build/registro_log.o build/utilidades.o
AGENT_OBJS = build/agente.o build/ipc_fifo.o build/utilidades.o

# Ejecutables en el directorio actual
all: bin/servidor bin/agente

bin/servidor: $(SERVER_OBJS)
	@mkdir -p bin
	$(CC) $(CFLAGS) $^ -o $@

bin/agente: $(AGENT_OBJS)
	@mkdir -p bin
	$(CC) $(CFLAGS) $^ -o $@

# Compilar cada .c a .o en build/
build/%.o: src/%.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f bin/servidor bin/agente build/*.o


###############################################################################
#Conclusiones
#------------
#El makefile organiza bien la compilacion del servidor y del agente en un solo lugar
#Permite compilar todo el proyecto con un solo comando de forma simple y ordenada
#Agrupa los objetos de cada componente y evita compilar archivos uno por uno
#Crea los directorios necesarios y mantiene el proyecto limpio y facil de ejecutar
###############################################################################
