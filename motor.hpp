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

inline volatile sig_atomic_t seremi = 0;

inline void interceptarSeremi(int) {
    seremi = 1;
}

inline void ejecutarPlan(std::vector<Actividad>& actividades, int K) {
    std::set<std::string> completadas;      
    std::set<std::string> corriendoIds;     
    std::set<std::string> fallidas;       
    std::map<pid_t, std::string> pidToId;   
    std::map<pid_t, int> pidToFd;           
    std::signal(SIGINT, interceptarSeremi);
    
    int activos = 0;
    size_t totalActividades = actividades.size();
    size_t tareasCompletadas = 0; 

    std::set<std::string> ids_validos; // guardar ids q existen
    for (const Actividad& a : actividades) {
        ids_validos.insert(a.id);
    }

    while (tareasCompletadas < totalActividades && !seremi) {
        bool despachamos_alguno = false; // deadlock

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

            // cancelar rama
            if (abortar_errorprevio) {
                std::cout << "Padre: Cancelando actividad de  " << a.id << " porque su dependencia fallo o no existe.\n";
                fallidas.insert(a.id);
                tareasCompletadas++;
                continue;
            }

            if (!listas) continue; 

            int fd[2];  
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
            
            if (pid == 0) { 
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
            } else {
                close(fd[1]); 
                pidToId[pid] = a.id;
                pidToFd[pid] = fd[0];
                corriendoIds.insert(a.id);
                activos++;
                despachamos_alguno = true; // si existe algo
            }
        }

        // no hay trabajo
        if (activos == 0 && !despachamos_alguno) {
            std::cout << "Ejecución detenida debido a que no hay trabajo";
            break;
        }

        if (activos > 0) {
            int status;
            pid_t terminado = wait(&status); 
            
            if (terminado > 0) {
                std::string idTerminado = pidToId[terminado];
                int fdLectura = pidToFd[terminado];
                
                if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                    char buffer[256];
                    int n = read(fdLectura, buffer, sizeof(buffer) - 1);
                    if (n > 0) {
                        buffer[n] = '\0'; 
                        std::cout << "Padre -> Mensaje por pipe: " << buffer << std::endl;
                    }
                    completadas.insert(idTerminado);
                } else if(!seremi) {
                    std::cout << "Padre-> ERROR: El trabajador " << idTerminado << " ha fallado.";
                    fallidas.insert(idTerminado); //a la lista
                }

                close(fdLectura);
                corriendoIds.erase(idTerminado);
                pidToId.erase(terminado);
                pidToFd.erase(terminado);
                
                activos--; 
                tareasCompletadas++;
            }
        }
    }
    if (seremi) {
        std::cout << "\n LLEGO LA LEY - CERRANDO OPERACIONES\n";
        for (auto const& [pid, id] : pidToId) {
            std::cout << "ELIMINANDO TRABAJADOR: " << pid << "-> Actividad " << id << ")\n";
            kill(pid, SIGTERM); // asesinar procesos huerfanos
        }
        
        // limpiar para evitar procesos zombies
        while (activos > 0) {
            int status;
            if (wait(&status) > 0) activos--;
        }
        std::cout << "Planificador clausurado.\n";
    }
}

#endif
