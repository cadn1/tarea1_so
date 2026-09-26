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
	std::cout << "Compilacion ready" <<std::endl;
	return 0;
}
