#ifndef LECTOR_HPP
#define LECTOR_HPP

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <map>
#include <cstdlib>

struct Actividad { // fila del archivo plan.txt
    std::string id;
    std::string nombre;
    int tiempo_ms;
    std::vector<std::string> dependencias;
};

inline std::string quitarEspacios(const std::string& s) { // limpiar id pa evitar errores
    size_t inicio=s.find_first_not_of(" \t");  
    size_t fin=s.find_last_not_of(" \t");       
    if (inicio==std::string::npos) return "";   
    return s.substr(inicio, fin - inicio + 1);    
}

inline Actividad parsearLinea(const std::string& linea) { //cortar linea
    Actividad a;
    std::stringstream ss(linea);
    std::string campo;

    std::getline(ss, campo, ':'); // obtener id
    a.id=quitarEspacios(campo);

    std::getline(ss, campo, ':'); // obtener nombre
    a.nombre=quitarEspacios(campo);

    std::getline(ss, campo, ':'); // obtener tiempo
    campo=quitarEspacios(campo);
    if(campo.empty()) {
        a.tiempo_ms=100+rand()%(5000 - 100 + 1);
    } else {
        a.tiempo_ms=std::stoi(campo);
    }

    std::getline(ss, campo); // obtener dependencias
    campo=quitarEspacios(campo);
    std::stringstream depsStream(campo);
    std::string dep;
    while(std::getline(depsStream, dep, ',')) {
        dep=quitarEspacios(dep);
        if(!dep.empty()) {
            a.dependencias.push_back(dep);
        }
    }

    return a;
}

inline std::map<std::string, std::vector<std::string>> construirDependientes(const std::vector<Actividad>& actividades) { // arma el DAG inverso: id -> quien depende de el
    std::map<std::string, std::vector<std::string>> dependientes;
    for (const Actividad& a : actividades) {
        for (const std::string& dep : a.dependencias) {
            dependientes[dep].push_back(a.id); 
        }
    }
    return dependientes;
}

#endif
