#include <iostream>

#include "arbol.h"
#include "csv.h"

// Punto de entrada temporal de la infraestructura.
// Carga el CSV, construye el arbol y muestra recorridos/busquedas.
// En la siguiente fase se llamara a ejecutarMenu(arbol).
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

    ArbolFamilia arbol;
    inicializarArbol(arbol);

    if (!construirArbol(lista, arbol, error, static_cast<int>(sizeof(error)))) {
        std::cout << "Error al construir el arbol:\n  " << error << "\n";
        liberarArbol(arbol);
        liberarLista(lista);
        return 1;
    }

    std::cout << "Casa Nostra - arbol construido\n";
    std::cout << "  raiz: " << arbol.raiz->id << " "
              << arbol.raiz->name << " " << arbol.raiz->last_name << "\n";
    std::cout << "  personas: " << arbol.cantidad << "\n\n";

    std::cout << "  pre-orden:  ";  mostrarPreOrden(arbol);
    std::cout << "  in-orden:   ";  mostrarInOrden(arbol);
    std::cout << "  post-orden: ";  mostrarPostOrden(arbol);
    std::cout << "  por niveles:";  mostrarPorNiveles(arbol);

    Persona* buscada = buscarPorId(arbol, 5);
    std::cout << "\n  buscarPorId(5): ";
    if (buscada != nullptr) {
        std::cout << buscada->name << " " << buscada->last_name;
    } else {
        std::cout << "no encontrado";
    }
    std::cout << "\n  buscarPorId(99): "
              << (buscarPorId(arbol, 99) != nullptr ? "encontrado" : "no encontrado")
              << "\n";

    Persona* michael = buscarPorId(arbol, 4);
    std::cout << "  padre de 4: "
              << (michael != nullptr && michael->padre != nullptr
                      ? michael->padre->id
                      : -1)
              << "\n";

    std::cout << "\nTODO: llamar a ejecutarMenu(arbol).\n";

    liberarArbol(arbol);
    liberarLista(lista);
    return 0;
}
