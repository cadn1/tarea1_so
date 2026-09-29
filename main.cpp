#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include "lector.hpp" 
#include "motor.hpp"  

int main(int argc, char* argv[]) { // validar
    if (argc != 3) {
        std::cerr << "Uso: " << argv[0] << " <archivo.txt> <K_concurrencia>" << std::endl;
        return 1;
    }

    std::string nombre_archivo = argv[1];
    int limite_k = std::stoi(argv[2]);

    std::ifstream archivo(nombre_archivo);
    if (!archivo.is_open()) {
        std::cerr << "Error: no se pudo abrir el archivo " << nombre_archivo << std::endl;
        return 1;
    }

    std::vector<Actividad> actividades;
    std::string linea;

    while (std::getline(archivo, linea)) {
        if (linea.empty()) continue;
        Actividad a = parsearLinea(linea);
        actividades.push_back(a);
    }

    std::cout << "--- Planificador Dieciochero ---" << std::endl;
    std::cout << "Archivo cargado: " << nombre_archivo << std::endl;
    std::cout << "Total de actividades a procesar: " << actividades.size() << std::endl;
    std::cout << "Limite de trabajadores en simultaneo (K): " << limite_k << std::endl;

    ejecutarPlan(actividades, limite_k);

    std::cout << "FIN" << std::endl;
    return 0;
}