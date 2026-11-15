#include "persona.h"

#include <iostream>

bool estaVivo(const Persona* persona) {
    return persona != nullptr && !persona->is_dead;
}

bool estaLibre(const Persona* persona) {
    return estaVivo(persona) && !persona->in_jail;
}

void mostrarPersona(const Persona* persona) {
    if (persona == nullptr) {
        std::cout << "  (persona inexistente)\n";
        return;
    }

    std::cout << "  id: " << persona->id
              << " | nombre: " << persona->name << " " << persona->last_name
              << " | genero: " << persona->gender
              << " | edad: " << persona->age
              << " | jefe: " << persona->id_boss << "\n";
    std::cout << "  estado: " << (persona->is_dead ? "muerto" : "vivo")
              << " | " << (persona->in_jail ? "en la carcel" : "en libertad")
              << " | jefe actual: " << (persona->is_boss ? "si" : "no")
              << " | fue jefe: " << (persona->was_boss ? "si" : "no") << "\n";
}
