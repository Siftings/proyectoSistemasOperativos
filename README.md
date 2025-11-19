# Proyecto Sistemas Operativos

Pontificia Universidad Javeriana
Departamento de Ingenieria de Sistemas
Materia: Sistemas Operativos
Docente: John Corredor, PhD.
Autores:
- Juliana Aguirre Ballesteros
- Juan Carlos Santamaria Orjuela
- Juan David Daza Caro

## Descripcion
---
Este repositorio contiene la implementación del proyecto del curso Sistemas Operativos. La descripcion detallada del proyecto (objetivos, diseño, pruebas y conclusiones) se encuentra en el [informe](./InformeProyectoSistemasOperativos.pdf).

## Compilacion
---
Requisitos: Ejecutar este proyecto en un sistema operativo UNIX, tener instalado `make` y `GCC` (GNU Compiler Collection).

1. Compilar todo (desde la raiz del proyecto):

```bash
make
```

2. Limpiar los archivos generados:

```bash
make clean
```

Los ejecutables resultantes se ubicaran en la carpeta `bin`, se pueden ejecutar con:
```bash
bin/servidor (argumentos)
bin/agente (argumentos)
```

## Ejemplo de ejecucion
---
Abrir dos terminales: en uno ejecutar el servidor y en otro el agente:

```zsh
# Terminal 1: iniciar servidor
# Arguentos de prueba
bin/servidor -i 7 -f 10 -s 2 -t 20 -p /tmp/pipeReservas

# Terminal 2: iniciar agente (Se pueden iniciar multiples agentes en simultaneo)
# Argumentos de prueba 
bin/agente -s AgenteA -a data/agenteA.csv -p /tmp/pipeReservas
```

## Datos
---
Los archivos de datos se encuentran en la carpeta `data/` (por ejemplo `agenteA.csv`, `agenteB.csv`).
