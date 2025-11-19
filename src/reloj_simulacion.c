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
*-Este archivo implementa el reloj de simulacion usado por el servidor
*-El reloj avanza las horas del sistema de forma automatica
*-El reloj avisa al servidor cada vez que cambia la hora para que actualice la agenda
*-El reloj usa un hilo separado que se ejecuta mientras avanza el dia
*-El reloj puede detenerse cuando el servidor termina su trabajo
*
*Objetivo:
*-Simular el paso del tiempo dentro del sistema de reservas
*-Notificar al servidor cada vez que el reloj avanza una hora
*-Ejecutar el reloj en un hilo independiente para no bloquear el programa
*-Permitir que el servidor pueda detener el reloj y esperar a que el hilo termine
**********************************************************************************************************************************************************/

#include "reloj_simulacion.h"
#include "registro_log.h"
#include "comunes.h"
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

// esta estructura guarda los datos del reloj
struct RelojSimulacion {
    int hora_ini;             // esta variable guarda la hora inicial
    int hora_fin;             // esta variable guarda la hora final
    int seg_por_hora;         // esta variable guarda segundos que dura una hora
    al_cambiar_hora_cb cb;    // esta variable guarda la funcion de aviso
    void *usuario;            // esta variable guarda datos del servidor
    pthread_t hilo;           // esta variable guarda el hilo del reloj
    atomic_int detener;       // esta variable indica si se debe parar el reloj
};

// esta funcion es el hilo que avanza el reloj
static void* hilo_reloj(void *arg) {
    struct RelojSimulacion *r = (struct RelojSimulacion*)arg;
    int h = r->hora_ini;             // inicia en la hora inicial

    if (r->cb) r->cb(h, r->usuario); // avisa la primera hora

    while (!atomic_load(&r->detener) && h < r->hora_fin) { // bucle del reloj
        sleep(r->seg_por_hora);       // espera el tiempo configurado
        if (atomic_load(&r->detener)) break;  // si debe detenerse sale
        h++;                           // avanza una hora
        if (h > r->hora_fin) break;    // si pasa la hora final sale
        if (r->cb) r->cb(h, r->usuario); // avisa el cambio de hora
    }
    return NULL;                       // termina el hilo
}

// esta funcion crea e inicia el reloj de simulacion
struct RelojSimulacion* iniciar_reloj_simulacion(
        int hora_ini, int hora_fin, int segundos_por_hora,
        al_cambiar_hora_cb on_tick, void *usuario) {

    struct RelojSimulacion *r = (struct RelojSimulacion*)calloc(1, sizeof *r); // reserva memoria
    if (!r) return NULL;                // si falla retorna null

    r->hora_ini = hora_ini;             // guarda hora inicial
    r->hora_fin = hora_fin;             // guarda hora final
    r->seg_por_hora = segundos_por_hora; // guarda segundos por hora
    r->cb = on_tick;                    // guarda funcion callback
    r->usuario = usuario;               // guarda datos del usuario
    atomic_store(&r->detener, 0);       // marca que no debe detenerse

    // crea el hilo del reloj
    if (pthread_create(&r->hilo, NULL, hilo_reloj, r) != 0) {
        free(r); return NULL;           // si no se puede crear libera memoria
    }

    return r;                            // retorna el reloj
}

// esta funcion indica que el reloj debe detenerse
void detener_reloj_simulacion(struct RelojSimulacion *r) {
    if (!r) return;                     // si es null no hace nada
    atomic_store(&r->detener, 1);       // marca detener en 1
}

// esta funcion espera a que el hilo del reloj termine
void esperar_fin_reloj(struct RelojSimulacion *r) {
    if (!r) return;                     // si es null no hace nada
    pthread_join(r->hilo, NULL);        // espera que el hilo termine
}


/**********************************************************************************************************************************************************
Conclusiones
------------
El reloj de simulacion cumple su funcion de avanzar las horas del sistema de forma automatica
Ejecuta la logica de tiempo en un hilo separado sin bloquear el hilo principal del servidor
Notifica al servidor cada vez que cambia la hora permitiendo actualizar la agenda en el momento correcto
El uso de una bandera atomica para detener el reloj hace que el cierre sea ordenado y seguro
La primera notificacion con la hora inicial asegura que el servidor arranque ya sincronizado con el reloj

Recomendaciones
---------------
Seria util liberar la memoria del reloj en una funcion adicional una vez que el hilo haya terminado
Podria validarse que los parametros hora_ini hora_fin y segundos_por_hora tengan valores coherentes antes de crear el hilo
Se podria agregar un mensaje de log cuando el reloj inicia y cuando termina para facilitar la depuracion
**********************************************************************************************************************************************************/
