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
*-Este archivo define la interfaz para manejar la agenda de reservas del sistema
*-La agenda controla las reservas por horas y el aforo disponible
*-Permite aceptar reprogamar o negar solicitudes segun cupo y reglas del sistema
*-Tambien guarda metricas y registra los cambios por hora
*-Incluye funciones para crear usar y destruir la agenda de forma sencilla
*
*Objetivo:
*-Proveer funciones que permitan administrar todas las reservas del dia
*-Ofrecer un mecanismo seguro con mutex para manejar concurrencia
*-Permitir que el servidor consulte la hora actual y avance el reloj
*-Registrar metricas y generar un reporte final del dia
*-Centralizar toda la logica de reservas en un solo modulo
****************************************************************************************************/

#ifndef AGENDA_RESERVAS_H
#define AGENDA_RESERVAS_H

#include "comunes.h"

/* Tipo opaco: encapsula la informacion interna de la agenda */
// esta estructura guarda la informacion interna de la agenda
struct AgendaReservas;

/* Ciclo de vida */
// esta funcion crea la agenda con horas y aforo maximo
struct AgendaReservas* crear_agenda_reservas(int hora_ini, int hora_fin, int aforo_maximo);
// esta funcion libera la agenda
void destruir_agenda_reservas(struct AgendaReservas *a);

/* Consulta */
// esta funcion devuelve la hora actual registrada en la agenda
int obtener_hora_actual_agenda(const struct AgendaReservas *a);

/* Operaciones de reserva
   0  = reserva aceptada
   1  = reserva reprogramada
  -1  = negada por estar fuera de tiempo
  -2  = negada por no haber cupo
*/
// esta funcion intenta reservar dos horas consecutivas
int intentar_reservar_dos_horas(struct AgendaReservas *a, const char *nombre_familia,
                                int hora_solicitada, int cantidad_personas,
                                int *hora_reprogramada_out);

/* Avance de hora */
// esta funcion aplica el cambio de hora y actualiza entradas y salidas
void aplicar_avance_de_hora(struct AgendaReservas *a, int nueva_hora);

/* Metricas */
// esta funcion suma estadisticas segun la respuesta enviada
void contar_respuesta_metricas(struct AgendaReservas *a, int codigo_respuesta);
// esta funcion imprime el reporte del dia
void imprimir_reporte_agenda(struct AgendaReservas *a);

#endif


/******************************************************************************************************************************************************
Conclusiones
------------
La interfaz de agenda encapsula correctamente toda la logica de reservas en un solo modulo
Ofrece un tipo opaco que oculta los detalles internos y simplifica el uso desde el servidor
Las funciones de ciclo de vida consulta reservas avance de hora y metricas cubren todas las necesidades del sistema
Los codigos de retorno de intentar_reservar_dos_horas permiten distinguir claramente entre reservas aceptadas reprogramadas y negadas
El modulo sirve como punto unico para generar el reporte final del dia a partir de la informacion acumulada
*********************************************************************************************************************************************************/
