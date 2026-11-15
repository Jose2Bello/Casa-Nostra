#include <iostream>
#include <string>

#include "persona.h"
#include "util.h"

// Punto de entrada temporal de la fase 1 (infraestructura).
// En la siguiente fase se reemplaza por:
//   - cargar bin/datos.csv
//   - construir el arbol binario
//   - llamar a ejecutarMenu(arbol)
int main() {
    std::cout << "Casa Nostra - infraestructura (fase 1)\n\n";

    std::string campos[MAX_CAMPOS_CSV];
    int cantidad = dividirCSV(
        "1, Vito, Corleone, H, 65, 0, 0, 0, 0, 1",
        campos, MAX_CAMPOS_CSV);

    std::cout << "Campos leidos: " << cantidad << "\n";
    for (int i = 0; i < cantidad; i++) {
        std::cout << "  [" << i << "] " << campos[i] << "\n";
    }

    Persona persona;
    convertirEntero(campos[0], persona.id);
    persona.name = campos[1];
    persona.last_name = campos[2];
    convertirGenero(campos[3], persona.gender);
    convertirEntero(campos[4], persona.age);
    convertirEntero(campos[5], persona.id_boss);
    convertirBooleano(campos[6], persona.is_dead);
    convertirBooleano(campos[7], persona.in_jail);
    convertirBooleano(campos[8], persona.was_boss);
    convertirBooleano(campos[9], persona.is_boss);

    std::cout << "\nPersona de prueba:\n";
    mostrarPersona(&persona);
    std::cout << "  estaVivo: " << std::boolalpha << estaVivo(&persona)
              << " | estaLibre: " << estaLibre(&persona) << "\n";

    std::cout << "\nTODO: cargar bin/datos.csv y llamar a ejecutarMenu().\n";
    return 0;
}
