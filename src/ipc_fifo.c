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
*-Este archivo implementa funciones para trabajar con pipes tipo FIFO
*-Permite crear abrir leer escribir y eliminar pipes usados por el servidor y los agentes
*-Cada funcion maneja la comunicacion entre procesos por medio de lectura y escritura bloqueante
*-Estas funciones se usan para enviar solicitudes y respuestas entre servidor y agentes
*-El archivo ofrece metodos seguros que reintentan en caso de interrupciones y aseguran que se lea o escriba el bloque completo
*
*Objetivo:
*-Proveer funciones simples para manejar pipes FIFO
*-Permitir la creacion y eliminacion de pipes segun los necesite el sistema
*-Llevar a cabo lectura y escritura completa de datos sin cortar la informacion
*-Asegurar que la comunicacion entre procesos funcione correctamente y sin errores
**********************************************************************************************************************************************************/

#include "ipc_fifo.h"
#include <fcntl.h>
#include <errno.h>
#include <string.h>

// esta funcion crea un fifo si no existe
int crear_fifo_si_no_existe(const char *ruta, mode_t modo) {
    if (mkfifo(ruta, modo) == 0) return 0;   // crea fifo
    if (errno == EEXIST) return 0;           // si ya existe esta bien
    return -1;                               // error
}

// esta funcion abre un fifo para lectura bloqueante
int abrir_lectura_bloqueante(const char *ruta) {
    int fd = open(ruta, O_RDONLY);           // abre modo lectura
    return fd;                               // retorna descriptor o -1 si falla
}

// esta funcion abre un fifo para escritura bloqueante
int abrir_escritura_bloqueante(const char *ruta) {
    int fd = open(ruta, O_WRONLY);           // abre modo escritura
    return fd;                               // retorna descriptor o -1 si falla
}

// esta funcion lee un bloque completo de bytes
ssize_t leer_bloque(int fd, void *buf, size_t nbytes) {
    char *p = (char*)buf;                    // puntero al buffer
    size_t tot = 0;                          // bytes leidos
    while (tot < nbytes) {                   // sigue hasta leer todo
        ssize_t r = read(fd, p + tot, nbytes - tot);  // lee datos
        if (r == 0) return 0;                // fin de archivo
        if (r < 0) {
            if (errno == EINTR) continue;    // si hubo interrupcion reintenta
            return -1;                       // error real
        }
        tot += (size_t)r;                    // suma bytes leidos
    }
    return (ssize_t)tot;                     // retorna total leido
}

// esta funcion escribe un bloque completo de bytes
ssize_t escribir_bloque(int fd, const void *buf, size_t nbytes) {
    const char *p = (const char*)buf;        // puntero a datos
    size_t tot = 0;                          // bytes escritos
    while (tot < nbytes) {                   // escribe hasta completar
        ssize_t r = write(fd, p + tot, nbytes - tot); // escribe bytes
        if (r < 0) {
            if (errno == EINTR) continue;    // reintenta si hubo senal
            return -1;                       // error
        }
        tot += (size_t)r;                    // suma bytes escritos
    }
    return (ssize_t)tot;                     // retorna total escrito
}

// esta funcion cierra un descriptor de archivo
int cerrar_fd(int fd) {
    return close(fd);                        // cierra el fd
}

// esta funcion elimina un fifo del sistema
int eliminar_fifo(const char *ruta) {
    return unlink(ruta);                     // borra el archivo fifo
}

/**********************************************************************************************************************************************************
*Conclusiones
*------------
*El modulo de pipes cumple correctamente su funcion dentro del sistema
*Proporciona creacion apertura lectura escritura y eliminacion de FIFOs de forma sencilla
*Las funciones leer_bloque y escribir_bloque garantizan que los datos se transfieran completos sin cortes
*El manejo de interrupciones mediante reintentos hace que la comunicacion sea estable y confiable
*El servidor y los agentes pueden intercambiar mensajes sin perdida de informacion gracias a estas rutinas
*Recomendaciones
*---------------
*Seria util agregar mensajes de error mas descriptivos para facilitar depuracion
*Podria verificarse si open en modo escritura bloqueante queda esperando indefinidamente cuando no hay lectores
*Seria conveniente validar que las rutas recibidas no sean nulas antes de operar sobre ellas
**********************************************************************************************************************************************************/
