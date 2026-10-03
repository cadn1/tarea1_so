# Planificador Dieciochero

Simulador de planificación de actividades para las Fiestas Patrias, modeladas como un Grafo Acíclico Dirigido (DAG), usando procesos, pipes y señales.

## Requisitos

- Sistema operativo tipo Unix (Linux, probado en Ubuntu 24.04)
- Compilador `g++` con soporte para C++17
- No requiere librerías externas adicionales

## Compilación

```bash
g++ -Wall -Wextra -std=c++17 -lpthread main.cpp -o planificador
```

## Uso

```bash
./planificador <archivo_plan.txt> <K>
```

| Argumento | Descripción |
|---|---|
| `archivo_plan.txt` | Archivo con formato `ID : Nombre : tiempo_ms : [dependencias]` |
| `K` | Límite de procesos simultáneos (concurrencia) |

Ejemplo:
```bash
./planificador plan.txt 2
```

## Estado del proyecto

- [x] Parseo de plan.txt
- [x] Modelado del DAG
- [x] Creación de procesos
- [x] Control de concurrencia (K), sin busy waiting, sin race conditions
- [x] Paso de mensajes (pipes)
- [x] Aislamiento de errores
- [x] Carga de estrés (10.000 actividades)
- [ ] Manejo de SIGINT (Ctrl+C) — en curso

## Estructura del código

| Archivo | Contenido |
|---|---|
| `lector.hpp` | `Actividad` (struct), `quitarEspacios`, `parsearLinea`, `construirDependientes` |
| `motor.hpp` | `ejecutarPlan`: fork, control de K, pipes, aislamiento de errores |
| `main.cpp` | Punto de entrada: valida argumentos, lee archivo, llama a `ejecutarPlan` |

## Decisiones de diseño

- **Concurrencia sin hilos**: el límite `K` lo controla únicamente el proceso padre con sus propias variables. Como los hijos no comparten memoria, no hay condiciones de carrera sin necesitar semáforos.
- **Sin busy waiting**: se usa `wait()`, que bloquea al padre hasta que un hijo real termina, en vez de consultar en un loop.
- **Paso de mensajes**: cada actividad abre un pipe antes del `fork()`. El hijo escribe un mensaje al terminar; el padre lo lee cuando `wait()` detecta su finalización.
- **Aislamiento de errores**: se detecta con `WIFEXITED`/`WEXITSTATUS` si un hijo falló. Las actividades que dependen (directa o indirectamente) de una fallida se cancelan sin ejecutarse.
- **IDs como string**: el enunciado describe los IDs como alfanuméricos, por lo que se usó `std::string` en vez de `int`.

## Pruebas realizadas

| Plan | K | Resultado |
|---|---|---|
| `plan.txt` (6 actividades, ejemplo del enunciado) | 2 | Orden de ejecución respeta dependencias |
| `plan_con_error.txt` (1 actividad falla a propósito) | 2 | La rama dependiente se cancela, el resto completa normal |
| `plan_grande.txt` (10.000 actividades generadas) | 50 | 10.000 completadas, código de salida 0, ~1m2s |

El plan de 10.000 actividades se genera con:
```bash
python3 generador.py
```

## Autores

- Juan Pablo Manriquez
- Cristobal Deck
