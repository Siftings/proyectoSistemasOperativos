/*********************************************************************************************************************************
*            Pontificia Universidad Javeriana
*-Autores: Juliana Aguirre Ballesteros
*Juan Carlos Santamaria Orjuela
*Juan David Daza
*-Materia: Sistemas Operativos
*-Proyecto de Sistemas Operativos 2025-30
*-Docente: J.Corredor.
*
*Descripcion:
*-Este archivo implementa el sistema de registro de mensajes del programa
*-Permite mostrar mensajes en pantalla con informacion del tiempo del proceso y del hilo
*-Cada mensaje incluye una marca de tiempo el pid y el tid
*-El archivo maneja informacion advertencias y errores
*-El archivo usa un mutex para evitar que varios hilos escriban al tiempo y mezclen los mensajes
*-Este sistema ayuda a depurar y entender el flujo del servidor y los agentes
*
*Objetivo:
*-Crear un metodo simple para imprimir mensajes con hora proceso y hilo
*-Asegurar que los mensajes no se mezclen usando un mutex
*-Ofrecer funciones para registrar informacion advertencias y errores de forma uniforme
*-Permitir que todas las partes del sistema usen el mismo estilo de log
***********************************************************************************************************************************/

#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include "registro_log.h"

// este mutex evita que dos hilos escriban al mismo tiempo
static pthread_mutex_t g_mtx = PTHREAD_MUTEX_INITIALIZER;

// esta funcion crea el prefijo de tiempo para cada mensaje
static void ts_prefix(char *buf, size_t n) {
    time_t t = time(NULL);                 // obtiene la hora actual
    struct tm tmv;                         // estructura para guardar la hora
    localtime_r(&t, &tmv);                 // convierte la hora
    snprintf(buf, n, "[%02d:%02d:%02d][pid=%d][tid=%lu]",
             tmv.tm_hour, tmv.tm_min, tmv.tm_sec, (int)getpid(),
             (unsigned long)pthread_self()); // escribe prefijo con hora pid y tid
}

// esta funcion imprime el mensaje con su nivel
static void log_emit(const char *lvl, const char *txt) {
    char ts[64];                           // buffer para el prefijo
    ts_prefix(ts, sizeof ts);              // crea el prefijo
    pthread_mutex_lock(&g_mtx);            // bloquea mutex
    fprintf(stdout, "%s[%s] %s\n", ts, lvl, txt); // imprime el mensaje
    fflush(stdout);                        // limpia el buffer
    pthread_mutex_unlock(&g_mtx);          // libera mutex
}

// esta funcion imprime mensaje de informacion
void registrar_info_texto(const char *texto)       { log_emit("INFO", texto); }

// esta funcion imprime mensaje de advertencia
void registrar_advertencia_texto(const char *texto){ log_emit("WARN", texto); }

// esta funcion imprime mensaje de error
void registrar_error_texto(const char *texto)      { log_emit("ERROR", texto); }


/*********************************************************************************************************************************
Conclusiones
------------
El sistema de registro de mensajes cumple su objetivo dentro del proyecto
Cada mensaje incluye hora pid y tid lo que facilita seguir el flujo de ejecucion
El uso de un unico formato para INFO WARN y ERROR da consistencia al log
El mutex global evita que varios hilos escriban al tiempo y mezclen las lineas
El modulo es sencillo de usar desde cualquier parte del codigo y no expone detalles internos

Recomendaciones
---------------
Seria util agregar niveles adicionales como DEBUG  para depuracion mas detallada
*********************************************************************************************************************************/
