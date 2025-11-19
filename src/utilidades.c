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
*-Este archivo contiene funciones de utilidad para leer y validar argumentos del servidor y de los agentes
*-Las funciones permiten obtener valores como hora inicial hora final segundos por hora aforo y rutas de pipes
*-El archivo tambien convierte lineas de un archivo csv en datos separados como nombre de familia hora y cantidad de personas
*-Las funciones revisan que los argumentos sean correctos y que cada valor tenga un formato valido
*-Tambien se revisa que el servidor tenga horas validas aforo mayor a cero y que los agentes tengan nombre ruta csv y pipe del servidor
*-Estas utilidades ayudan a que el servidor y los agentes inicien con parametros correctos y no fallen por datos mal escritos
*
*Objetivo:
*-Proveer funciones simples para leer argumentos desde la linea de comandos
*-Separar texto contenido en archivos csv para extraer familia hora y personas
*-Validar valores numericos como horas aforo y segundos para evitar errores en el sistema
*-Asegurar que el servidor reciba todos los parametros necesarios antes de iniciar
*-Asegurar que los agentes reciban su nombre su archivo csv y el pipe del servidor
*-Ofrecer funciones estandar y faciles de usar por el servidor y los agentes
**********************************************************************************************************************************************************/
#include "utilidades.h"
#include "comunes.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

// esta funcion convierte texto a entero y revisa si es valido
static int leer_entero(const char *s, int *out) {
    char *end = NULL;                     // puntero para detectar errores
    long v = strtol(s, &end, 10);         // convierte a entero
    if (!s || *s == '\0' || (end && *end!='\0')) return -1;  // si hay error retorna -1
    *out = (int)v;                        // guarda el numero convertido
    return 0;
}

// esta funcion lee argumentos del servidor desde la linea de comandos
int leer_argumentos_servidor(int argc, char **argv, int *hora_ini, int *hora_fin,
                             int *seg_hora, int *aforo, const char **pipe_entrada) {
    if (!hora_ini || !hora_fin || !seg_hora || !aforo || !pipe_entrada) return -1; // valida punteros
    *pipe_entrada = NULL;                // inicia pipe en NULL
    for (int i=1; i<argc; ++i) {         // recorre los argumentos
        if (strcmp(argv[i], "-i")==0 && i+1<argc) { if (leer_entero(argv[++i], hora_ini)!=0) return -1; } // lee hora inicial
        else if (strcmp(argv[i], "-f")==0 && i+1<argc) { if (leer_entero(argv[++i], hora_fin)!=0) return -1; } // lee hora final
        else if (strcmp(argv[i], "-s")==0 && i+1<argc) { if (leer_entero(argv[++i], seg_hora)!=0) return -1; } // lee segundos por hora
        else if (strcmp(argv[i], "-t")==0 && i+1<argc) { if (leer_entero(argv[++i], aforo)!=0) return -1; } // lee aforo
        else if (strcmp(argv[i], "-p")==0 && i+1<argc) { *pipe_entrada = argv[++i]; } // ruta del pipe entrada
        else { /* ignorar o fallar */ } // ignora otros argumentos
    }
    if (!*pipe_entrada) return -1;       // verifica que el pipe exista
    return 0;
}

// esta funcion lee argumentos para un agente desde la linea de comandos
int leer_argumentos_agente(int argc, char **argv, char *nombre_agente, int tam_agente,
                           char *ruta_csv, int tam_csv, char *pipe_servidor, int tam_pipe) {
    if (!nombre_agente || !ruta_csv || !pipe_servidor) return -1;  // valida punteros
    nombre_agente[0]=ruta_csv[0]=pipe_servidor[0]='\0';            // inicia strings vacios

    for (int i=1; i<argc; ++i) {                                   // recorre argumentos
        if (strcmp(argv[i], "-s")==0 && i+1<argc) {                // lee nombre del agente
            strncpy(nombre_agente, argv[++i], tam_agente-1);
            nombre_agente[tam_agente-1]='\0';
        }
        else if (strcmp(argv[i], "-a")==0 && i+1<argc) {           // lee ruta del csv
            strncpy(ruta_csv, argv[++i], tam_csv-1);
            ruta_csv[tam_csv-1]='\0';
        }
        else if (strcmp(argv[i], "-p")==0 && i+1<argc) {           // lee pipe del servidor
            strncpy(pipe_servidor, argv[++i], tam_pipe-1);
            pipe_servidor[tam_pipe-1]='\0';
        }
    }

    if (nombre_agente[0]=='\0' || ruta_csv[0]=='\0' || pipe_servidor[0]=='\0') return -1; // valida parametros
    return 0;
}

// esta funcion divide una linea csv en familia hora y personas
int dividir_linea_csv(const char *linea, char *familia_out, int *hora_out, int *personas_out) {
    if (!linea || !familia_out || !hora_out || !personas_out) return -1;  // valida punteros
    char buf[256];                                                        // buffer local
    strncpy(buf, linea, sizeof buf - 1);                                  // copia linea
    buf[sizeof buf - 1] = '\0';                                           // asegura fin de string

    // extrae campos separados por coma
    char *p1 = strtok(buf, ",");          // familia
    char *p2 = strtok(NULL, ",");         // hora
    char *p3 = strtok(NULL, ",");         // personas
    if (!p1 || !p2 || !p3) return -1;     // si falta algun campo retorna error

    // quita espacios al inicio
    while (isspace((unsigned char)*p1)) ++p1;
    while (isspace((unsigned char)*p2)) ++p2;
    while (isspace((unsigned char)*p3)) ++p3;

    strncpy(familia_out, p1, FAMILIA_MAX-1);  // copia familia
    familia_out[FAMILIA_MAX-1]='\0';

    if (leer_entero(p2, hora_out)!=0) return -1;         // convierte hora
    if (leer_entero(p3, personas_out)!=0) return -1;     // convierte personas
    return 0;
}

// esta funcion valida parametros del servidor
int validar_parametros_servidor(int hora_ini, int hora_fin, int seg_hora, int aforo) {
    if (hora_ini < HORA_MIN || hora_ini > HORA_MAX) return -1;  // valida hora inicial
    if (hora_fin  < HORA_MIN || hora_fin  > HORA_MAX) return -1; // valida hora final
    if (hora_ini >= hora_fin) return -1;                        // hora ini debe ser menor
    if (seg_hora <= 0) return -1;                               // segundos por hora debe ser mayor a cero
    if (aforo    <= 0) return -1;                               // aforo debe ser mayor a cero
    return 0;
}


/**********************************************************************************************************************************************************
Conclusiones
------------
Las funciones de utilidades cumplen su papel al preparar correctamente los parametros del servidor y de los agentes
El modulo evita que el sistema arranque con datos incompletos o en formatos invalidos gracias a las validaciones basicas
La conversion de lineas csv en familia hora y personas es sencilla y clara y se integra bien con la logica del agente
La separacion de responsabilidades hace que el servidor y los agentes puedan concentrarse en la logica principal y no en parsear argumentos

Recomendaciones
---------------
Seria util reportar mensajes de error mas detallados cuando falle la lectura de argumentos para indicar que parametro exacto es invalido
**********************************************************************************************************************************************************/
