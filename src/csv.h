#ifndef CSV_H
#define CSV_H

#include <string>

#include "persona.h"

// Arreglo dinamico de personas cargadas desde el CSV.
// Es el duenio de los objetos Persona hasta que se llame a liberarLista.
struct ListaPersonas {
    Persona** datos;
    int cantidad;
    int capacidad;
};

void inicializarLista(ListaPersonas& lista);
void liberarLista(ListaPersonas& lista);

// Carga y valida el archivo CSV indicado.
// Devuelve true si todo esta correcto. Si falla, deja el motivo en `error`
// y libera lo que haya reservado.
bool cargarCSV(const char* ruta, ListaPersonas& lista, std::string& error);

#endif
