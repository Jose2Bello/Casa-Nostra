#include "menu.h"

#include <iostream>
#include <string>

#include "sucesion.h"
#include "util.h"

// ---------------------------------------------------------------------------
// Utilidades de entrada/salida del menu
// ---------------------------------------------------------------------------
static bool leerLinea(const std::string& prompt, std::string& salida) {
    std::cout << prompt;
    if (!std::getline(std::cin, salida)) {
        return false;
    }
    salida = trim(salida);
    return true;
}

static bool leerEnteroEnRango(const std::string& prompt, int minVal, int maxVal,
                              int& valor) {
    while (true) {
        std::string entrada;
        if (!leerLinea(prompt, entrada)) return false;

        if (convertirEntero(entrada, valor) &&
            valor >= minVal && valor <= maxVal) {
            return true;
        }
        std::cout << "  Valor invalido. Ingrese un entero entre "
                  << minVal << " y " << maxVal << ".\n";
    }
}

static bool leerBooleano(const std::string& prompt, bool& valor) {
    while (true) {
        std::string entrada;
        if (!leerLinea(prompt, entrada)) return false;

        if (convertirBooleano(entrada, valor)) return true;
        std::cout << "  Valor invalido. Ingrese 0 (no) o 1 (si).\n";
    }
}

static bool leerGenero(const std::string& prompt, char& valor) {
    while (true) {
        std::string entrada;
        if (!leerLinea(prompt, entrada)) return false;

        if (convertirGenero(entrada, valor)) return true;
        std::cout << "  Valor invalido. Ingrese H o M.\n";
    }
}

static bool leerTextoNoVacio(const std::string& prompt, std::string& valor) {
    while (true) {
        std::string entrada;
        if (!leerLinea(prompt, entrada)) return false;

        if (entrada.empty()) {
            std::cout << "  El texto no puede estar vacio.\n";
            continue;
        }
        if (entrada.find(',') != std::string::npos) {
            std::cout << "  El texto no puede contener comas.\n";
            continue;
        }
        valor = entrada;
        return true;
    }
}

// ---------------------------------------------------------------------------
// Normalizacion inicial de is_boss
// ---------------------------------------------------------------------------
static void normalizarEstadoInicial(ArbolFamilia& arbol) {
    int cantidadJefes = 0;
    Persona* primero = nullptr;
    for (int i = 0; i < arbol.cantidad; i++) {
        if (arbol.indice[i] != nullptr && arbol.indice[i]->is_boss) {
            if (primero == nullptr) primero = arbol.indice[i];
            cantidadJefes++;
        }
    }

    if (cantidadJefes > 1) {
        asignar_jefe(arbol, nullptr, primero);
    }

    Persona* jefe = buscar_jefe(arbol);
    if (jefe == nullptr || !es_elegible(jefe)) {
        std::string resultado = revisar_sucesion(arbol);
        std::cout << "[Revision inicial] " << resultado << "\n\n";
    }
}

// ---------------------------------------------------------------------------
// Vistas
// ---------------------------------------------------------------------------
static void mostrarNodoArbol(const Persona* nodo, int nivel,
                             const std::string& etiqueta) {
    if (nodo == nullptr) return;

    for (int i = 0; i < nivel; i++) {
        std::cout << "    ";
    }

    std::cout << etiqueta << " [id " << nodo->id << "] "
              << nodo->name << " " << nodo->last_name
              << " (" << nodo->age << " anos, " << nodo->gender << ")";

    if (nodo->is_boss) {
        std::cout << (nodo->in_jail ? " [JEFE INTERINO]" : " [JEFE]");
    }
    if (nodo->is_dead) std::cout << " [MUERTO]";
    if (nodo->in_jail) std::cout << " [CARCEL]";
    if (nodo->age > EDAD_MAXIMA_JEFE) std::cout << " [>70]";
    if (nodo->was_boss && !nodo->is_boss) std::cout << " [EX-JEFE]";
    std::cout << "\n";

    mostrarNodoArbol(nodo->izq, nivel + 1, "1er->");
    mostrarNodoArbol(nodo->der, nivel + 1, "2do->");
}

static void mostrarJefeActual(const ArbolFamilia& arbol) {
    Persona* jefe = buscar_jefe(arbol);
    std::cout << "\n--- Jefe actual ---\n";
    if (jefe == nullptr) {
        std::cout << "  [R7] La familia no tiene jefe actualmente (is_boss=0 para todos).\n";
        return;
    }

    if (es_jefe_interino(jefe)) {
        std::cout << "  [INTERINO EN CARCEL - R4]\n";
    }
    mostrarPersona(jefe);
}

static void mostrarArbolCompleto(const ArbolFamilia& arbol) {
    std::cout << "\n--- Arbol familiar ---\n";
    if (arbol.raiz == nullptr) {
        std::cout << "  (arbol vacio)\n";
        return;
    }

    mostrarNodoArbol(arbol.raiz, 0, "[Raiz]");
    std::cout << "\n  Recorridos (IDs):\n";
    std::cout << "    Pre-orden:   ";
    mostrarPreOrden(arbol);
    std::cout << "    In-orden:    ";
    mostrarInOrden(arbol);
    std::cout << "    Post-orden:  ";
    mostrarPostOrden(arbol);
    std::cout << "    Por niveles: ";
    mostrarPorNiveles(arbol);
}

static void buscarIntegrante(const ArbolFamilia& arbol) {
    int id = 0;
    if (!leerEnteroEnRango("\n  Ingrese el ID de la persona: ", 1, 1000000000,
                           id)) {
        return;
    }

    Persona* persona = buscarPorId(arbol, id);
    if (persona == nullptr) {
        std::cout << "  No se encontro ninguna persona con ID " << id << ".\n";
        return;
    }

    std::cout << "\n--- Datos del integrante ---\n";
    mostrarPersona(persona);
}

// ---------------------------------------------------------------------------
// Edicion de integrantes y disparadores de sucesion (R6)
// ---------------------------------------------------------------------------
static void editarIntegrante(ArbolFamilia& arbol) {
    int id = 0;
    if (!leerEnteroEnRango("\n  Ingrese el ID de la persona a editar: ",
                           1, 1000000000, id)) {
        return;
    }

    Persona* persona = buscarPorId(arbol, id);
    if (persona == nullptr) {
        std::cout << "  No se encontro ninguna persona con ID " << id << ".\n";
        return;
    }

    while (true) {
        std::cout << "\n--- Editando a [id " << persona->id << "] "
                  << persona->name << " " << persona->last_name << " ---\n";
        mostrarPersona(persona);
        std::cout << "\n  Campos editables (id e id_boss son fijos):\n"
                  << "    1. Nombre\n"
                  << "    2. Apellido\n"
                  << "    3. Genero (H/M)\n"
                  << "    4. Edad (0-120)\n"
                  << "    5. Estado vital (is_dead: 0=vivo, 1=muerto)\n"
                  << "    6. Carcel (in_jail: 0=libre, 1=preso)\n"
                  << "    7. Historico jefe (was_boss: 0/1)\n"
                  << "    8. Jefe actual (is_boss: 0/1)\n"
                  << "    0. Volver al menu principal\n";

        int opcion = -1;
        if (!leerEnteroEnRango("  Campo a editar: ", 0, 8, opcion)) return;
        if (opcion == 0) return;

        bool disparaRevision = false;

        switch (opcion) {
            case 1:
                if (!leerTextoNoVacio("  Nuevo nombre: ", persona->name)) {
                    return;
                }
                std::cout << "  Nombre actualizado.\n";
                break;
            case 2:
                if (!leerTextoNoVacio("  Nuevo apellido: ", persona->last_name)) {
                    return;
                }
                std::cout << "  Apellido actualizado.\n";
                break;
            case 3:
                if (!leerGenero("  Nuevo genero (H/M): ", persona->gender)) {
                    return;
                }
                std::cout << "  Genero actualizado.\n";
                break;
            case 4:
                if (!leerEnteroEnRango("  Nueva edad (0-120): ", 0, 120,
                                       persona->age)) {
                    return;
                }
                std::cout << "  Edad actualizada.\n";
                disparaRevision = true;
                break;
            case 5:
                if (!leerBooleano("  Esta muerto? (0=no, 1=si): ",
                                  persona->is_dead)) {
                    return;
                }
                std::cout << "  Estado vital actualizado.\n";
                disparaRevision = true;
                break;
            case 6:
                if (!leerBooleano("  Esta en la carcel? (0=no, 1=si): ",
                                  persona->in_jail)) {
                    return;
                }
                std::cout << "  Estado de carcel actualizado.\n";
                disparaRevision = true;
                break;
            case 7:
                if (!leerBooleano("  Fue jefe anteriormente? (0=no, 1=si): ",
                                  persona->was_boss)) {
                    return;
                }
                std::cout << "  Historico was_boss actualizado.\n";
                break;
            case 8: {
                bool nuevoJefe = false;
                if (!leerBooleano("  Asignar como jefe actual? (0=no, 1=si): ",
                                  nuevoJefe)) {
                    return;
                }
                if (nuevoJefe) {
                    asignar_jefe(arbol, buscar_jefe(arbol), persona);
                    std::cout << "  is_boss normalizado en [id " << persona->id
                              << "].\n";
                    disparaRevision = true;   // valida si el nuevo jefe es elegible
                } else if (persona->is_boss) {
                    persona->is_boss = false;
                    persona->was_boss = true;
                    std::cout << "  Se retiro el cargo a [id " << persona->id
                              << "]. La familia queda sin jefe; use la opcion 6 "
                              << "para recalcular.\n";
                    disparaRevision = false;  // respeta la edicion explicita
                } else {
                    std::cout << "  [id " << persona->id
                              << "] no era el jefe actual.\n";
                    disparaRevision = false;
                }
                break;
            }
            default:
                break;
        }

        if (disparaRevision) {
            std::string resultado = revisar_sucesion(arbol);
            std::cout << "  >> " << resultado << "\n";
        }
    }
}

// ---------------------------------------------------------------------------
// Bucle principal
// ---------------------------------------------------------------------------
void ejecutarMenu(ArbolFamilia& arbol) {
    normalizarEstadoInicial(arbol);

    while (true) {
        Persona* jefe = buscar_jefe(arbol);
        std::cout << "\n========================================\n"
                  << "  CASA NOSTRA - GESTION DE SUCESION\n";
        if (jefe == nullptr) {
            std::cout << "  Jefe actual: NINGUNO (sin sucesor - R7)\n";
        } else {
            std::cout << "  Jefe actual: [id " << jefe->id << "] "
                      << jefe->name << " " << jefe->last_name
                      << (es_jefe_interino(jefe) ? " [INTERINO]" : "") << "\n";
        }
        std::cout << "========================================\n"
                  << "  1. Ver jefe actual\n"
                  << "  2. Ver arbol familiar y recorridos\n"
                  << "  3. Ver linea de sucesion\n"
                  << "  4. Buscar integrante por ID\n"
                  << "  5. Editar integrante\n"
                  << "  6. Forzar revision de sucesion\n"
                  << "  0. Salir\n";

        int opcion = -1;
        if (!leerEnteroEnRango("Seleccione una opcion: ", 0, 6, opcion)) {
            std::cout << "\nFin de entrada. Saliendo...\n";
            return;
        }

        switch (opcion) {
            case 1:
                mostrarJefeActual(arbol);
                break;
            case 2:
                mostrarArbolCompleto(arbol);
                break;
            case 3:
                std::cout << "\n--- Linea de sucesion ---\n";
                mostrar_linea_sucesion(arbol);
                break;
            case 4:
                buscarIntegrante(arbol);
                break;
            case 5:
                editarIntegrante(arbol);
                break;
            case 6:
                std::cout << "\n--- Revision de sucesion ---\n  "
                          << revisar_sucesion(arbol) << "\n";
                break;
            case 0:
                std::cout << "Saliendo del sistema.\n";
                return;
            default:
                break;
        }
    }
}
