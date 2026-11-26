#include "menu.h"

#include <iostream>
#include <string>

#include "sucesion.h"
#include "util.h"

static const int ANCHO_CAJA = 54;

// ---------------------------------------------------------------------------
// Entrada
// ---------------------------------------------------------------------------
static bool leerLinea(const std::string& prompt, std::string& salida) {
    std::cout << prompt;
    if (!std::getline(std::cin, salida)) return false;
    salida = trim(salida);
    return true;
}

static bool leerEnteroEnRango(const std::string& prompt, int minimo, int maximo,
                              int& valor) {
    while (true) {
        std::string entrada;
        if (!leerLinea(prompt, entrada)) return false;

        if (convertirEntero(entrada, valor) &&
            valor >= minimo && valor <= maximo) {
            return true;
        }

        imprimirCentrado("Valor inválido. Ingrese un entero entre " +
                             std::to_string(minimo) + " y " +
                             std::to_string(maximo) + ".",
                         anchoConsola());
    }
}

static bool leerBooleano(const std::string& prompt, bool& valor) {
    while (true) {
        std::string entrada;
        if (!leerLinea(prompt, entrada)) return false;
        if (convertirBooleano(entrada, valor)) return true;

        imprimirCentrado("Valor inválido. Ingrese 0 (no) o 1 (sí).",
                         anchoConsola());
    }
}

static bool leerGenero(const std::string& prompt, char& valor) {
    while (true) {
        std::string entrada;
        if (!leerLinea(prompt, entrada)) return false;
        if (convertirGenero(entrada, valor)) return true;

        imprimirCentrado("Valor inválido. Ingrese H o M.", anchoConsola());
    }
}

static bool leerTextoNoVacio(const std::string& prompt, std::string& valor) {
    while (true) {
        std::string entrada;
        if (!leerLinea(prompt, entrada)) return false;

        if (entrada.empty()) {
            imprimirCentrado("El texto no puede estar vacío.", anchoConsola());
            continue;
        }
        if (entrada.find(',') != std::string::npos) {
            imprimirCentrado("El texto no puede contener comas.", anchoConsola());
            continue;
        }

        valor = entrada;
        return true;
    }
}

// ---------------------------------------------------------------------------
// Dibujo de la caja del menú
// ---------------------------------------------------------------------------
static std::string recortarVisible(const std::string& texto, int ancho) {
    int visibles = 0;
    int i = 0;

    while (i < static_cast<int>(texto.size())) {
        unsigned char byte = static_cast<unsigned char>(texto[i]);
        if ((byte & 0xC0) != 0x80) {
            if (visibles == ancho) break;
            visibles++;
        }
        i++;
    }

    return texto.substr(0, i);
}

static std::string rellenarDerecha(const std::string& texto, int ancho) {
    std::string recortado = recortarVisible(texto, ancho);
    int longitud = longitudVisible(recortado);
    if (longitud < ancho) {
        recortado += std::string(ancho - longitud, ' ');
    }
    return recortado;
}

static void bordeSuperior() {
    imprimirCentrado("╔" + repetir("═", ANCHO_CAJA) + "╗", anchoConsola());
}

static void bordeMedio() {
    imprimirCentrado("╠" + repetir("═", ANCHO_CAJA) + "╣", anchoConsola());
}

static void bordeInferior() {
    imprimirCentrado("╚" + repetir("═", ANCHO_CAJA) + "╝", anchoConsola());
}

static void fila(const std::string& texto) {
    std::string contenido = " " + texto;
    contenido = rellenarDerecha(contenido, ANCHO_CAJA);
    imprimirCentrado("║" + contenido + "║", anchoConsola());
}

static void dibujarMenu(const ArbolFamilia& arbol) {
    Persona* jefe = buscar_jefe(arbol);

    bordeSuperior();
    fila("CASA NOSTRA");
    fila("Gestión de la sucesión familiar");
    bordeMedio();

    if (jefe != nullptr) {
        std::string estado = "Jefe actual: " + jefe->name + " " +
                             jefe->last_name + " (id " +
                             std::to_string(jefe->id) + ")";
        if (es_jefe_interino(jefe)) estado += " [INTERINO]";
        fila(estado);
    } else {
        fila("Jefe actual: NINGUNO");
    }

    fila("Personas registradas: " + std::to_string(arbol.cantidad));
    bordeInferior();

    std::cout << "\n";

    bordeSuperior();
    fila("1. Ver jefe actual");
    fila("2. Ver árbol familiar y recorridos");
    fila("3. Ver línea de sucesión");
    fila("4. Buscar integrante por ID");
    fila("5. Editar integrante");
    fila("6. Forzar revisión de sucesión");
    bordeMedio();
    fila("0. Salir");
    bordeInferior();

    std::cout << "\n";
}

// ---------------------------------------------------------------------------
// Normalizacion inicial
// ---------------------------------------------------------------------------
static std::string normalizarEstadoInicial(ArbolFamilia& arbol) {
    int cantidadJefes = 0;
    Persona* primero = nullptr;

    for (int i = 0; i < arbol.cantidad; i++) {
        if (arbol.indice[i] != nullptr && arbol.indice[i]->is_boss) {
            if (primero == nullptr) primero = arbol.indice[i];
            cantidadJefes++;
        }
    }

    bool normalizado = false;
    if (cantidadJefes > 1) {
        asignar_jefe(arbol, nullptr, primero);
        normalizado = true;
    }

    Persona* jefe = buscar_jefe(arbol);
    if (jefe == nullptr || !es_elegible(jefe)) {
        return revisar_sucesion(arbol);
    }

    if (normalizado) {
        return "Se normalizó el estado inicial: un solo jefe.";
    }

    return "";
}

// ---------------------------------------------------------------------------
// Vistas
// ---------------------------------------------------------------------------
static std::string etiquetaEstado(const Persona* nodo) {
    std::string estado;

    if (nodo->is_boss) {
        estado += nodo->in_jail ? " [JEFE INTERINO]" : " [JEFE]";
    }
    if (nodo->is_dead) estado += " [MUERTO]";
    if (nodo->in_jail) estado += " [CÁRCEL]";
    if (nodo->age > EDAD_MAXIMA_JEFE) estado += " [>70]";
    if (nodo->was_boss && !nodo->is_boss) estado += " [EX-JEFE]";

    return estado;
}

static std::string describirNodo(const Persona* nodo) {
    return "[id " + std::to_string(nodo->id) + "] " + nodo->name + " " +
           nodo->last_name + " (" + std::to_string(nodo->age) + " años, " +
           std::string(1, nodo->gender) + ")" + etiquetaEstado(nodo);
}

static void mostrarRama(const Persona* nodo, const std::string& prefijo,
                        bool esUltimo, const std::string& etiqueta) {
    if (nodo == nullptr) return;

    std::cout << prefijo << (esUltimo ? "└── " : "├── ")
              << etiqueta << " " << describirNodo(nodo) << "\n";

    std::string siguiente = prefijo + (esUltimo ? "    " : "│   ");
    bool tieneIzq = nodo->izq != nullptr;
    bool tieneDer = nodo->der != nullptr;

    if (tieneIzq) mostrarRama(nodo->izq, siguiente, !tieneDer, "1er");
    if (tieneDer) mostrarRama(nodo->der, siguiente, true, "2do");
}

static void mostrarJefeActual(const ArbolFamilia& arbol) {
    Persona* jefe = buscar_jefe(arbol);

    std::cout << "\n";
    imprimirCentrado("JEFE ACTUAL", anchoConsola());
    std::cout << "\n";

    if (jefe == nullptr) {
        imprimirCentrado("La familia no tiene jefe actualmente.",
                         anchoConsola());
        return;
    }

    if (es_jefe_interino(jefe)) {
        imprimirCentrado("[Interino en la cárcel]", anchoConsola());
    }
    mostrarPersona(jefe);
}

static void mostrarArbolCompleto(const ArbolFamilia& arbol) {
    std::cout << "\n";
    imprimirCentrado("ÁRBOL FAMILIAR", anchoConsola());
    std::cout << "\n";

    if (arbol.raiz == nullptr) {
        imprimirCentrado("(árbol vacío)", anchoConsola());
        return;
    }

    const std::string base = "  ";
    std::cout << base << "[Raíz] " << describirNodo(arbol.raiz) << "\n";

    bool tieneIzq = arbol.raiz->izq != nullptr;
    bool tieneDer = arbol.raiz->der != nullptr;
    if (tieneIzq) mostrarRama(arbol.raiz->izq, base, !tieneDer, "1er");
    if (tieneDer) mostrarRama(arbol.raiz->der, base, true, "2do");

    int vivos = 0;
    int libres = 0;
    int presos = 0;
    int muertos = 0;

    for (int i = 0; i < arbol.cantidad; i++) {
        Persona* persona = arbol.indice[i];
        if (persona == nullptr) continue;

        if (persona->is_dead) {
            muertos++;
        } else {
            vivos++;
            if (persona->in_jail) presos++;
            else libres++;
        }
    }

    std::cout << "\n  Resumen: " << vivos
              << (vivos == 1 ? " vivo (" : " vivos (")
              << libres << (libres == 1 ? " libre, " : " libres, ")
              << presos << (presos == 1 ? " preso), " : " presos), ")
              << muertos << (muertos == 1 ? " muerto.\n" : " muertos.\n");
    std::cout << "  Leyenda: [JEFE] [JEFE INTERINO] [MUERTO] [CÁRCEL] "
              << "[>70] [EX-JEFE]\n";

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
    if (!leerEnteroEnRango("\n  Ingrese el ID de la persona: ",
                           1, 1000000000, id)) {
        return;
    }

    Persona* persona = buscarPorId(arbol, id);
    if (persona == nullptr) {
        imprimirCentrado("No se encontró ninguna persona con ID " +
                             std::to_string(id) + ".",
                         anchoConsola());
        return;
    }

    std::cout << "\n";
    imprimirCentrado("DATOS DEL INTEGRANTE", anchoConsola());
    std::cout << "\n";
    mostrarPersona(persona);
}

// ---------------------------------------------------------------------------
// Edicion de integrantes
// ---------------------------------------------------------------------------
static void editarIntegrante(ArbolFamilia& arbol) {
    int id = 0;
    if (!leerEnteroEnRango("\n  Ingrese el ID de la persona a editar: ",
                           1, 1000000000, id)) {
        return;
    }

    Persona* persona = buscarPorId(arbol, id);
    if (persona == nullptr) {
        imprimirCentrado("No se encontró ninguna persona con ID " +
                             std::to_string(id) + ".",
                         anchoConsola());
        return;
    }

    while (true) {
        limpiarPantalla();

        std::cout << "\n";
        imprimirCentrado("EDITAR INTEGRANTE", anchoConsola());
        std::cout << "\n";
        mostrarPersona(persona);
        std::cout << "\n  Campos editables (id e id_boss son fijos):\n"
                  << "    1. Nombre\n"
                  << "    2. Apellido\n"
                  << "    3. Género (H/M)\n"
                  << "    4. Edad (0-120)\n"
                  << "    5. Estado vital (is_dead: 0=vivo, 1=muerto)\n"
                  << "    6. Cárcel (in_jail: 0=libre, 1=preso)\n"
                  << "    7. Histórico jefe (was_boss: 0/1)\n"
                  << "    8. Jefe actual (is_boss: 0/1)\n"
                  << "    0. Volver al menú principal\n";

        int opcion = -1;
        if (!leerEnteroEnRango("  Campo a editar: ", 0, 8, opcion)) return;
        if (opcion == 0) {
            limpiarPantalla();
            return;
        }

        bool disparaRevision = false;

        switch (opcion) {
            case 1:
                if (!leerTextoNoVacio("  Nuevo nombre: ", persona->name)) {
                    return;
                }
                std::cout << "  Nombre actualizado.\n";
                break;

            case 2:
                if (!leerTextoNoVacio("  Nuevo apellido: ",
                                      persona->last_name)) {
                    return;
                }
                std::cout << "  Apellido actualizado.\n";
                break;

            case 3:
                if (!leerGenero("  Nuevo género (H/M): ", persona->gender)) {
                    return;
                }
                std::cout << "  Género actualizado.\n";
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
                if (!leerBooleano("  ¿Está muerto? (0=no, 1=sí): ",
                                  persona->is_dead)) {
                    return;
                }
                std::cout << "  Estado vital actualizado.\n";
                disparaRevision = true;
                break;

            case 6:
                if (!leerBooleano("  ¿Está en la cárcel? (0=no, 1=sí): ",
                                  persona->in_jail)) {
                    return;
                }
                std::cout << "  Estado de cárcel actualizado.\n";
                disparaRevision = true;
                break;

            case 7:
                if (!leerBooleano("  ¿Fue jefe anteriormente? (0=no, 1=sí): ",
                                  persona->was_boss)) {
                    return;
                }
                std::cout << "  Histórico was_boss actualizado.\n";
                break;

            case 8: {
                bool nuevoJefe = false;
                if (!leerBooleano("  Asignar como jefe actual? (0=no, 1=sí): ",
                                  nuevoJefe)) {
                    return;
                }

                if (nuevoJefe) {
                    asignar_jefe(arbol, buscar_jefe(arbol), persona);
                    std::cout << "  is_boss normalizado en [id "
                              << persona->id << "].\n";
                    disparaRevision = true;
                } else if (persona->is_boss) {
                    persona->is_boss = false;
                    persona->was_boss = true;
                    std::cout << "  Se retiró el cargo a [id " << persona->id
                              << "]. La familia queda sin jefe; use la opción 6 "
                              << "para recalcular.\n";
                    disparaRevision = false;
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

        pausar();
    }
}

// ---------------------------------------------------------------------------
// Bucle principal
// ---------------------------------------------------------------------------
void ejecutarMenu(ArbolFamilia& arbol) {
    std::string inicial = normalizarEstadoInicial(arbol);
    if (!inicial.empty()) {
        limpiarPantalla();
        std::cout << "\n";
        imprimirCentrado(inicial, anchoConsola());
        pausar();
    }

    while (true) {
        limpiarPantalla();
        dibujarMenu(arbol);

        int opcion = -1;
        std::string prompt = centrar("Seleccione una opción: ", anchoConsola());
        if (!leerEnteroEnRango(prompt, 0, 6, opcion)) {
            limpiarPantalla();
            std::cout << "\n";
            imprimirCentrado("Fin de entrada. Saliendo...", anchoConsola());
            return;
        }

        if (opcion == 0) {
            limpiarPantalla();
            std::cout << "\n";
            imprimirCentrado("Hasta luego.", anchoConsola());
            return;
        }

        switch (opcion) {
            case 1:
                mostrarJefeActual(arbol);
                pausar();
                break;

            case 2:
                mostrarArbolCompleto(arbol);
                pausar();
                break;

            case 3:
                std::cout << "\n";
                imprimirCentrado("LÍNEA DE SUCESIÓN", anchoConsola());
                std::cout << "\n";
                mostrar_linea_sucesion(arbol);
                pausar();
                break;

            case 4:
                buscarIntegrante(arbol);
                pausar();
                break;

            case 5:
                editarIntegrante(arbol);
                break;

            case 6:
                std::cout << "\n";
                imprimirCentrado("REVISIÓN DE SUCESIÓN", anchoConsola());
                std::cout << "\n  " << revisar_sucesion(arbol) << "\n";
                pausar();
                break;

            default:
                break;
        }
    }
}
