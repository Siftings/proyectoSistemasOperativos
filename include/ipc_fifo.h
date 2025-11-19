/****************************************************************************************
*            Pontificia Universidad Javeriana
*Autores: Juliana Aguirre Ballesteros
*Juan Carlos Santamaria Orjuela
*Juan David Daza
*Materia: Sistemas Operativos
*Proyecto de Sistemas Operativos 2025-30
*-Docente: J.Corredor.
*
*Descripcion:
*-Este archivo define funciones simples para crear abrir leer escribir y borrar pipes FIFO
*-Los pipes permiten que el servidor y los agentes se comuniquen entre procesos
*-Las funciones permiten leer y escribir bloques completos de memoria
*-Tambien permiten cerrar descriptores y eliminar pipes cuando ya no se necesitan
*
*Objetivo:
*-Proveer funciones basicas para manejar pipes FIFO en el sistema
*-Permitir que los procesos puedan enviar y recibir mensajes de forma segura
*-Facilitar el manejo de lectura y escritura continua sin cortar los datos
*-Ofrecer funciones sencillas que el servidor y los agentes puedan usar sin problemas
*****************************************************************************************/

#ifndef IPC_FIFO_H
#define IPC_FIFO_H

#include <sys/types.h>  /* mode_t */
#include <sys/stat.h>   /* mkfifo */
#include <unistd.h>     /* read write close */
#include <stddef.h>     /* size_t */

// esta funcion crea un pipe fifo si no existe
int crear_fifo_si_no_existe(const char *ruta, mode_t modo);

// esta funcion abre un pipe en modo lectura
int abrir_lectura_bloqueante(const char *ruta);

// esta funcion abre un pipe en modo escritura
int abrir_escritura_bloqueante(const char *ruta);

// esta funcion lee un bloque completo del pipe
ssize_t leer_bloque(int fd, void *buf, size_t nbytes);

// esta funcion escribe un bloque completo en el pipe
ssize_t escribir_bloque(int fd, const void *buf, size_t nbytes);

// esta funcion cierra un descriptor de archivo
int cerrar_fd(int fd);

// esta funcion elimina un pipe fifo
int eliminar_fifo(const char *ruta);

#endif


/****************************************************************************************
Conclusiones
------------
El archivo entrega funciones basicas y claras para manejar pipes FIFO en todo el sistema
Permite crear abrir leer escribir y eliminar pipes de forma simple
Las funciones aseguran que los datos se lean y escriban completos sin cortarse
El servidor y los agentes pueden comunicarse sin problemas gracias a estas rutinas
****************************************************************************************/
