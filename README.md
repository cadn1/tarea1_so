# Planificador Dieciochero

Simulador de planificación de actividades para las Fiestas Patrias, modeladas como un Grafo Acíclico Dirigido (DAG), usando procesos, pipes y señales.

## Requisitos

- Sistema operativo tipo Unix (Linux, probado en Ubuntu 24.04)
- Compilador `g++` con soporte para C++17
- No requiere librerías externas adicionales
- Python 3 solo para `generador.py` (herramienta externa de testing, no es parte del programa entregado en C++)

## Compilación

```bash
g++ -Wall -Wextra -std=c++17 -lpthread src/main.cpp -o planificador
```

## Uso

```bash
./planificador <tests/archivo_plan.txt> <K>
```

| Argumento | Descripción |
|---|---|
| `archivo_plan.txt` | Archivo con formato `ID : Nombre : tiempo_ms : [dependencias]` |
| `K` | Límite de procesos simultáneos (concurrencia) |

Ejemplo:
```bash
./planificador tests/plan.txt 2
```

## Estado del proyecto

- [x] Parseo de plan.txt
- [x] Modelado del DAG
- [x] Creación de procesos
- [x] Control de concurrencia (K), sin busy waiting, sin race conditions
- [x] Paso de mensajes (pipes)
- [x] Aislamiento de errores
- [x] Carga de estrés (10.000 actividades)
- [x] Manejo de SIGINT (Ctrl+C, SEREMI)

## Estructura del código

| Archivo | Contenido |
|---|---|
| `lector.hpp` | `Actividad` (struct), `quitarEspacios`, `parsearLinea`, `construirDependientes` |
| `motor.hpp` | `ejecutarPlan`: fork, control de K, pipes, aislamiento de errores, manejo de SIGINT |
| `main.cpp` | Punto de entrada: valida argumentos, lee archivo, llama a `ejecutarPlan` |

## Decisiones de diseño

- **Concurrencia sin hilos**: el límite `K` lo controla únicamente el proceso padre con sus propias variables. Como los hijos no comparten memoria, no hay condiciones de carrera sin necesitar semáforos.
- **Sin busy waiting**: se usa `wait()`, que bloquea al padre hasta que un hijo real termina, en vez de consultar en un loop.
- **Paso de mensajes**: cada actividad abre un pipe antes del `fork()`. El hijo escribe un mensaje al terminar; el padre lo lee cuando `wait()` detecta su finalización.
- **Aislamiento de errores**: se detecta con `WIFEXITED`/`WEXITSTATUS` si un hijo falló. Las actividades que dependen (directa o indirectamente) de una fallida se cancelan sin ejecutarse.
- **Manejo de SIGINT**: un handler marca una bandera (`volatile sig_atomic_t`) al recibir Ctrl+C. El ciclo principal revisa esa bandera y, si está activa, envía `SIGTERM` a todos los procesos hijos que seguían corriendo y espera su finalización con `wait()` antes de salir, evitando procesos zombies.
- **IDs como string**: el enunciado describe los IDs como alfanuméricos, por lo que se usó `std::string` en vez de `int`.

## Por qué el plan.txt de ejemplo es pequeño

El `plan.txt` usado para las pruebas principales corresponde al ejemplo del propio enunciado (la ramada del señor Loyola: prender_carbon, comprar_carne, comprar_pan, asar_longaniza, armar_choripan, servir_mesa). Se eligió a propósito un plan chico para poder verificar a mano, línea por línea, que el parseo y el orden de ejecución respetan las dependencias correctamente — algo imposible de revisar visualmente con un grafo de miles de nodos.

La escalabilidad del planificador no se prueba con este archivo, sino con `plan_grande.txt`, generado con `generador.py` (10.000 actividades). Se separaron ambos propósitos: un DAG chico y legible para validar la lógica, y uno grande para validar que la arquitectura aguanta carga real sin romperse.

## Pruebas realizadas

| Plan | K | Resultado |
|---|---|---|
| `plan.txt` (6 actividades, ejemplo del enunciado) | 2 | Orden de ejecución respeta dependencias |
| `plan_con_error.txt` (1 actividad falla a propósito) | 2 | La rama dependiente se cancela, el resto completa normal |
| `plan_grande.txt` (10.000 actividades generadas) | 50 | 10.000 completadas, código de salida 0, ~1m2s |
| `plan_grande.txt` (10.000 actividades, interrumpido) | 5 | Ctrl+C detecta la señal, mata procesos activos y termina sin zombies |

El plan de 10.000 actividades se genera con:
```bash
python3 tests/generador.py
```

## Autores

- Juan Pablo Manriquez
- Cristobal Deck
