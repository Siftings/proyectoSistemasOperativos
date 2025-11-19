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
*-Este archivo implementa el proceso servidor del sistema de reservas
*-El servidor es el encargado de manejar todo lo que pasa en el parque durante el dia
*-El servidor recibe solicitudes de los agentes que representan familias que quieren entrar al parque
*-El servidor revisa si hay espacio en la hora que pide la familia y decide si acepta niega o cambia la reserva
*-El servidor usa un reloj de simulacion que avanza las horas y controla cuando entran y salen las familias del parque
*-Cada vez que pasa una hora el servidor actualiza la agenda del parque y registra lo que sucede
*-El servidor tambien guarda un registro simple de cada solicitud y de cada respuesta
*-Al final del dia el servidor imprime un reporte con la informacion del dia como horas mas llenas horas mas vacias y cuantas reservas fueron aceptadas o negadas
*
*Objetivo:
*-Crear el proceso servidor que controla todo el sistema de reservas
*-Hacer que el servidor reciba informacion de los agentes usando pipes
*-Revisar el aforo de cada hora para no superar el limite permitido
*-Manejar el reloj de simulacion para saber en que hora va el sistema
*-Procesar solicitudes de registro y solicitudes de reserva de forma ordenada
****************************************************************************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "comunes.h"
#include "ipc_fifo.h"
#include "agenda_reservas.h"
#include "reloj_simulacion.h"
#include "utilidades.h"
#include "registro_log.h"

/* Estado simple para reloj servidor */
// esta estructura guarda datos del servidor
struct EstadoServidor {
    struct AgendaReservas *agenda;   // esta variable guarda la agenda
    int hora_fin;                    // esta variable guarda la hora final
    int fin_dia;                     // esta variable marca cuando termina el dia
};

// esta funcion se ejecuta cada vez que pasa una hora
static void on_tick(int hora_actual, void *usr) {
    struct EstadoServidor *st = (struct EstadoServidor*)usr;
    char linea[128];
    snprintf(linea, sizeof linea, "Tick -> hora actual %d", hora_actual);
    registrar_info_texto(linea);
    aplicar_avance_de_hora(st->agenda, hora_actual);  // esta funcion actualiza la agenda
    if (hora_actual >= st->hora_fin) {
        st->fin_dia = 1;  // esta linea marca fin del dia
    }
}

#define MAX_AGENTES 256
static char g_agentes_pipes[MAX_AGENTES][RUTA_MAX]; // esta variable guarda los pipes de los agentes
static int  g_agentes_cnt = 0;                       // esta variable cuenta los agentes

// esta funcion registra el pipe de un agente
static void registrar_agente_pipe(const char *ruta_pipe) {
    if (!ruta_pipe || !*ruta_pipe) return;
    for (int i=0;i<g_agentes_cnt;i++) {
        if (strncmp(g_agentes_pipes[i], ruta_pipe, RUTA_MAX)==0) return; // si ya existe no hace nada
    }
    if (g_agentes_cnt < MAX_AGENTES) {
        strncpy(g_agentes_pipes[g_agentes_cnt], ruta_pipe, RUTA_MAX-1);
        g_agentes_pipes[g_agentes_cnt][RUTA_MAX-1]='\0';
        g_agentes_cnt++;  // aumenta el contador
    }
}

// esta funcion responde al registro inicial
static void responder_handshake(const char *pipe_resp, int hora_actual) {
    struct MensajeRespuesta r = {0};
    r.operacion = OP_RESPUESTA;          // tipo de respuesta
    r.codigo_respuesta = RESP_ACEPTADA;  // esta respuesta indica ok
    r.hora_asignada = 0;
    r.hora_actual = hora_actual;
    snprintf(r.detalle, sizeof r.detalle, "Registro OK. Hora actual %d", hora_actual);

    int fd = abrir_escritura_bloqueante(pipe_resp);  // abre el pipe para escribir
    if (fd >= 0) {
        escribir_bloque(fd, &r, sizeof r);   // envia respuesta
        cerrar_fd(fd);
    } else {
        registrar_advertencia_texto("No se pudo abrir pipe de respuesta del agente en registro");
    }
}

// esta funcion responde a una solicitud de reserva
static void responder_solicitud(const char *pipe_resp, int codigo, int hora_asignada, int hora_actual) {
    struct MensajeRespuesta r = {0};
    r.operacion = OP_RESPUESTA;   // tipo de mensaje

    switch (codigo) {  // esta parte decide la respuesta
        case 0: r.codigo_respuesta = RESP_ACEPTADA;            break;
        case 1: r.codigo_respuesta = RESP_REPROGRAMADA;        break;
        case -1:r.codigo_respuesta = RESP_NEGADA_EXTEMPORANEA; break;
        case -2:r.codigo_respuesta = RESP_NEGADA_SIN_CUPO;     break;
        default:r.codigo_respuesta = RESP_NEGADA_SIN_CUPO;     break;
    }

    r.hora_asignada = (codigo==1 || codigo==0) ? hora_asignada : 0;
    r.hora_actual = hora_actual;

    snprintf(r.detalle, sizeof r.detalle, "Resp=%d hora_asig=%d hora_act=%d",
             r.codigo_respuesta, r.hora_asignada, r.hora_actual);

    int fd = abrir_escritura_bloqueante(pipe_resp);  // abre pipe
    if (fd >= 0) {
        escribir_bloque(fd, &r, sizeof r);  // envia mensaje
        cerrar_fd(fd);
    } else {
        registrar_error_texto("No se pudo abrir pipe de respuesta del agente");
    }
}

// esta funcion avisa fin del dia a todos los agentes
static void notificar_fin_a_agentes(void) {
    struct MensajeRespuesta r = {0};
    r.operacion = OP_FIN_DIA;
    r.codigo_respuesta = RESP_ACEPTADA;
    strncpy(r.detalle, "FIN DEL DIA", sizeof r.detalle - 1);

    for (int i=0;i<g_agentes_cnt;i++) {   // recorre todos los pipes
        int fd = abrir_escritura_bloqueante(g_agentes_pipes[i]);
        if (fd >= 0) {
            escribir_bloque(fd, &r, sizeof r);
            cerrar_fd(fd);
        }
    }
}

// funcion principal
int main(int argc, char **argv) {
    int hora_ini=0, hora_fin=0, seg_hora=0, aforo=0;
    const char *pipe_recibe=NULL;   // este es el pipe de entrada

    // esta parte lee los argumentos
    if (leer_argumentos_servidor(argc, argv, &hora_ini, &hora_fin, &seg_hora, &aforo, &pipe_recibe)!=0 ||
        validar_parametros_servidor(hora_ini, hora_fin, seg_hora, aforo)!=0) {
        fprintf(stderr, "Uso: %s -i <horaIni> -f <horaFin> -s <segHoras> -t <aforo> -p <pipeRecibe>\n", argv[0]);
        return 1;
    }

    // esta parte crea la agenda
    struct AgendaReservas *agenda = crear_agenda_reservas(hora_ini, hora_fin, aforo);
    if (!agenda) { registrar_error_texto("No hay memoria para agenda"); return 2; }

    // esta parte crea el pipe principal
    if (crear_fifo_si_no_existe(pipe_recibe, 0660)!=0) {
        perror("mkfifo"); return 3;
    }
    int fd_in = abrir_lectura_bloqueante(pipe_recibe);  // abre pipe para leer
    if (fd_in < 0) { perror("open lectura"); eliminar_fifo(pipe_recibe); return 4; }

    // esta parte inicia el reloj
    struct EstadoServidor st = { .agenda = agenda, .hora_fin = hora_fin, .fin_dia = 0 };
    struct RelojSimulacion *reloj = iniciar_reloj_simulacion(hora_ini, hora_fin, seg_hora, on_tick, &st);
    if (!reloj) { registrar_error_texto("No se pudo iniciar el reloj"); cerrar_fd(fd_in); eliminar_fifo(pipe_recibe); return 5; }

    registrar_info_texto("Servidor listo. Esperando agentes y solicitudes...");

    // bucle principal
    while (!st.fin_dia) {
        struct MensajeSolicitud msg;  // esta variable recibe mensajes
        ssize_t n = leer_bloque(fd_in, &msg, sizeof msg);

        if (n == 0) {  // si llega fin de archivo vuelve a abrir
            cerrar_fd(fd_in);
            fd_in = abrir_lectura_bloqueante(pipe_recibe);
            if (fd_in < 0) break;
            continue;
        }
        if (n < 0) {
            if (errno == EINTR) continue;
            perror("read"); break;
        }
        if ((size_t)n != sizeof msg) {
            registrar_advertencia_texto("Mensaje con tamano inesperado");
            continue;
        }

        // esta parte maneja el registro
        if (msg.operacion == OP_REGISTRO) {
            registrar_agente_pipe(msg.pipe_respuesta);
            char linea[256];
            snprintf(linea, sizeof linea, "Registro agente %s pipe_resp %s", msg.nombre_agente, msg.pipe_respuesta);
            registrar_info_texto(linea);
            int hora_act = obtener_hora_actual_agenda(agenda);
            responder_handshake(msg.pipe_respuesta, hora_act);

        // esta parte maneja solicitudes
        } else if (msg.operacion == OP_SOLICITUD) {
            char linea[256];
            snprintf(linea, sizeof linea, "Solicitud de %s familia %s hora %d personas %d",
                     msg.nombre_agente, msg.nombre_familia, msg.hora_solicitada, msg.cantidad_personas);
            registrar_info_texto(linea);

            int hora_asig = 0;
            int rc = intentar_reservar_dos_horas(agenda, msg.nombre_familia,
                                                 msg.hora_solicitada, msg.cantidad_personas, &hora_asig);
            int codigo_resp;
            switch (rc) {
                case 0: codigo_resp = RESP_ACEPTADA; break;
                case 1: codigo_resp = RESP_REPROGRAMADA; break;
                case -1: codigo_resp = RESP_NEGADA_EXTEMPORANEA; break;
                default: codigo_resp = RESP_NEGADA_SIN_CUPO; break;
            }
            contar_respuesta_metricas(agenda, codigo_resp);
            int hora_act = obtener_hora_actual_agenda(agenda);
            if (codigo_resp == RESP_ACEPTADA && hora_asig == 0) hora_asig = msg.hora_solicitada;
            responder_solicitud(msg.pipe_respuesta, rc, hora_asig, hora_act);
        } else {
            registrar_advertencia_texto("Operacion desconocida");
        }
    }

    // esta parte imprime fin de dia
    registrar_info_texto("Fin del dia. Imprimiendo reporte...");
    imprimir_reporte_agenda(agenda);
    notificar_fin_a_agentes();

    detener_reloj_simulacion(reloj);
    esperar_fin_reloj(reloj);

    cerrar_fd(fd_in);
    eliminar_fifo(pipe_recibe);
    destruir_agenda_reservas(agenda);

    registrar_info_texto("Servidor termina.");
    return 0;
}


/****************************************************************************************************************************************************************
Conclusiones
------------
El servidor cumple su papel como componente central del sistema de reservas y coordina correctamente a todos los agentes
Integra de forma coherente la agenda de reservas el reloj de simulacion y la comunicacion por pipes nominales
El manejo del bucle principal permite recibir registros y solicitudes de reserva de varios agentes de manera ordenada
La logica de decision entre aceptar reprogramar o negar se apoya bien en la agenda lo que simplifica el codigo del servidor
El uso de notificar_fin_a_agentes y el mensaje OP_FIN_DIA permite un cierre limpio de la simulacion para todos los clientes
El servidor genera trazas de log claras tanto para registros solicitudes y fin de dia lo que facilita entender el flujo de ejecucion

Recomendaciones
---------------
Seria util validar y limitar el numero maximo de agentes antes de registrar mas pipes para evitar desbordar el arreglo global
****************************************************************************************************************************************************************/
