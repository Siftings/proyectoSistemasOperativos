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
*-Este archivo contiene funciones de ayuda usadas por el servidor y los agentes
*-Aqui se leen los argumentos que el usuario pasa por consola al iniciar cada proceso
*-Tambien se validan los parametros para evitar errores en la configuracion
*-Incluye una funcion para separar los datos de una linea CSV con familia hora y personas
*-Estas utilidades permiten que el sistema funcione con entradas correctas y ordenadas
*
*Objetivo:
*-Proveer funciones simples para leer argumentos del servidor y del agente
*-Verificar que los valores ingresados sean validos
*-Procesar lineas CSV y extraer familia hora y personas
*-Evitar errores al iniciar el sistema garantizando que todo este bien configurado
****************************************************************************************************/

#ifndef UTILIDADES_H
#define UTILIDADES_H

// esta funcion lee los argumentos del servidor
int leer_argumentos_servidor(int argc, char **argv, int *hora_ini, int *hora_fin, int *seg_hora, int *aforo, const char **pipe_entrada);

// esta funcion lee los argumentos del agente
int leer_argumentos_agente(int argc, char **argv, char *nombre_agente, int tam_agente, char *ruta_csv, int tam_csv, char *pipe_servidor, int tam_pipe);

// esta funcion separa una linea csv en familia hora y personas
int dividir_linea_csv(const char *linea, char *familia_out, int *hora_out, int *personas_out);

// esta funcion valida los parametros del servidor
int validar_parametros_servidor(int hora_ini, int hora_fin, int seg_hora, int aforo);

#endif


/****************************************************************************************************
Conclusiones
------------
El archivo ofrece funciones simples para leer y validar argumentos del servidor y del agente
Permite procesar lineas CSV y extraer datos de forma clara y directa
Ayuda a evitar errores al inicio del sistema al revisar parametros antes de usarlos
Facilita que los procesos arranquen con valores correctos y compatibles entre si
****************************************************************************************************/
