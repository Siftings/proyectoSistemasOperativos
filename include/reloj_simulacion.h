/****************************************************************************************************
*            Pontificia Universidad Javeriana
*Autores: Juliana Aguirre Ballesteros
*Juan Carlos Santamaria Orjuela
*Juan David Daza
*Materia: Sistemas Operativos
*Proyecto de Sistemas Operativos 2025-30
*-Docente: J.Corredor.
*
*Descripcion:
*-Este archivo define la estructura del reloj de simulacion usado por el servidor
*-El reloj avanza la hora de forma automatica y llama una funcion cada vez que cambia la hora
*-Permite iniciar detener y esperar a que el reloj termine su trabajo
*-Este reloj controla el flujo del dia en el sistema de reservas
*
*Objetivo:
*-Proveer un reloj simple para simular el paso de las horas en el sistema
*-Permitir que el servidor reciba notificaciones cuando la hora cambia
*-Ofrecer funciones para iniciar detener y finalizar el ciclo del reloj
*-Mantener el control del tiempo de manera ordenada y facil de usar
****************************************************************************************************/

#ifndef RELOJ_SIMULACION_H
#define RELOJ_SIMULACION_H

// esta funcion se llama cada vez que el reloj avanza una hora
typedef void (*al_cambiar_hora_cb)(int hora_actual, void *usuario);

// tipo opaco del reloj no se ve su contenido aqui
struct RelojSimulacion;

// esta funcion inicia el reloj de simulacion
struct RelojSimulacion* iniciar_reloj_simulacion(int hora_ini, int hora_fin, int segundos_por_hora, al_cambiar_hora_cb on_tick, void *usuario);

// esta funcion marca que el reloj debe detenerse
void detener_reloj_simulacion(struct RelojSimulacion *r);

// esta funcion espera a que el hilo del reloj termine
void esperar_fin_reloj(struct RelojSimulacion *r);

#endif


/****************************************************************************************************
Conclusiones
------------
El archivo entrega una interfaz simple para manejar el reloj de simulacion
Permite iniciar detener y esperar el fin del reloj de manera clara
El callback facilita avisar al servidor cuando cambia la hora
El modulo mantiene el control del tiempo organizado y facil de integrar con el resto del sistema
****************************************************************************************************/
