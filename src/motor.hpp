#ifndef MOTOR_HPP
#define MOTOR_HPP

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <unistd.h>
#include <sys/wait.h>
#include "lector.hpp"
#include <csignal>

inline volatile sig_atomic_t seremi = 0; // se activa si llega SIGINT

inline void interceptarSeremi(int) { // handler de SIGINT, solo marca
    seremi = 1;
}

inline void ejecutarPlan(std::vector<Actividad>& actividades, int K) { //fork, K, pipes, errores y SIGINT
    std::set<std::string> completadas;      
    std::set<std::string> corriendoIds;     
    std::set<std::string> fallidas;       
    std::map<pid_t, std::string> pidToId;   
    std::map<pid_t, int> pidToFd;           
    std::signal(SIGINT, interceptarSeremi);
    
    int activos = 0; //procesos que estan activos 
    size_t totalActividades = actividades.size(); // total de actividades 
    size_t tareasCompletadas = 0; //cuantas terminaron (las que salieron bien o fallaron)

    std::set<std::string> ids_validos; // guardar ids que existen
    for (const Actividad& a : actividades) {
        ids_validos.insert(a.id);
    }

    while (tareasCompletadas < totalActividades && !seremi) {
        bool despachamos_alguno = false; // evita el deadlock

        for (Actividad& a : actividades) {
            if (completadas.count(a.id) || fallidas.count(a.id) || corriendoIds.count(a.id)) continue; 
            
            if (activos >= K) break; 
            
            bool listas = true;
            bool abortar_errorprevio = false;

            for (const std::string& dep : a.dependencias) {
                if (fallidas.count(dep) || ids_validos.find(dep) == ids_validos.end()) {
                    abortar_errorprevio = true;
                    break;
                }
                if (!completadas.count(dep)) { 
                    listas = false;
                    break; 
                }
            }

            if (abortar_errorprevio) { // cancela esta rama por dependencia fallida
                std::cout << "Padre: Cancelando actividad de  " << a.id << " porque su dependencia fallo o no existe.\n";
                fallidas.insert(a.id);
                tareasCompletadas++;
                continue;
            }

            if (!listas) continue; 

            int fd[2]; // pipe para que el hijo avise al padre  
            if (pipe(fd) == -1) {
                perror("Error al crear el pipe");
                continue;
            }

            pid_t pid = fork(); 
            
            if (pid < 0) {
                std::cout << "Fallo en el forkk." << std::endl;
                close(fd[0]); close(fd[1]);
                continue;
            }
            
            if (pid == 0) { // proceso hijo
                close(fd[0]); 
                
                std::cout << "Hijo " << getpid() << " ejecutando actividad "
                           << a.id << " (" << a.nombre << ") durante " << a.tiempo_ms << "ms" << std::endl;
                
                usleep(a.tiempo_ms * 1000); 
		
		// prueba: esta actividad falla a proposito para probar el aislamiento pueden utilizarla si gustan
                /*if (a.nombre == "actividad_falla") {
                    close(fd[1]);
                    exit(1);
                }*/

                std::string mensaje = "Actividad " + a.id + " (" + a.nombre + ") completada";
                write(fd[1], mensaje.c_str(), mensaje.size());
                close(fd[1]); 

                std::cout << "Hijo " << getpid() << " terminó la actividad " << a.id << std::endl;
                
                exit(0); 
            } else { // proceso padre
                close(fd[1]); 
                pidToId[pid] = a.id;
                pidToFd[pid] = fd[0];
                corriendoIds.insert(a.id);
                activos++;
                despachamos_alguno = true; // si existe algo
            }
        }
        // no hay trabajo
        if (activos == 0 && !despachamos_alguno) { // no queda nada por hacer
            std::cout << "Ejecución detenida debido a que no hay trabajo";
            break;
        }

        if (activos > 0) {
            int status;
            pid_t terminado = wait(&status); // bloquea sin busy waiting
            
            if (terminado > 0) {
                std::string idTerminado = pidToId[terminado];
                int fdLectura = pidToFd[terminado];
                
                if (WIFEXITED(status) && WEXITSTATUS(status) == 0) { // termino bien
                    char buffer[256];
                    int n = read(fdLectura, buffer, sizeof(buffer) - 1);
                    if (n > 0) {
                        buffer[n] = '\0'; 
                        std::cout << "Padre -> Mensaje por pipe: " << buffer << std::endl;
                    }
                    completadas.insert(idTerminado);
                } else if(!seremi) { // termino con error
                    std::cout << "Padre-> ERROR: El trabajador " << idTerminado << " ha fallado.";
                    fallidas.insert(idTerminado);
                }

                close(fdLectura); // cierre del pipe
                corriendoIds.erase(idTerminado); // ya no esta corriendo
                pidToId.erase(terminado); // aca limpia su pid procesado
                pidToFd.erase(terminado); // limpia el pid guardado :)
                
                activos--; // Libera un cupo de K
                tareasCompletadas++; 
            }
        }
    }
    if (seremi) { // llego Ctrl+C: matar hijos activos y limpiar zombies
        std::cout << "\n LLEGO LA LEY - CERRANDO OPERACIONES\n";
        for (auto const& [pid, id] : pidToId) {
            std::cout << "ELIMINANDO TRABAJADOR: " << pid << "-> Actividad " << id << ")\n";
            kill(pid, SIGTERM); // asesinar procesos huerfanos
        }
        
        while (activos > 0) { // limpia para evitar procesos zombies
            int status;
            if (wait(&status) > 0) activos--;
        }
        std::cout << "Planificador clausurado.\n";
    }
}

#endif
