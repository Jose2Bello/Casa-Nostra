#ifndef ARBOL_H
#define ARBOL_H

#include "csv.h"
#include "persona.h"

// Vista del arbol construido sobre la lista de personas.
// El arbol NO es duenio de los objetos Persona: solo los enlaza.
struct ArbolFamilia {
    Persona* raiz;
    Persona** indice;   // punteros ordenados por id
    int cantidad;
};

void inicializarArbol(ArbolFamilia& arbol);
void liberarArbol(ArbolFamilia& arbol);

// Construye izq/der/padre a partir de id_boss, en el orden del CSV:
// primer sucesor -> izq, segundo sucesor -> der.
// La lista debe haber sido validada por cargarCSV.
bool construirArbol(ListaPersonas& lista, ArbolFamilia& arbol,
                    char* error, int tamError);

// Busqueda binaria sobre el indice ordenado por id.
Persona* buscarPorId(const ArbolFamilia& arbol, int id);

// Recorridos de verificacion: imprimen los ids separados por espacios.
void mostrarPreOrden(const ArbolFamilia& arbol);
void mostrarInOrden(const ArbolFamilia& arbol);
void mostrarPostOrden(const ArbolFamilia& arbol);
void mostrarPorNiveles(const ArbolFamilia& arbol);

#endif
