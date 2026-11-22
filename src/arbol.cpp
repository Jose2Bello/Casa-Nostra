#include "arbol.h"

#include <iostream>
#include <string>

// ---------------------------------------------------------------------------
// Indice ordenado por id (insercion, sin contenedores de la STL)
// ---------------------------------------------------------------------------
static void ordenarIndice(Persona** datos, int cantidad) {
    for (int i = 1; i < cantidad; i++) {
        Persona* clave = datos[i];
        int j = i - 1;
        while (j >= 0 && datos[j]->id > clave->id) {
            datos[j + 1] = datos[j];
            j--;
        }
        datos[j + 1] = clave;
    }
}

void inicializarArbol(ArbolFamilia& arbol) {
    arbol.raiz = nullptr;
    arbol.indice = nullptr;
    arbol.cantidad = 0;
}

void liberarArbol(ArbolFamilia& arbol) {
    delete[] arbol.indice;
    arbol.raiz = nullptr;
    arbol.indice = nullptr;
    arbol.cantidad = 0;
}

// ---------------------------------------------------------------------------
// Busqueda binaria por id
// ---------------------------------------------------------------------------
Persona* buscarPorId(const ArbolFamilia& arbol, int id) {
    int bajo = 0;
    int alto = arbol.cantidad - 1;

    while (bajo <= alto) {
        int medio = bajo + (alto - bajo) / 2;
        int idActual = arbol.indice[medio]->id;

        if (idActual == id) return arbol.indice[medio];
        if (idActual < id) bajo = medio + 1;
        else alto = medio - 1;
    }

    return nullptr;
}

// ---------------------------------------------------------------------------
// Construccion del arbol
// ---------------------------------------------------------------------------
bool construirArbol(ListaPersonas& lista, ArbolFamilia& arbol,
                    std::string& error) {
    if (lista.cantidad == 0) {
        error = "No hay personas para construir el arbol";
        return false;
    }

    arbol.indice = new Persona*[lista.cantidad];
    arbol.cantidad = lista.cantidad;

    for (int i = 0; i < lista.cantidad; i++) {
        arbol.indice[i] = lista.datos[i];
    }
    ordenarIndice(arbol.indice, arbol.cantidad);

    Persona* raiz = nullptr;
    for (int i = 0; i < lista.cantidad; i++) {
        lista.datos[i]->izq = nullptr;
        lista.datos[i]->der = nullptr;
        lista.datos[i]->padre = nullptr;

        if (lista.datos[i]->id_boss == 0) raiz = lista.datos[i];
    }

    if (raiz == nullptr) {
        error = "No se encontro la raiz (id_boss=0)";
        liberarArbol(arbol);
        return false;
    }

    for (int i = 0; i < lista.cantidad; i++) {
        Persona* persona = lista.datos[i];
        if (persona->id_boss == 0) continue;

        Persona* jefe = buscarPorId(arbol, persona->id_boss);
        if (jefe == nullptr) {
            error = "La persona " + std::to_string(persona->id) +
                    " tiene un jefe inexistente (" +
                    std::to_string(persona->id_boss) + ")";
            liberarArbol(arbol);
            return false;
        }

        if (jefe->izq == nullptr) {
            jefe->izq = persona;
        } else if (jefe->der == nullptr) {
            jefe->der = persona;
        } else {
            error = "El jefe " + std::to_string(jefe->id) +
                    " tiene mas de 2 sucesores";
            liberarArbol(arbol);
            return false;
        }

        persona->padre = jefe;
    }

    arbol.raiz = raiz;
    return true;
}

// ---------------------------------------------------------------------------
// Recorridos
// ---------------------------------------------------------------------------
static void preOrdenEn(const Persona* nodo) {
    if (nodo == nullptr) return;
    std::cout << nodo->id << " ";
    preOrdenEn(nodo->izq);
    preOrdenEn(nodo->der);
}

static void inOrdenEn(const Persona* nodo) {
    if (nodo == nullptr) return;
    inOrdenEn(nodo->izq);
    std::cout << nodo->id << " ";
    inOrdenEn(nodo->der);
}

static void postOrdenEn(const Persona* nodo) {
    if (nodo == nullptr) return;
    postOrdenEn(nodo->izq);
    postOrdenEn(nodo->der);
    std::cout << nodo->id << " ";
}

void mostrarPreOrden(const ArbolFamilia& arbol) {
    if (arbol.raiz == nullptr) {
        std::cout << "(vacio)\n";
        return;
    }
    preOrdenEn(arbol.raiz);
    std::cout << "\n";
}

void mostrarInOrden(const ArbolFamilia& arbol) {
    if (arbol.raiz == nullptr) {
        std::cout << "(vacio)\n";
        return;
    }
    inOrdenEn(arbol.raiz);
    std::cout << "\n";
}

void mostrarPostOrden(const ArbolFamilia& arbol) {
    if (arbol.raiz == nullptr) {
        std::cout << "(vacio)\n";
        return;
    }
    postOrdenEn(arbol.raiz);
    std::cout << "\n";
}

void mostrarPorNiveles(const ArbolFamilia& arbol) {
    if (arbol.raiz == nullptr) {
        std::cout << "(vacio)\n";
        return;
    }

    Persona** cola = new Persona*[arbol.cantidad];
    int frente = 0;
    int fin = 0;
    cola[fin++] = arbol.raiz;

    while (frente < fin) {
        Persona* actual = cola[frente++];
        std::cout << actual->id << " ";

        if (actual->izq != nullptr) cola[fin++] = actual->izq;
        if (actual->der != nullptr) cola[fin++] = actual->der;
    }

    delete[] cola;
    std::cout << "\n";
}
