import random

N = 10000  # cantidad de actividades
MAX_DEPS = 3  # maximo de dependencias por actividad

with open("plan_grande.txt", "w") as f:
    for i in range(1, N + 1):
        nombre = f"actividad_{i}"
        tiempo = random.randint(100, 500)  # ms, bajo para que la prueba no demore eternamente

        # Solo puede depender de actividades con ID menor (garantiza que no haya ciclos)
        posibles_deps = list(range(1, i))
        if posibles_deps:
            cantidad = random.randint(0, min(MAX_DEPS, len(posibles_deps)))
            deps = random.sample(posibles_deps, cantidad)
            deps_str = ", ".join(str(d) for d in sorted(deps))
        else:
            deps_str = ""

        f.write(f"{i} : {nombre} : {tiempo} : {deps_str}\n")

print(f"Generado plan_grande.txt con {N} actividades")
