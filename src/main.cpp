#include <iostream>
#include <string>

#include "arbol.h"
#include "csv.h"
#include "menu.h"
#include "util.h"

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

// Carga el CSV, construye el arbol y lanza el menu interactivo.
int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    const char* ruta = (argc > 1) ? argv[1] : "bin/datos.csv";

    ListaPersonas lista;
    inicializarLista(lista);

    std::string error;
    if (!cargarCSV(ruta, lista, error)) {
        std::cout << "Error al cargar el CSV:\n  " << error << "\n";
        liberarLista(lista);
        return 1;
    }

    ArbolFamilia arbol;
    inicializarArbol(arbol);

    if (!construirArbol(lista, arbol, error)) {
        std::cout << "Error al construir el árbol:\n  " << error << "\n";
        liberarArbol(arbol);
        liberarLista(lista);
        return 1;
    }

    imprimirCentrado("Casa Nostra - " + std::to_string(arbol.cantidad) +
                         " personas cargadas.",
                     anchoConsola());

    ejecutarMenu(arbol);

    liberarArbol(arbol);
    liberarLista(lista);
    return 0;
}
