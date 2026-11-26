#include "sucesion.h"

#include <iostream>
#include <string>

// ---------------------------------------------------------------------------
// Helpers de busqueda
// ---------------------------------------------------------------------------

// Pre-orden: nodo, izquierda, derecha.
static Persona* primerEnSubarbol(Persona* raiz,
                                 bool (*predicado)(const Persona*)) {
    if (raiz == nullptr) return nullptr;
    if (predicado(raiz)) return raiz;

    Persona* encontrado = primerEnSubarbol(raiz->izq, predicado);
    if (encontrado != nullptr) return encontrado;

    return primerEnSubarbol(raiz->der, predicado);
}

// Busca primero en los descendientes y deja a la raiz para el final.
// Se usa para el hermano y el tio: si tienen sucesores, ellos van primero;
// si no tienen, el propio hermano/tio puede asumir.
static Persona* primerEnRamaSinRaiz(Persona* raiz,
                                    bool (*predicado)(const Persona*)) {
    if (raiz == nullptr) return nullptr;

    Persona* encontrado = primerEnSubarbol(raiz->izq, predicado);
    if (encontrado != nullptr) return encontrado;

    encontrado = primerEnSubarbol(raiz->der, predicado);
    if (encontrado != nullptr) return encontrado;

    return predicado(raiz) ? raiz : nullptr;
}

static Persona* otroHijo(Persona* padre, Persona* hijo) {
    if (padre == nullptr || hijo == nullptr) return nullptr;
    if (padre->izq == hijo) return padre->der;
    if (padre->der == hijo) return padre->izq;
    return nullptr;
}

// ---------------------------------------------------------------------------
// Elegibilidad
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
// Jefe actual
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
// Línea de sucesión para mostrar
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
        if (predicado(orden[idx])) cantidad++;
    }

    if (cantidad == 0) {
        delete[] orden;
        return nullptr;
    }

    Persona** resultado = new Persona*[cantidad];
    int pos = 0;
    for (int k = inicio; k < total; k++) {
        int idx = (posJefe == -1) ? k : (posJefe + k) % total;
        if (predicado(orden[idx])) resultado[pos++] = orden[idx];
    }

    delete[] orden;
    return resultado;
}

Persona** construir_linea_sucesion(const ArbolFamilia& arbol, int& cantidad) {
    return construirLineaFiltrada(arbol, es_elegible, cantidad);
}

void mostrar_linea_sucesion(const ArbolFamilia& arbol) {
    int cantidad = 0;
    Persona** linea = construir_linea_sucesion(arbol, cantidad);

    if (cantidad > 0) {
        std::cout << "  Línea de sucesión (elegibles libres, pre-orden):\n";
        for (int i = 0; i < cantidad; i++) {
            std::cout << "    " << (i + 1) << ". [id " << linea[i]->id << "] "
                      << linea[i]->name << " " << linea[i]->last_name
                      << " (edad " << linea[i]->age << ")\n";
        }
        delete[] linea;
        return;
    }

    std::cout << "  No hay sucesores libres elegibles.\n";

    int cantInterinos = 0;
    Persona** interinos =
        construirLineaFiltrada(arbol, es_elegible_interino, cantInterinos);

    if (cantInterinos > 0) {
        std::cout << "  Candidatos a jefe interino (presos elegibles):\n";
        for (int i = 0; i < cantInterinos; i++) {
            std::cout << "    " << (i + 1) << ". [id " << interinos[i]->id
                      << "] " << interinos[i]->name << " "
                      << interinos[i]->last_name << " (edad "
                      << interinos[i]->age << ", en cárcel)\n";
        }
        delete[] interinos;
        return;
    }

    Persona* jefe = buscar_jefe(arbol);
    if (es_jefe_interino(jefe) && estaVivo(jefe) &&
        jefe->age <= EDAD_MAXIMA_JEFE) {
        std::cout << "  El jefe actual [id " << jefe->id << "] " << jefe->name
                  << " " << jefe->last_name
                  << " continúa como interino desde la cárcel.\n";
    } else {
        std::cout << "  No hay candidatos interinos disponibles.\n";
    }
}

// ---------------------------------------------------------------------------
// R1 - R5: busqueda de un sucesor libre
// ---------------------------------------------------------------------------
static Persona* buscar_sucesor_libre(Persona* jefe, Persona* raiz) {
    if (jefe == nullptr) {
        return primerEnSubarbol(raiz, es_elegible);
    }

    // R1: primer elegible en el arbol del jefe (sus descendientes).
    Persona* encontrado = primerEnSubarbol(jefe->izq, es_elegible);
    if (encontrado != nullptr) return encontrado;

    encontrado = primerEnSubarbol(jefe->der, es_elegible);
    if (encontrado != nullptr) return encontrado;

    bool tieneSucesores = (jefe->izq != nullptr || jefe->der != nullptr);
    Persona* padre = jefe->padre;

    if (!tieneSucesores && padre != nullptr) {
        // R2 / R3: arbol del companero sucesor (hermano).
        Persona* hermano = otroHijo(padre, jefe);
        if (hermano != nullptr) {
            encontrado = primerEnRamaSinRaiz(hermano, es_elegible);
            if (encontrado != nullptr) return encontrado;
        }

        // R4: arbol del companero sucesor del jefe anterior (tio).
        Persona* abuelo = padre->padre;
        if (abuelo != nullptr) {
            Persona* tio = otroHijo(abuelo, padre);
            if (tio != nullptr) {
                encontrado = primerEnRamaSinRaiz(tio, es_elegible);
                if (encontrado != nullptr) return encontrado;
            }
        }
    }

    // R5: ancestro mas cercano con dos sucesores libres.
    for (Persona* ancestro = padre; ancestro != nullptr;
         ancestro = ancestro->padre) {
        if (ancestro->izq != nullptr && ancestro->der != nullptr &&
            es_elegible(ancestro->izq) && es_elegible(ancestro->der)) {
            encontrado = primerEnSubarbol(ancestro->izq, es_elegible);
            if (encontrado != nullptr) return encontrado;

            encontrado = primerEnSubarbol(ancestro->der, es_elegible);
            if (encontrado != nullptr) return encontrado;
        }
    }

    // Respaldo: primer elegible libre del arbol completo.
    return primerEnSubarbol(raiz, es_elegible);
}

// ---------------------------------------------------------------------------
// R6: presos vivos, empezando por el mas cercano al jefe actual
// ---------------------------------------------------------------------------
static Persona* buscar_preso_cercano(Persona* inicio, Persona* excluir,
                                     int capacidad) {
    if (inicio == nullptr || capacidad <= 0) return nullptr;

    Persona** cola = new Persona*[capacidad];
    Persona** visitados = new Persona*[capacidad];
    int frente = 0;
    int fin = 0;
    int cantidadVisitados = 0;

    cola[fin++] = inicio;
    visitados[cantidadVisitados++] = inicio;

    while (frente < fin) {
        Persona* actual = cola[frente++];

        if (actual != excluir && es_elegible_interino(actual)) {
            delete[] cola;
            delete[] visitados;
            return actual;
        }

        Persona* vecinos[3] = {actual->padre, actual->izq, actual->der};
        for (int i = 0; i < 3; i++) {
            Persona* vecino = vecinos[i];
            if (vecino == nullptr) continue;

            bool visto = false;
            for (int j = 0; j < cantidadVisitados; j++) {
                if (visitados[j] == vecino) { visto = true; break; }
            }

            if (!visto && fin < capacidad) {
                visitados[cantidadVisitados++] = vecino;
                cola[fin++] = vecino;
            }
        }
    }

    delete[] cola;
    delete[] visitados;
    return nullptr;
}

// ---------------------------------------------------------------------------
// Busqueda principal
// ---------------------------------------------------------------------------
Persona* buscar_sucesor(const ArbolFamilia& arbol) {
    Persona* jefe = buscar_jefe(arbol);

    // Si el jefe actual sigue libre, vivo y <= 70, conserva el cargo.
    if (jefe != nullptr && es_elegible(jefe)) return nullptr;

    Persona* libre = buscar_sucesor_libre(jefe, arbol.raiz);
    if (libre != nullptr) return libre;

    // Un interino preso vigente se mantiene si no hay libres.
    if (jefe != nullptr && es_elegible_interino(jefe)) return nullptr;

    Persona* inicio = (jefe != nullptr) ? jefe : arbol.raiz;
    return buscar_preso_cercano(inicio, jefe, arbol.cantidad);
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
        entrante->was_boss = true;
    }
}

static std::string formatearPersona(const Persona* persona) {
    if (persona == nullptr) return "(ninguno)";
    return persona->name + " " + persona->last_name +
           " (id " + std::to_string(persona->id) + ")";
}

std::string revisar_sucesion(ArbolFamilia& arbol) {
    Persona* jefe = buscar_jefe(arbol);

    if (jefe != nullptr && es_elegible(jefe)) {
        return "El jefe actual " + formatearPersona(jefe) +
               " sigue siendo elegible y conserva el cargo.";
    }

    Persona* sucesor = buscar_sucesor(arbol);

    if (sucesor != nullptr) {
        bool interino = es_elegible_interino(sucesor);
        asignar_jefe(arbol, jefe, sucesor);

        if (interino) {
            std::string mensaje = "[Interinato] " +
                                  formatearPersona(sucesor) +
                                  " asume como jefe interino desde la cárcel";
            if (jefe != nullptr) {
                mensaje += " en reemplazo de " + formatearPersona(jefe);
            }
            return mensaje + ".";
        }

        std::string mensaje = "[Sucesión] " + formatearPersona(sucesor) +
                              " asume como nuevo jefe";
        if (jefe != nullptr) {
            mensaje += " en reemplazo de " + formatearPersona(jefe);
        }
        return mensaje + ".";
    }

    if (jefe != nullptr && es_elegible_interino(jefe)) {
        return "[Interinato vigente] No hay sucesores libres; " +
               formatearPersona(jefe) +
               " continúa como jefe interino desde la cárcel.";
    }

    if (jefe != nullptr) {
        asignar_jefe(arbol, jefe, nullptr);
        return "[Sin sucesor] " + formatearPersona(jefe) +
               " deja el cargo y no hay candidatos libres ni interinos. "
               "La familia queda sin jefe.";
    }

    return "[Sin sucesor] La familia continúa sin jefe; no hay candidatos "
           "elegibles.";
}
