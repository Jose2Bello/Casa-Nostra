#include "csv.h"

#include <fstream>
#include <string>

#include "util.h"

static const int COLUMNAS = 10;

static const char* ENCABEZADO[COLUMNAS] = {
    "id", "name", "last_name", "gender", "age",
    "id_boss", "is_dead", "in_jail", "was_boss", "is_boss"
};

// ---------------------------------------------------------------------------
// Lista dinamica de personas
// ---------------------------------------------------------------------------
void inicializarLista(ListaPersonas& lista) {
    lista.datos = nullptr;
    lista.cantidad = 0;
    lista.capacidad = 0;
}

void liberarLista(ListaPersonas& lista) {
    if (lista.datos != nullptr) {
        for (int i = 0; i < lista.cantidad; i++) {
            delete lista.datos[i];
        }
        delete[] lista.datos;
    }
    lista.datos = nullptr;
    lista.cantidad = 0;
    lista.capacidad = 0;
}

static bool asegurarCapacidad(ListaPersonas& lista) {
    if (lista.cantidad < lista.capacidad) return true;

    int nuevaCapacidad = (lista.capacidad == 0) ? 8 : lista.capacidad * 2;
    Persona** nuevos = new Persona*[nuevaCapacidad];

    for (int i = 0; i < lista.cantidad; i++) {
        nuevos[i] = lista.datos[i];
    }

    delete[] lista.datos;
    lista.datos = nuevos;
    lista.capacidad = nuevaCapacidad;
    return true;
}

static int buscarIndicePorId(const ListaPersonas& lista, int id) {
    for (int i = 0; i < lista.cantidad; i++) {
        if (lista.datos[i]->id == id) return i;
    }
    return -1;
}

// ---------------------------------------------------------------------------
// Encabezado
// ---------------------------------------------------------------------------
static bool validarEncabezado(const std::string& linea, std::string& error) {
    std::string campos[MAX_CAMPOS_CSV];
    int cantidad = dividirCSV(linea, campos, MAX_CAMPOS_CSV);

    if (cantidad != COLUMNAS) {
        error = "El encabezado debe tener " + std::to_string(COLUMNAS) +
                " columnas y tiene " + std::to_string(cantidad);
        return false;
    }

    for (int i = 0; i < COLUMNAS; i++) {
        if (aMinusculas(campos[i]) != ENCABEZADO[i]) {
            error = "Encabezado invalido: se esperaba '" +
                    std::string(ENCABEZADO[i]) + "' en la columna " +
                    std::to_string(i + 1);
            return false;
        }
    }

    return true;
}

// ---------------------------------------------------------------------------
// Una fila del CSV
// ---------------------------------------------------------------------------
static bool parsearPersona(const std::string& linea, int numeroLinea,
                           Persona*& salida, std::string& error) {
    std::string campos[MAX_CAMPOS_CSV];
    int cantidad = dividirCSV(linea, campos, MAX_CAMPOS_CSV);

    if (cantidad != COLUMNAS) {
        error = "Linea " + std::to_string(numeroLinea) + ": se esperaban " +
                std::to_string(COLUMNAS) + " columnas y se encontraron " +
                std::to_string(cantidad);
        return false;
    }

    Persona* persona = new Persona();
    bool valida = true;

    if (!convertirEntero(campos[0], persona->id) || persona->id <= 0) {
        error = "Linea " + std::to_string(numeroLinea) +
                ": el id debe ser un entero positivo";
        valida = false;
    }

    if (valida) {
        persona->name = trim(campos[1]);
        persona->last_name = trim(campos[2]);
        if (persona->name.empty() || persona->last_name.empty()) {
            error = "Linea " + std::to_string(numeroLinea) +
                    ": name y last_name no pueden estar vacios";
            valida = false;
        }
    }

    if (valida && !convertirGenero(campos[3], persona->gender)) {
        error = "Linea " + std::to_string(numeroLinea) +
                ": gender debe ser H o M";
        valida = false;
    }

    if (valida &&
        (!convertirEntero(campos[4], persona->age) ||
         persona->age < 0 || persona->age > 120)) {
        error = "Linea " + std::to_string(numeroLinea) +
                ": age debe estar entre 0 y 120";
        valida = false;
    }

    if (valida &&
        (!convertirEntero(campos[5], persona->id_boss) ||
         persona->id_boss < 0)) {
        error = "Linea " + std::to_string(numeroLinea) +
                ": id_boss debe ser un entero no negativo";
        valida = false;
    }

    if (valida && persona->id_boss == persona->id) {
        error = "Linea " + std::to_string(numeroLinea) +
                ": una persona no puede ser su propio jefe";
        valida = false;
    }

    if (valida && !convertirBooleano(campos[6], persona->is_dead)) {
        error = "Linea " + std::to_string(numeroLinea) +
                ": is_dead debe ser 0 o 1";
        valida = false;
    }

    if (valida && !convertirBooleano(campos[7], persona->in_jail)) {
        error = "Linea " + std::to_string(numeroLinea) +
                ": in_jail debe ser 0 o 1";
        valida = false;
    }

    if (valida && !convertirBooleano(campos[8], persona->was_boss)) {
        error = "Linea " + std::to_string(numeroLinea) +
                ": was_boss debe ser 0 o 1";
        valida = false;
    }

    if (valida && !convertirBooleano(campos[9], persona->is_boss)) {
        error = "Linea " + std::to_string(numeroLinea) +
                ": is_boss debe ser 0 o 1";
        valida = false;
    }

    if (!valida) {
        delete persona;
        return false;
    }

    salida = persona;
    return true;
}

// ---------------------------------------------------------------------------
// Relaciones: jefe existente, maximo 2 hijos y sin ciclos
// ---------------------------------------------------------------------------
static bool validarRelaciones(const ListaPersonas& lista, std::string& error) {
    int* hijos = new int[lista.cantidad];
    for (int i = 0; i < lista.cantidad; i++) hijos[i] = 0;

    for (int i = 0; i < lista.cantidad; i++) {
        Persona* persona = lista.datos[i];
        if (persona->id_boss == 0) continue;   // es la raiz

        int indiceJefe = buscarIndicePorId(lista, persona->id_boss);
        if (indiceJefe == -1) {
            error = "La persona " + std::to_string(persona->id) +
                    " tiene un jefe inexistente (id_boss=" +
                    std::to_string(persona->id_boss) + ")";
            delete[] hijos;
            return false;
        }

        hijos[indiceJefe]++;
        if (hijos[indiceJefe] > 2) {
            error = "El jefe " + std::to_string(lista.datos[indiceJefe]->id) +
                    " tiene mas de 2 sucesores";
            delete[] hijos;
            return false;
        }
    }

    // Deteccion de ciclos: subir por los jefes con un limite de pasos.
    for (int i = 0; i < lista.cantidad; i++) {
        int pasos = 0;
        int idActual = lista.datos[i]->id_boss;

        while (idActual != 0) {
            int indice = buscarIndicePorId(lista, idActual);
            if (indice == -1) break;   // ya se reporto arriba

            pasos++;
            if (pasos > lista.cantidad) {
                error = "Ciclo detectado en id_boss (persona " +
                        std::to_string(lista.datos[i]->id) + ")";
                delete[] hijos;
                return false;
            }
            idActual = lista.datos[indice]->id_boss;
        }
    }

    delete[] hijos;
    return true;
}

// ---------------------------------------------------------------------------
// Carga principal
// ---------------------------------------------------------------------------
bool cargarCSV(const char* ruta, ListaPersonas& lista, std::string& error) {
    error.clear();

    if (lista.datos == nullptr) inicializarLista(lista);

    std::ifstream archivo(ruta);
    if (!archivo.is_open()) {
        error = "No se pudo abrir el archivo: " + std::string(ruta);
        return false;
    }

    std::string linea;
    if (!std::getline(archivo, linea)) {
        error = "El archivo CSV esta vacio";
        archivo.close();
        return false;
    }

    // Quitar el BOM de UTF-8 si existe.
    if (linea.size() >= 3 &&
        static_cast<unsigned char>(linea[0]) == 0xEF &&
        static_cast<unsigned char>(linea[1]) == 0xBB &&
        static_cast<unsigned char>(linea[2]) == 0xBF) {
        linea.erase(0, 3);
    }

    if (trim(linea).empty()) {
        error = "El archivo CSV esta vacio";
        archivo.close();
        return false;
    }

    if (!validarEncabezado(linea, error)) {
        archivo.close();
        return false;
    }

    int numeroLinea = 1;
    int raices = 0;
    int jefes = 0;

    while (std::getline(archivo, linea)) {
        numeroLinea++;

        if (trim(linea).empty()) continue;   // lineas en blanco

        Persona* persona = nullptr;
        if (!parsearPersona(linea, numeroLinea, persona, error)) {
            liberarLista(lista);
            archivo.close();
            return false;
        }

        if (buscarIndicePorId(lista, persona->id) != -1) {
            error = "Linea " + std::to_string(numeroLinea) +
                    ": id duplicado (" + std::to_string(persona->id) + ")";
            delete persona;
            liberarLista(lista);
            archivo.close();
            return false;
        }

        asegurarCapacidad(lista);
        lista.datos[lista.cantidad] = persona;
        lista.cantidad++;

        if (persona->id_boss == 0) raices++;
        if (persona->is_boss) jefes++;
    }

    archivo.close();

    if (lista.cantidad == 0) {
        error = "El CSV no contiene personas";
        liberarLista(lista);
        return false;
    }

    if (jefes != 1) {
        error = "Debe haber exactamente un is_boss=1 (se encontraron " +
                std::to_string(jefes) + ")";
        liberarLista(lista);
        return false;
    }

    if (raices != 1) {
        error = "Debe haber exactamente una raiz con id_boss=0 (se encontraron " +
                std::to_string(raices) + ")";
        liberarLista(lista);
        return false;
    }

    if (!validarRelaciones(lista, error)) {
        liberarLista(lista);
        return false;
    }

    return true;
}
