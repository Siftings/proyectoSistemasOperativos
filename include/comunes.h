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
*-Este archivo contiene valores constantes y estructuras usadas por todo el proyecto
*-Define limites como horas validas tamanos maximos de nombres y rutas
*-Define codigos de operacion para que el agente y el servidor entiendan los mensajes
*-Define codigos de respuesta que el servidor envia segun acepte niegue o reprograme
*-Incluye estructuras para mensajes que van del agente al servidor y del servidor al agente
*
*Objetivo:
*-Proveer un archivo comun para compartir configuraciones y formatos de mensajes
*-Asegurar que todos los modulos usen los mismos valores y codigos
*-Permitir que el servidor y los agentes se comuniquen con estructuras estandar
*-Evitar errores al manejar nombres familias horas o rutas
****************************************************************************************************/

#ifndef COMUNES_H
#define COMUNES_H

#define HORA_MIN        7
#define HORA_MAX        19
#define NOMBRE_MAX      32
#define FAMILIA_MAX     32
#define RUTA_MAX        128

/* Operaciones de mensaje */
// esta constante indica registro del agente
#define OP_REGISTRO     1
// esta constante indica una solicitud de reserva
#define OP_SOLICITUD    2
// esta constante indica una respuesta del servidor
#define OP_RESPUESTA    3
// esta constante indica que termino el dia
#define OP_FIN_DIA      4

/* Codigos de respuesta del servidor */
// esta respuesta indica que la reserva fue aceptada
#define RESP_ACEPTADA             0
// esta respuesta indica que la reserva fue reprogramada
#define RESP_REPROGRAMADA         1
// esta respuesta indica que se nego por ser muy tarde
#define RESP_NEGADA_EXTEMPORANEA  2
// esta respuesta indica que no hay cupo
#define RESP_NEGADA_SIN_CUPO      3

/* Mensaje Agente -> Servidor */
struct MensajeSolicitud {
    int  operacion;                          // tipo de operacion enviada
    char nombre_agente[NOMBRE_MAX];          // nombre del agente
    char pipe_respuesta[RUTA_MAX];           // ruta del pipe donde recibe respuesta
    char nombre_familia[FAMILIA_MAX];        // familia que pide reserva
    int  hora_solicitada;                    // hora pedida
    int  cantidad_personas;                  // numero de personas de la reserva
};

/* Mensaje Servidor -> Agente */
struct MensajeRespuesta {
    int  operacion;           // tipo de mensaje enviado por el servidor
    int  codigo_respuesta;    // codigo de respuesta RESP_*
    int  hora_asignada;       // si la reserva fue reprogramada
    int  hora_actual;         // hora en la simulacion
    char detalle[128];        // texto descriptivo
};

#endif


/*****************************************************************************************************************************
Conclusiones
------------
El archivo comunes centraliza las constantes y estructuras basicas usadas por todo el proyecto
Define rangos de horas y tamanos maximos que ayudan a validar datos en varios modulos
Los codigos de operacion y respuesta permiten que servidor y agentes entiendan los mensajes sin ambiguedad
Las estructuras MensajeSolicitud y MensajeRespuesta unifican el formato de comunicacion entre procesos
Este encabezado reduce duplicacion de definiciones y evita inconsistencias entre diferentes archivos

********************************************************************************************************************************/
