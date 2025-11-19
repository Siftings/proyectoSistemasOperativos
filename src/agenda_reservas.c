/***********************************************************************************************************
*            Pontificia Universidad Javeriana
*-Autores: Juliana Aguirre Ballesteros
*Juan Carlos Santamaria Orjuela
*Juan David Daza
*-Materia: Sistemas Operativos
*-Proyecto de Sistemas Operativos 2025-30
*-Docente: J.Corredor.
*
*Descripcion:
*-Este archivo implementa la agenda de reservas usada por el servidor
*-La agenda guarda cuantas personas hay en cada hora del dia y controla la ocupacion
*-Tambien registra las familias que entran y las que salen del parque en cada hora
*-La agenda permite aceptar negar o reprogramar las reservas segun el aforo disponible
*-La agenda lleva metricas como reservas aceptadas reprogramadas y negadas
*-Tambien calcula las horas mas llenas y mas vacias para el reporte final
*-Todas las operaciones estan protegidas con mutex para evitar problemas entre hilos
*
*Objetivo:
*-Ofrecer una estructura clara que represente toda la ocupacion del parque por horas
*-Verificar si hay cupo para aceptar una reserva en dos horas seguidas
*-Procesar reservas extemporaneas y buscar una nueva hora disponible
*-Actualizar la agenda cuando el reloj avanza y mostrar que familias entran y salen
*-Llevar metricas del dia y generar un reporte final para el servidor
************************************************************************************************************/

#include "agenda_reservas.h"
#include "registro_log.h"
#include "comunes.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>

#define MAX_EVENTOS 1024        // cantidad maxima de eventos por hora

// esta estructura guarda informacion de una familia al entrar o salir
struct Evento {
    char familia[FAMILIA_MAX];  // nombre de familia
    int  personas;              // cantidad de personas
};

// esta estructura es la agenda completa
struct AgendaReservas {
    int hora_ini;               // hora inicial del parque
    int hora_fin;               // hora final del parque
    int hora_actual;            // hora actual segun el reloj
    int aforo;                  // aforo maximo permitido
    int ocupacion[25];          // cuantas personas hay por hora

    int personas_por_hora[25];  // estadisticas de asistencia por hora

    // familias que entran por hora
    struct Evento entradas[25][MAX_EVENTOS];
    int cnt_entradas[25];

    // familias que salen por hora
    struct Evento salidas[25][MAX_EVENTOS];
    int cnt_salidas[25];

    // metricas del dia
    int m_aceptadas_en_hora;
    int m_reprogramadas;
    int m_negadas_ext;
    int m_negadas_sincupo;

    pthread_mutex_t mtx;        // mutex para proteger la agenda
};

// esta funcion revisa si una hora esta en el rango permitido
static int en_rango_sim(int h, int hmin, int hmax) {
    return h >= hmin && h <= hmax;
}

// esta funcion crea una nueva agenda
struct AgendaReservas* crear_agenda_reservas(int hora_ini, int hora_fin, int aforo_maximo) {
    struct AgendaReservas *a = (struct AgendaReservas*)calloc(1, sizeof *a); // reserva memoria
    if (!a) return NULL;
    a->hora_ini = hora_ini;
    a->hora_fin = hora_fin;
    a->hora_actual = hora_ini;
    a->aforo = aforo_maximo;
    pthread_mutex_init(&a->mtx, NULL); // inicia mutex
    return a;
}

// esta funcion destruye la agenda
void destruir_agenda_reservas(struct AgendaReservas *a) {
    if (!a) return;
    pthread_mutex_destroy(&a->mtx);
    free(a);
}

// esta funcion retorna la hora actual
int obtener_hora_actual_agenda(const struct AgendaReservas *a) {
    return a ? a->hora_actual : -1;
}

// esta funcion revisa si hay cupo en dos horas seguidas
static int hay_cupo_bloque(struct AgendaReservas *a, int h, int personas) {
    if (!en_rango_sim(h, a->hora_ini, a->hora_fin) || !en_rango_sim(h+1, a->hora_ini, a->hora_fin)) return 0;
    if (a->ocupacion[h] + personas > a->aforo) return 0;
    if (a->ocupacion[h+1] + personas > a->aforo) return 0;
    return 1;
}

// esta funcion intenta reservar dos horas para una familia
int intentar_reservar_dos_horas(struct AgendaReservas *a,
                                const char *nombre_familia,
                                int hora_solicitada,
                                int cantidad_personas,
                                int *hora_reprogramada_out) {
    if (!a || !nombre_familia || cantidad_personas <= 0) return -2;
    pthread_mutex_lock(&a->mtx);

    // si la hora es menor a la actual se intenta reprogramar
    if (hora_solicitada < a->hora_actual) {

        int h;
        for (h = a->hora_actual; h <= a->hora_fin - 1; ++h) {
            if (hay_cupo_bloque(a, h, cantidad_personas)) {

                // registra ocupacion
                a->ocupacion[h]   += cantidad_personas;
                a->ocupacion[h+1] += cantidad_personas;

                a->personas_por_hora[h]   += cantidad_personas;
                a->personas_por_hora[h+1] += cantidad_personas;

                // registra entrada
                int ce = a->cnt_entradas[h]++;
                if (ce < MAX_EVENTOS) {
                    strncpy(a->entradas[h][ce].familia, nombre_familia, FAMILIA_MAX-1);
                    a->entradas[h][ce].familia[FAMILIA_MAX-1] = '\0';
                    a->entradas[h][ce].personas = cantidad_personas;
                }

                // registra salida
                int cs = a->cnt_salidas[h+2]++;
                if (cs < MAX_EVENTOS) {
                    strncpy(a->salidas[h+2][cs].familia, nombre_familia, FAMILIA_MAX-1);
                    a->salidas[h+2][cs].familia[FAMILIA_MAX-1] = '\0';
                    a->salidas[h+2][cs].personas = cantidad_personas;
                }

                if (hora_reprogramada_out) *hora_reprogramada_out = h;
                pthread_mutex_unlock(&a->mtx);
                return 1; // reprogramada
            }
        }

        pthread_mutex_unlock(&a->mtx);
        return -1; // negada extemporanea
    }

    // si esta fuera de rango o excede aforo
    if (!en_rango_sim(hora_solicitada, a->hora_ini, a->hora_fin) ||
        !en_rango_sim(hora_solicitada+1, a->hora_ini, a->hora_fin) ||
        cantidad_personas > a->aforo) {
        pthread_mutex_unlock(&a->mtx);
        return -2;
    }

    // si hay cupo se acepta en hora exacta
    if (hay_cupo_bloque(a, hora_solicitada, cantidad_personas)) {
        a->ocupacion[hora_solicitada]   += cantidad_personas;
        a->ocupacion[hora_solicitada+1] += cantidad_personas;

        a->personas_por_hora[hora_solicitada]   += cantidad_personas;
        a->personas_por_hora[hora_solicitada+1] += cantidad_personas;

        int ce = a->cnt_entradas[hora_solicitada]++;
        if (ce < MAX_EVENTOS) {
            strncpy(a->entradas[hora_solicitada][ce].familia, nombre_familia, FAMILIA_MAX-1);
            a->entradas[hora_solicitada][ce].familia[FAMILIA_MAX-1] = '\0';
            a->entradas[hora_solicitada][ce].personas = cantidad_personas;
        }

        int cs = a->cnt_salidas[hora_solicitada+2]++;
        if (cs < MAX_EVENTOS) {
            strncpy(a->salidas[hora_solicitada+2][cs].familia, nombre_familia, FAMILIA_MAX-1);
            a->salidas[hora_solicitada+2][cs].familia[FAMILIA_MAX-1] = '\0';
            a->salidas[hora_solicitada+2][cs].personas = cantidad_personas;
        }

        pthread_mutex_unlock(&a->mtx);
        return 0; // aceptada
    }

    // reprogramacion normal
    int h_ini = (hora_solicitada > a->hora_actual) ? hora_solicitada : a->hora_actual;
    for (int h = h_ini; h <= a->hora_fin - 1; ++h) {
        if (hay_cupo_bloque(a, h, cantidad_personas)) {

            a->ocupacion[h]   += cantidad_personas;
            a->ocupacion[h+1] += cantidad_personas;

            a->personas_por_hora[h]   += cantidad_personas;
            a->personas_por_hora[h+1] += cantidad_personas;

            int ce = a->cnt_entradas[h]++;
            if (ce < MAX_EVENTOS) {
                strncpy(a->entradas[h][ce].familia, nombre_familia, FAMILIA_MAX-1);
                a->entradas[h][ce].familia[FAMILIA_MAX-1] = '\0';
                a->entradas[h][ce].personas = cantidad_personas;
            }

            int cs = a->cnt_salidas[h+2]++;
            if (cs < MAX_EVENTOS) {
                strncpy(a->salidas[h+2][cs].familia, nombre_familia, FAMILIA_MAX-1);
                a->salidas[h+2][cs].familia[FAMILIA_MAX-1] = '\0';
                a->salidas[h+2][cs].personas = cantidad_personas;
            }

            if (hora_reprogramada_out) *hora_reprogramada_out = h;
            pthread_mutex_unlock(&a->mtx);
            return 1;
        }
    }

    pthread_mutex_unlock(&a->mtx);
    return -2; // negada sin cupo
}

// esta funcion aplica cambios cuando avanza la hora
void aplicar_avance_de_hora(struct AgendaReservas *a, int nueva_hora) {
    if (!a) return;
    pthread_mutex_lock(&a->mtx);
    a->hora_actual = nueva_hora;

    char linea[512];
    int h = nueva_hora;
    int printed = 0;
    int idx;

    // procesar salidas
    if (a->cnt_salidas[h] > 0) {
        int total = 0;
        snprintf(linea, sizeof linea, "Hora %d: salen:", h);
        registrar_info_texto(linea);

        for (idx = 0; idx < a->cnt_salidas[h]; ++idx) {
            snprintf(linea, sizeof linea, "  - %s (%d)", a->salidas[h][idx].familia, a->salidas[h][idx].personas);
            registrar_info_texto(linea);
            total += a->salidas[h][idx].personas;
        }

        a->ocupacion[h-2] -= total;
        if (a->ocupacion[h-1] >= total) a->ocupacion[h-1] -= total;
        printed = 1;
    }

    // procesar entradas
    if (a->cnt_entradas[h] > 0) {
        snprintf(linea, sizeof linea, "Hora %d: entran:", h);
        registrar_info_texto(linea);

        for (idx = 0; idx < a->cnt_entradas[h]; ++idx) {
            snprintf(linea, sizeof linea, "  - %s (%d)", a->entradas[h][idx].familia, a->entradas[h][idx].personas);
            registrar_info_texto(linea);
        }

        printed = 1;
    }

    // si no hubo cambios
    if (!printed) {
        snprintf(linea, sizeof linea, "Hora %d: sin cambios de entrada salida", h);
        registrar_info_texto(linea);
    }

    pthread_mutex_unlock(&a->mtx);
}

// esta funcion cuenta metricas segun el tipo de respuesta
void contar_respuesta_metricas(struct AgendaReservas *a, int codigo_respuesta) {
    if (!a) return;
    pthread_mutex_lock(&a->mtx);

    switch (codigo_respuesta) {
        case RESP_ACEPTADA:            a->m_aceptadas_en_hora++; break;
        case RESP_REPROGRAMADA:        a->m_reprogramadas++;     break;
        case RESP_NEGADA_EXTEMPORANEA: a->m_negadas_ext++;       break;
        case RESP_NEGADA_SIN_CUPO:     a->m_negadas_sincupo++;   break;
        default: break;
    }

    pthread_mutex_unlock(&a->mtx);
}

// esta funcion imprime el reporte final del dia
void imprimir_reporte_agenda(struct AgendaReservas *a) {
    if (!a) return;
    pthread_mutex_lock(&a->mtx);

    char linea[256];

    int total_negadas = a->m_negadas_ext + a->m_negadas_sincupo;

    int max_p = -1, min_p = -1;
    int hora_max = -1, hora_min = -1;

    // buscar horas mas llenas y mas vacias
    for (int h = a->hora_ini; h <= a->hora_fin; ++h) {
        int p = a->personas_por_hora[h];
        if (max_p < 0 || p > max_p) {
            max_p = p;
            hora_max = h;
        }
        if (min_p < 0 || p < min_p) {
            min_p = p;
            hora_min = h;
        }
    }

    registrar_info_texto("===== REPORTE DEL DIA =====");
    snprintf(linea, sizeof linea, "Aceptadas en hora: %d", a->m_aceptadas_en_hora);
    registrar_info_texto(linea);
    snprintf(linea, sizeof linea, "Reprogramadas   : %d", a->m_reprogramadas);
    registrar_info_texto(linea);
    snprintf(linea, sizeof linea, "Solicitudes negadas (total): %d", total_negadas);
    registrar_info_texto(linea);
    snprintf(linea, sizeof linea, "Negadas extemp. : %d", a->m_negadas_ext);
    registrar_info_texto(linea);
    snprintf(linea, sizeof linea, "Negadas sin cupo: %d", a->m_negadas_sincupo);
    registrar_info_texto(linea);

    if (hora_max >= 0) {
        snprintf(linea, sizeof linea, "Hora con MAYOR numero de personas: %d (personas=%d)", hora_max, max_p);
        registrar_info_texto(linea);
    }

    if (hora_min >= 0) {
        snprintf(linea, sizeof linea, "Hora con MENOR numero de personas: %d (personas=%d)", hora_min, min_p);
        registrar_info_texto(linea);
    }

    registrar_info_texto("===========================");
    pthread_mutex_unlock(&a->mtx);
}


/********************************************************************************************************************************
Conclusiones 
------------
La agenda funciona bien como memoria del parque.
Guarda cuantas personas hay por hora y que familias entran y salen.
Esto permite al servidor tomar decisiones correctas.

La logica de reservas esta completa.
El código puede aceptar, reprogramar o negar una reserva según el aforo y la hora.
También distingue si una reserva es extemporánea.

No hay problemas con hilos.
El mutex protege todos los datos, así que el servidor y el reloj pueden trabajar al mismo tiempo sin dañar la información.

El módulo deja todo listo para el reporte final.
Cuenta cuántas reservas fueron aceptadas, reprogramadas o negadas, y calcula las horas mas llenas y mas vacias.

El diseño es claro y fácil de usar desde el servidor.
Expone funciones simples: reservar, avanzar hora, contar metricas y generar el reporte.

Recomendaciones 
----------------
Proteger mejor las salidas.
Cuando se restan personas en h-2 y h-1, sería bueno revisar que esos índices no sean negativos.

***************************************************************************************************************************/