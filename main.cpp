#include <iostream>
#include <vector>
#include <string>

struct Actividad {
    std::string id;
    std::string nombre;
    int tiempo_ms;
    std::vector<std::string> dependencias;
};

int main() {
    Actividad prueba;
    prueba.id = "1";
    prueba.nombre = "prender_carbon";
    prueba.tiempo_ms = 500;

    std::cout << "ID: " << prueba.id << std::endl;
    std::cout << "Nombre: " << prueba.nombre << std::endl;
    std::cout << "Tiempo: " << prueba.tiempo_ms << " ms" << std::endl;

    // prender_carbon no depende de nada, dejare una prueba con dependencias para ver que pasa
    Actividad prueba2;
    prueba2.id = "4";
    prueba2.nombre = "asar_longaniza";
    prueba2.tiempo_ms = 800;
    prueba2.dependencias.push_back("1");
    prueba2.dependencias.push_back("2");

    std::cout << "Actividad 2 - ID: " << prueba2.id << ", depende de: ";
    for (const std::string& dep : prueba2.dependencias) {
        std::cout << dep << " ";
    }
    std::cout << std::endl;

    return 0;
}
