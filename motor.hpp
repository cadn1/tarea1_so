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

inline void ejecutarPlan(std::vector<Actividad>& actividades, int K) {
    std::set<std::string> completadas;      
    std::set<std::string> corriendoIds;     
    std::map<pid_t, std::string> pidToId;   
    std::map<pid_t, int> pidToFd;           
    
    int activos = 0;
    size_t totalActividades = actividades.size();
    size_t completadasCount = 0;

    while (completadasCount < totalActividades) {
        
        for (Actividad& a : actividades) {
            if (completadas.count(a.id) || corriendoIds.count(a.id)) continue; 
            
            if (activos >= K) break; 
            bool listas = true;
            for (const std::string& dep : a.dependencias) {
                if (!completadas.count(dep)) { 
                    listas = false;
                    break; 
                }
            }
            if (!listas) continue; 


            int fd[2];  // pipe
            if (pipe(fd) == -1) {
                perror("Error creando el pipe");
                continue;
            }

            pid_t pid = fork(); //clonar
            
            if (pid < 0) {
                std::cout << "Error critico al clonar proceso (fork falló)." << std::endl;
                close(fd[0]); close(fd[1]);
                continue;
            }
            
            if (pid == 0) { //hijo
                close(fd[0]); // cerrar lectura
                
                std::cout << "[Hijo " << getpid() << "] Ejecutando actividad "
                           << a.id << " (" << a.nombre << ") por " << a.tiempo_ms << "ms" << std::endl;
                
                usleep(a.tiempo_ms * 1000); 

                std::string mensaje = "Actividad " + a.id + " (" + a.nombre + ") completada";
                write(fd[1], mensaje.c_str(), mensaje.size());
                close(fd[1]); // cerrer escritura

                std::cout << "[Hijo " << getpid() << "] Termino actividad " << a.id << std::endl;
                exit(0); // matar proceso
            } else {
                close(fd[1]); // cerrar escritura
                pidToId[pid] = a.id;
                pidToFd[pid] = fd[0];
                corriendoIds.insert(a.id);
                activos++;
            }
        }


        int status;
        pid_t terminado = wait(&status); // busy-waiting
        
        if (terminado > 0) {
            std::string idTerminado = pidToId[terminado];
            
            int fdLectura = pidToFd[terminado];
            char buffer[256];
            
            int n = read(fdLectura, buffer, sizeof(buffer) - 1);
            if (n > 0) {
                buffer[n] = '\0'; 
                std::cout << "[Padre] Mensaje por pipe: " << buffer << std::endl;
            }
            close(fdLectura);

            completadas.insert(idTerminado);
            corriendoIds.erase(idTerminado);
            
            pidToId.erase(terminado);
            pidToFd.erase(terminado);
            
            activos--; 
            completadasCount++;
        }
    }
}

#endif