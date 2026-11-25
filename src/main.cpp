#include <iostream>
#include <string>

#include "arbol.h"
#include "csv.h"
#include "menu.h"

// Carga el CSV, construye el arbol y lanza el menu interactivo.
int main(int argc, char* argv[]) {
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
        std::cout << "Error al construir el arbol:\n  " << error << "\n";
        liberarArbol(arbol);
        liberarLista(lista);
        return 1;
    }

    std::cout << "Casa Nostra - " << arbol.cantidad
              << " personas cargadas.\n";

    ejecutarMenu(arbol);

    liberarArbol(arbol);
    liberarLista(lista);
    return 0;
}
