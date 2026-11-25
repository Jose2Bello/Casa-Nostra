#include "sucesion.h"

#include <iostream>
#include <string>

// ---------------------------------------------------------------------------
// Helpers internos
// ---------------------------------------------------------------------------
static void llenarPreOrden(Persona* nodo, Persona** orden, int& idx,
                           int maxCantidad) {
    if (nodo == nullptr || idx >= maxCantidad) return;
    orden[idx++] = nodo;
    llenarPreOrden(nodo->izq, orden, idx, maxCantidad);
    llenarPreOrden(nodo->der, orden, idx, maxCantidad);
}

static Persona** construirLineaFiltrada(const ArbolFamilia& arbol,
                                        bool (*predicado)(const Persona*),
                                        int& cantidad) {
    cantidad = 0;
    if (arbol.raiz == nullptr || arbol.cantidad <= 0) return nullptr;

    Persona** orden = new Persona*[arbol.cantidad];
    int total = 0;
    llenarPreOrden(arbol.raiz, orden, total, arbol.cantidad);

    if (total == 0) {
        delete[] orden;
        return nullptr;
    }

    Persona* jefe = buscar_jefe(arbol);
    int posJefe = -1;
    if (jefe != nullptr) {
        for (int i = 0; i < total; i++) {
            if (orden[i] == jefe) {
                posJefe = i;
                break;
            }
        }
    }

    int inicio = (posJefe == -1) ? 0 : 1;
    for (int k = inicio; k < total; k++) {
        int idx = (posJefe == -1) ? k : (posJefe + k) % total;
        if (predicado(orden[idx])) {
            cantidad++;
        }
    }

    if (cantidad == 0) {
        delete[] orden;
        return nullptr;
    }

    Persona** resultado = new Persona*[cantidad];
    int pos = 0;
    for (int k = inicio; k < total; k++) {
        int idx = (posJefe == -1) ? k : (posJefe + k) % total;
        if (predicado(orden[idx])) {
            resultado[pos++] = orden[idx];
        }
    }

    delete[] orden;
    return resultado;
}

static std::string formatearPersona(const Persona* persona) {
    if (persona == nullptr) return "(ninguno)";
    return persona->name + " " + persona->last_name +
           " (id " + std::to_string(persona->id) + ")";
}

// ---------------------------------------------------------------------------
// R2 / R4: Elegibilidad
// ---------------------------------------------------------------------------
bool es_elegible(const Persona* persona) {
    return estaLibre(persona) && persona->age <= EDAD_MAXIMA_JEFE;
}

bool es_elegible_interino(const Persona* persona) {
    return estaVivo(persona) && persona->in_jail &&
           persona->age <= EDAD_MAXIMA_JEFE;
}

bool es_jefe_interino(const Persona* persona) {
    return persona != nullptr && persona->is_boss && persona->in_jail;
}

// ---------------------------------------------------------------------------
// Busqueda de jefe actual
// ---------------------------------------------------------------------------
Persona* buscar_jefe(const ArbolFamilia& arbol) {
    if (arbol.indice == nullptr || arbol.cantidad <= 0) return nullptr;

    for (int i = 0; i < arbol.cantidad; i++) {
        if (arbol.indice[i] != nullptr && arbol.indice[i]->is_boss) {
            return arbol.indice[i];
        }
    }
    return nullptr;
}

// ---------------------------------------------------------------------------
// R1: Linea de sucesion (pre-orden ciclico desde el jefe)
// ---------------------------------------------------------------------------
Persona** construir_linea_sucesion(const ArbolFamilia& arbol, int& cantidad) {
    return construirLineaFiltrada(arbol, es_elegible, cantidad);
}

void mostrar_linea_sucesion(const ArbolFamilia& arbol) {
    Persona* jefe = buscar_jefe(arbol);
    int cantidad = 0;
    Persona** linea = construir_linea_sucesion(arbol, cantidad);

    if (cantidad > 0) {
        std::cout << "  Linea de sucesion (R1/R2 - elegibles libres):\n";
        for (int i = 0; i < cantidad; i++) {
            std::cout << "    " << (i + 1) << ". [id " << linea[i]->id << "] "
                      << linea[i]->name << " " << linea[i]->last_name
                      << " (edad " << linea[i]->age << ")";
            if (i == 0 && es_jefe_interino(jefe)) {
                std::cout << "  [relevo disponible]";
            }
            std::cout << "\n";
        }
        delete[] linea;
        return;
    }

    std::cout << "  No hay sucesores libres elegibles.\n";

    int cantInterinos = 0;
    Persona** interinos =
        construirLineaFiltrada(arbol, es_elegible_interino, cantInterinos);

    if (cantInterinos > 0) {
        std::cout << "  Candidatos a jefe interino (R4 - presos elegibles):\n";
        for (int i = 0; i < cantInterinos; i++) {
            std::cout << "    " << (i + 1) << ". [id " << interinos[i]->id
                      << "] " << interinos[i]->name << " "
                      << interinos[i]->last_name << " (edad "
                      << interinos[i]->age << ", en carcel)\n";
        }
        delete[] interinos;
        return;
    }

    if (es_jefe_interino(jefe) && estaVivo(jefe) &&
        jefe->age <= EDAD_MAXIMA_JEFE) {
        std::cout << "  El jefe actual [id " << jefe->id << "] " << jefe->name
                  << " " << jefe->last_name
                  << " continua como interino desde la carcel.\n";
    } else {
        std::cout << "  No hay candidatos interinos disponibles (R7).\n";
    }
}

// ---------------------------------------------------------------------------
// R3 / R4 / R5 / R7: Calculo y asignacion de sucesor
// ---------------------------------------------------------------------------
Persona* buscar_sucesor(const ArbolFamilia& arbol) {
    Persona* jefe = buscar_jefe(arbol);

    // Si el jefe actual sigue siendo elegible (vivo, libre, <= 70), conserva el cargo.
    if (jefe != nullptr && es_elegible(jefe)) {
        return nullptr;
    }

    // Prioridad 1 (R3 y R5): primer candidato libre elegible en pre-orden ciclico.
    int cantLibres = 0;
    Persona** libres = construir_linea_sucesion(arbol, cantLibres);
    if (cantLibres > 0) {
        Persona* sucesor = libres[0];
        delete[] libres;
        return sucesor;
    }
    delete[] libres;

    // Si hay un interino preso vigente (vivo y <= 70) y nadie libre, sigue en el cargo.
    if (jefe != nullptr && es_jefe_interino(jefe) && estaVivo(jefe) &&
        jefe->age <= EDAD_MAXIMA_JEFE) {
        return nullptr;
    }

    // Prioridad 2 (R4): primer candidato preso vivo y <= 70.
    int cantInterinos = 0;
    Persona** interinos =
        construirLineaFiltrada(arbol, es_elegible_interino, cantInterinos);
    if (cantInterinos > 0) {
        Persona* interino = interinos[0];
        delete[] interinos;
        return interino;
    }
    delete[] interinos;

    // R7: no hay sucesor libre ni interino.
    return nullptr;
}

void asignar_jefe(ArbolFamilia& arbol, Persona* saliente, Persona* entrante) {
    if (arbol.indice != nullptr) {
        for (int i = 0; i < arbol.cantidad; i++) {
            Persona* actual = arbol.indice[i];
            if (actual != nullptr && actual->is_boss && actual != entrante) {
                actual->is_boss = false;
                actual->was_boss = true;
            }
        }
    }

    if (saliente != nullptr && saliente != entrante) {
        saliente->is_boss = false;
        saliente->was_boss = true;
    }

    if (entrante != nullptr) {
        entrante->is_boss = true;
    }
}

std::string revisar_sucesion(ArbolFamilia& arbol) {
    Persona* jefe = buscar_jefe(arbol);

    if (jefe != nullptr && es_elegible(jefe)) {
        return "El jefe actual " + formatearPersona(jefe) +
               " sigue siendo elegible y conserva el cargo.";
    }

    Persona* sucesor = buscar_sucesor(arbol);
    if (sucesor != nullptr) {
        bool jefeEraInterino = (jefe != nullptr && es_jefe_interino(jefe) &&
                                estaVivo(jefe) && jefe->age <= EDAD_MAXIMA_JEFE);
        asignar_jefe(arbol, jefe, sucesor);

        if (es_jefe_interino(sucesor)) {
            std::string msg = "[R4 - Interinato] " + formatearPersona(sucesor) +
                              " asume como jefe interino desde la carcel";
            if (jefe != nullptr) {
                msg += " en reemplazo de " + formatearPersona(jefe);
            }
            return msg + ".";
        }

        if (jefeEraInterino) {
            return "[R5 - Relevo] " + formatearPersona(jefe) +
                   " entrega el interinato a " + formatearPersona(sucesor) + ".";
        }

        std::string msg = "[R3 - Sucesion] " + formatearPersona(sucesor) +
                          " asume como nuevo jefe";
        if (jefe != nullptr) {
            msg += " en reemplazo de " + formatearPersona(jefe);
        }
        return msg + ".";
    }

    if (jefe != nullptr && es_jefe_interino(jefe) && estaVivo(jefe) &&
        jefe->age <= EDAD_MAXIMA_JEFE) {
        return "[R4 - Interinato vigente] No hay sucesores libres; " +
               formatearPersona(jefe) +
               " continua como jefe interino desde la carcel.";
    }

    if (jefe != nullptr) {
        asignar_jefe(arbol, jefe, nullptr);
        return "[R7 - Sin sucesor] " + formatearPersona(jefe) +
               " deja el cargo y no hay candidatos libres ni interinos. "
               "La familia queda sin jefe.";
    }

    return "[R7 - Sin sucesor] La familia continua sin jefe; no hay "
           "candidatos elegibles.";
}
