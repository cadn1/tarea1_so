#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <map>

struct Actividad {
    std::string id;
    std::string nombre;
    int tiempo_ms;
    std::vector<std::string> dependencias;
};

// Quita espacios y tabs al inicio y al final de un string.
// Necesario porque plan.txt tiene espacios alrededor de los ":" (ej: "1 : prender_carbon")
// y sin esto, el id quedaria como "1 " en vez de "1".
std::string quitarEspacios(const std::string& s) {
    size_t inicio = s.find_first_not_of(" \t");  // primera posicion que NO es espacio/tab
    size_t fin = s.find_last_not_of(" \t");       // ultima posicion que NO es espacio/tab
    if (inicio == std::string::npos) return "";   // el string era solo espacios (o vacio)
    return s.substr(inicio, fin - inicio + 1);    // devuelve solo la parte "real" del string 
}


// Recibe una linea completa de plan.txt (ej: "4 : asar_longaniza : 800 : 1, 2")
Actividad parsearLinea(const std::string& linea) {
    Actividad a;
    std::stringstream ss(linea);
    std::string campo;
// Para la parte del id
    std::getline(ss, campo, ':');
    a.id = quitarEspacios(campo);
// Para la parte del nombre
    std::getline(ss, campo, ':');
    a.nombre = quitarEspacios(campo);

// Para la parte del tiempo en ms
    std::getline(ss, campo, ':');
    campo = quitarEspacios(campo);
    if (campo.empty()) {
        a.tiempo_ms = 100 + rand() % (5000 - 100 + 1);
    } else {
        a.tiempo_ms = std::stoi(campo);
    }

// Para la parte de las dependencias
    std::getline(ss, campo);
    campo = quitarEspacios(campo);
    std::stringstream depsStream(campo);
    std::string dep;
    while (std::getline(depsStream, dep, ',')) {
        dep = quitarEspacios(dep);
        if (!dep.empty()) {
            a.dependencias.push_back(dep);
        }
    }

    return a;
}

// Arma el mapa inverso: dado un id, quien depende de el
std::map<std::string, std::vector<std::string>> construirDependientes(const std::vector<Actividad>& actividades) {
    std::map<std::string, std::vector<std::string>> dependientes;

    for (const Actividad& a : actividades) {
        for (const std::string& dep : a.dependencias) {
            dependientes[dep].push_back(a.id); // dep es esperado por a.id
        }
    }

    return dependientes;
}
 
int main() {
    std::ifstream archivo("plan.txt");
    if (!archivo.is_open()) {
        std::cout << "Error: no se pudo abrir el archivo plan.txt" << std::endl;
        return 1;
    }

    std::vector<Actividad> actividades;
    std::string linea;

    while (std::getline(archivo, linea)) {
        if (linea.empty()) continue; // salta lineas vacias, por si acaso
        Actividad a = parsearLinea(linea);
        actividades.push_back(a);
    }

    // Imprimir todo lo que se parseo, para verificar contra el plan.txt original
    for (const Actividad& a : actividades) {
        std::cout << "ID: " << a.id
                   << " | Nombre: " << a.nombre
                   << " | Tiempo: " << a.tiempo_ms << "ms"
                   << " | Depende de: ";
        for (const std::string& dep : a.dependencias) {
            std::cout << dep << " ";
        }
        std::cout << std::endl;
    }

    // Construye el mapa de dependientes
    auto dependientes = construirDependientes(actividades);

    // Imprime el mapa para verificar que quedo bien
    std::cout << "\n--- Dependientes ---" << std::endl;
    for (const auto& par : dependientes) {
        std::cout << "Actividad " << par.first << " es esperada por: ";
        for (const std::string& dep : par.second) {
            std::cout << dep << " ";
        }
        std::cout << std::endl;
    }

    return 0;
}
