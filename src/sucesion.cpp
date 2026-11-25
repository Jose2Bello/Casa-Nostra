#include "sucesion.h"

// Motor de sucesion. La implementacion se completa por etapas.

bool es_elegible(const Persona* persona) {
    (void)persona;
    return false;
}

bool es_elegible_interino(const Persona* persona) {
    (void)persona;
    return false;
}

bool es_jefe_interino(const Persona* persona) {
    (void)persona;
    return false;
}

Persona* buscar_jefe(const ArbolFamilia& arbol) {
    (void)arbol;
    return nullptr;
}

Persona** construir_linea_sucesion(const ArbolFamilia& arbol, int& cantidad) {
    (void)arbol;
    cantidad = 0;
    return nullptr;
}

void mostrar_linea_sucesion(const ArbolFamilia& arbol) {
    (void)arbol;
}

Persona* buscar_sucesor(const ArbolFamilia& arbol) {
    (void)arbol;
    return nullptr;
}

void asignar_jefe(ArbolFamilia& arbol, Persona* saliente, Persona* entrante) {
    (void)arbol;
    (void)saliente;
    (void)entrante;
}

std::string revisar_sucesion(ArbolFamilia& arbol) {
    (void)arbol;
    return "";
}
