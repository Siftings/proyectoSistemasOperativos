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
*-Este archivo define funciones simples para escribir mensajes en el log del sistema
*-Permite registrar mensajes de informacion advertencia y error
*-Cada mensaje se imprime con un formato basico ya calculado por el modulo de log
*-Estas funciones ayudan a que el servidor y los agentes reporten lo que sucede
*
*Objetivo:
*-Proveer funciones claras para que cualquier parte del sistema pueda escribir mensajes en el log
*-Separar los tipos de mensaje segun si son de informacion advertencia o error
*-Facilitar la lectura y seguimiento del funcionamiento del sistema
****************************************************************************************************/

#ifndef REGISTRO_LOG_H
#define REGISTRO_LOG_H

// esta funcion escribe un mensaje de informacion
void registrar_info_texto(const char *texto);

// esta funcion escribe un mensaje de error
void registrar_error_texto(const char *texto);

// esta funcion escribe un mensaje de advertencia
void registrar_advertencia_texto(const char *texto);

#endif


/****************************************************************************************************
Conclusiones
------------
El archivo entrega funciones simples para escribir mensajes en el log del sistema
Permite separar mensajes de info advertencia y error de forma clara
Facilita que todos los modulos reporten lo que pasa sin repetir codigo
Ayuda a mantener un registro ordenado del funcionamiento del sistema
****************************************************************************************************/
