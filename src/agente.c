/**********************************************************************************************************************************************************
*            Pontificia Universidad Javeriana
*-Autores: Juliana Aguirre Ballesteros
*Juan Carlos Santamaria Orjuela
*Juan David Daza
*-Materia: Sistemas Operativos
*-Proyecto de Sistemas Operativos 2025-30
*-Docente: J.Corredor.
*
*Descripcion:
*-Este archivo implementa el proceso agente del sistema de reservas
*-El agente representa a un proceso cliente que envia solicitudes al servidor
*-El agente se registra primero con el servidor para recibir la hora actual y confirmar conexion
*-El agente lee un archivo csv que contiene solicitudes de familias con su hora y cantidad de personas
*-El agente envia cada solicitud al servidor usando pipes nominales
*-El agente espera la respuesta del servidor para saber si la reserva fue aceptada negada o reprogramada
*-El agente actualiza su hora actual segun la respuesta para no enviar solicitudes atrasadas
*-El agente espera dos segundos entre cada solicitud para simular tiempo real
*-Si el servidor anuncia fin del dia el agente deja de enviar solicitudes y termina su ejecucion
*-El agente crea su propio pipe de respuesta unico y lo elimina al finalizar su trabajo
*
*Objetivo:
*-Crear el proceso agente que actua como cliente dentro del sistema de reservas
*-Leer un archivo csv y convertir cada linea en una solicitud de reserva
*-Enviar solicitudes al servidor usando pipes nominales
*-Esperar respuestas del servidor y procesarlas de forma simple y clara
*-Manejar codigos de respuesta aceptada reprogramada extemporanea o sin cupo
*-Mantener la hora actual recibida del servidor para decidir si una solicitud es valida
*-Esperar hasta recibir el mensaje de fin del dia para terminar su ejecucion de forma correcta
****************************************************************************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "comunes.h"
#include "ipc_fifo.h"
#include "utilidades.h"

#define BUFFER_SIZE 256

/* Estado del agente */
struct EstadoAgente {
    char nombre[NOMBRE_MAX];
    char pipe_servidor[RUTA_MAX];
    char pipe_respuesta[RUTA_MAX];
    char archivo_csv[RUTA_MAX];
    int  hora_actual_servidor;
    int  fd_servidor;
    int  fd_respuesta;
};

/**
 * Registra el agente con el servidor (handshake inicial)
 * Retorna 0 si OK, -1 si falla
 */
static int registrar_con_servidor(struct EstadoAgente *ag) {
    struct MensajeSolicitud reg = {0};
    reg.operacion = OP_REGISTRO;
    strncpy(reg.nombre_agente, ag->nombre, sizeof(reg.nombre_agente) - 1);
    strncpy(reg.pipe_respuesta, ag->pipe_respuesta, sizeof(reg.pipe_respuesta) - 1);

    printf("[%s] Registrandose con el servidor...\n", ag->nombre);
    
    // Enviar mensaje de registro
    ag->fd_servidor = abrir_escritura_bloqueante(ag->pipe_servidor);
    if (ag->fd_servidor < 0) {
        perror("Error al abrir pipe del servidor");
        return -1;
    }
    
    if (escribir_bloque(ag->fd_servidor, &reg, sizeof(reg)) < 0) {
        perror("Error al enviar registro");
        cerrar_fd(ag->fd_servidor);
        return -1;
    }
    cerrar_fd(ag->fd_servidor);

    // Esperar respuesta (handshake)
    ag->fd_respuesta = abrir_lectura_bloqueante(ag->pipe_respuesta);
    if (ag->fd_respuesta < 0) {
        perror("Error al abrir pipe de respuesta");
        return -1;
    }

    struct MensajeRespuesta resp = {0};
    ssize_t n = leer_bloque(ag->fd_respuesta, &resp, sizeof(resp));
    if (n <= 0) {
        perror("Error al leer handshake");
        cerrar_fd(ag->fd_respuesta);
        return -1;
    }
    cerrar_fd(ag->fd_respuesta);

    ag->hora_actual_servidor = resp.hora_actual;
    printf("[%s] Registrado OK. Hora actual del servidor: %d\n", ag->nombre, ag->hora_actual_servidor);
    
    return 0;
}

/**
 * Envia una solicitud de reserva al servidor
 */
static int enviar_solicitud(struct EstadoAgente *ag, const char *familia, int hora, int personas) {
    struct MensajeSolicitud sol = {0};
    sol.operacion = OP_SOLICITUD;
    strncpy(sol.nombre_agente, ag->nombre, sizeof(sol.nombre_agente) - 1);
    strncpy(sol.nombre_familia, familia, sizeof(sol.nombre_familia) - 1);
    strncpy(sol.pipe_respuesta, ag->pipe_respuesta, sizeof(sol.pipe_respuesta) - 1);
    sol.hora_solicitada = hora;
    sol.cantidad_personas = personas;

    printf("[%s] Solicitando reserva: Familia=%s Hora=%d Personas=%d\n", 
           ag->nombre, familia, hora, personas);

    // Enviar solicitud
    ag->fd_servidor = abrir_escritura_bloqueante(ag->pipe_servidor);
    if (ag->fd_servidor < 0) {
        perror("Error al abrir pipe del servidor");
        return -1;
    }

    if (escribir_bloque(ag->fd_servidor, &sol, sizeof(sol)) < 0) {
        perror("Error al enviar solicitud");
        cerrar_fd(ag->fd_servidor);
        return -1;
    }
    cerrar_fd(ag->fd_servidor);

    return 0;
}

/**
 * Recibe y procesa la respuesta del servidor
 * Retorna 0 si debe continuar 1 si es fin de dia -1 si error
 */
static int recibir_respuesta(struct EstadoAgente *ag) {
    ag->fd_respuesta = abrir_lectura_bloqueante(ag->pipe_respuesta);
    if (ag->fd_respuesta < 0) {
        perror("Error al abrir pipe de respuesta");
        return -1;
    }

    struct MensajeRespuesta resp = {0};
    ssize_t n = leer_bloque(ag->fd_respuesta, &resp, sizeof(resp));
    if (n <= 0) {
        perror("Error al leer respuesta");
        cerrar_fd(ag->fd_respuesta);
        return -1;
    }
    cerrar_fd(ag->fd_respuesta);

    // Verificar si es fin de dia
    if (resp.operacion == OP_FIN_DIA) {
        printf("[%s] Servidor notifica FIN DEL DIA: %s\n", ag->nombre, resp.detalle);
        return 1;
    }

    // Actualizar hora actual del servidor
    ag->hora_actual_servidor = resp.hora_actual;

    // Imprimir respuesta segun el codigo
    switch (resp.codigo_respuesta) {
        case RESP_ACEPTADA:
            printf("[%s] RESERVA ACEPTADA en hora solicitada. Hora actual servidor: %d\n", 
                   ag->nombre, resp.hora_actual);
            break;
        
        case RESP_REPROGRAMADA:
            printf("[%s] RESERVA REPROGRAMADA para hora %d (hora actual servidor: %d)\n", 
                   ag->nombre, resp.hora_asignada, resp.hora_actual);
            break;
        
        case RESP_NEGADA_EXTEMPORANEA:
            printf("[%s] RESERVA NEGADA extemporanea. Hora actual servidor: %d\n", 
                   ag->nombre, resp.hora_actual);
            break;
        
        case RESP_NEGADA_SIN_CUPO:
            printf("[%s] RESERVA NEGADA sin cupo o fuera de rango. Debe volver otro dia.\n", 
                   ag->nombre);
            break;
        
        default:
            printf("[%s] Respuesta desconocida codigo=%d\n", ag->nombre, resp.codigo_respuesta);
            break;
    }

    return 0;
}

/**
 * Procesa el archivo CSV linea por linea
 */
static int procesar_archivo_csv(struct EstadoAgente *ag) {
    FILE *fp = fopen(ag->archivo_csv, "r");
    if (!fp) {
        fprintf(stderr, "[%s] Error al abrir archivo CSV: %s\n", ag->nombre, ag->archivo_csv);
        perror("fopen");
        return -1;
    }

    printf("[%s] Leyendo archivo: %s\n", ag->nombre, ag->archivo_csv);

    char linea[BUFFER_SIZE];
    int num_linea = 0;

    while (fgets(linea, sizeof(linea), fp)) {
        num_linea++;
        
        // Remover salto de linea
        size_t len = strlen(linea);
        if (len > 0 && linea[len-1] == '\n') {
            linea[len-1] = '\0';
        }

        // Ignorar lineas vacias
        if (strlen(linea) == 0) continue;

        // Parsear CSV: Familia Hora Personas
        char familia[FAMILIA_MAX];
        int hora, personas;

        if (dividir_linea_csv(linea, familia, &hora, &personas) != 0) {
            fprintf(stderr, "[%s] Error al parsear linea %d: %s\n", ag->nombre, num_linea, linea);
            continue;
        }

        // Validar hora
        if (hora < ag->hora_actual_servidor) {
            printf("[%s] Advertencia hora %d < hora_actual %d. Enviando de todos modos.\n",
                   ag->nombre, hora, ag->hora_actual_servidor);
        }

        // Enviar solicitud
        if (enviar_solicitud(ag, familia, hora, personas) != 0) {
            fprintf(stderr, "[%s] Error al enviar solicitud para linea %d\n", ag->nombre, num_linea);
            continue;
        }

        // Recibir respuesta
        int rc = recibir_respuesta(ag);
        if (rc == 1) {
            fclose(fp);
            return 1;
        } else if (rc < 0) {
            fprintf(stderr, "[%s] Error al recibir respuesta para linea %d\n", ag->nombre, num_linea);
            continue;
        }

        printf("[%s] Esperando 2 segundos...\n", ag->nombre);
        sleep(2);
    }

    fclose(fp);
    
    printf("[%s] Archivo procesado completamente.\n", ag->nombre);
    return 0;
}

int main(int argc, char **argv) {
    struct EstadoAgente ag = {0};

    // Leer argumentos
    if (leer_argumentos_agente(argc, argv, ag.nombre, sizeof(ag.nombre),
                               ag.archivo_csv, sizeof(ag.archivo_csv),
                               ag.pipe_servidor, sizeof(ag.pipe_servidor)) != 0) {
        fprintf(stderr, "Uso: %s -s <nombre_agente> -a <archivo.csv> -p <pipe_servidor>\n", argv[0]);
        fprintf(stderr, "Ejemplo: %s -s AgenteA -a datos/agenteA.csv -p /tmp/pipeReservas\n", argv[0]);
        return 1;
    }

    // Construir pipe de respuesta unico
    snprintf(ag.pipe_respuesta, sizeof(ag.pipe_respuesta), "/tmp/pipe_%s_%d", ag.nombre, (int)getpid());

    // Crear pipe de respuesta
    if (crear_fifo_si_no_existe(ag.pipe_respuesta, 0660) != 0) {
        fprintf(stderr, "[%s] Error al crear pipe de respuesta: %s\n", ag.nombre, ag.pipe_respuesta);
        perror("mkfifo");
        return 2;
    }

    printf("[%s] Iniciando agente...\n", ag.nombre);
    printf("[%s] Pipe de respuesta: %s\n", ag.nombre, ag.pipe_respuesta);

    // Registrarse con servidor
    if (registrar_con_servidor(&ag) != 0) {
        fprintf(stderr, "[%s] Error en el registro con el servidor\n", ag.nombre);
        eliminar_fifo(ag.pipe_respuesta);
        return 3;
    }

    // Procesar archivo CSV
    int rc = procesar_archivo_csv(&ag);

    // Si termino el archivo y falta fin del dia esperar
    if (rc == 0) {
        printf("[%s] Esperando notificacion de fin de dia...\n", ag.nombre);
        while (1) {
            int r2 = recibir_respuesta(&ag);
            if (r2 == 1) {
                break;
            }
            if (r2 < 0) {
                fprintf(stderr, "[%s] Error esperando fin de dia\n", ag.nombre);
                break;
            }
        }
    }

    eliminar_fifo(ag.pipe_respuesta);

    if (rc < 0) {
        fprintf(stderr, "[%s] Error al procesar archivo\n", ag.nombre);
        return 4;
    }

    printf("Agente %s termina.\n", ag.nombre);
    return 0;
}

/********************************************************************************************************************************
Conclusiones
---------------
El agente cumple su papel como cliente del sistema de reservas y se comunica correctamente con el servidor
Realiza el registro inicial handshake y obtiene la hora actual del servidor de forma segura
Lee el archivo csv linea por linea y convierte cada entrada en una solicitud de reserva hacia el servidor
Envía cada solicitud por pipe nominal y espera la respuesta correspondiente antes de continuar
Interpreta los codigos de respuesta aceptada reprogramada extemporanea o sin cupo y los muestra de forma clara en consola
Respeta la hora actual del servidor al advertir solicitudes atrasadas y espera dos segundos entre solicitudes para simular tiempo real
Espera correctamente el mensaje de fin de dia antes de terminar y limpia su propio pipe de respuesta antes de salir

********************************************************************************************************************************/