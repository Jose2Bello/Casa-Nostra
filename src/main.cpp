#include <iostream>

#include "csv.h"

// Punto de entrada temporal de la infraestructura.
// Carga y valida el CSV y muestra las personas. En la siguiente fase se
// construira el arbol y se llamara a ejecutarMenu().
int main(int argc, char* argv[]) {
    const char* ruta = (argc > 1) ? argv[1] : "bin/datos.csv";

    ListaPersonas lista;
    inicializarLista(lista);

    char error[256];
    if (!cargarCSV(ruta, lista, error, static_cast<int>(sizeof(error)))) {
        std::cout << "Error al cargar el CSV:\n  " << error << "\n";
        liberarLista(lista);
        return 1;
    }

    std::cout << "Casa Nostra - personas cargadas: " << lista.cantidad << "\n\n";

    for (int i = 0; i < lista.cantidad; i++) {
        mostrarPersona(lista.datos[i]);
        std::cout << "\n";
    }

    liberarLista(lista);
    return 0;
}
