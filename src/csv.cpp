#include "csv.h"

#include <cstdarg>
#include <cstdio>
#include <fstream>
#include <string>

#include "util.h"

static const int COLUMNAS = 10;

static const char* ENCABEZADO[COLUMNAS] = {
    "id", "name", "last_name", "gender", "age",
    "id_boss", "is_dead", "in_jail", "was_boss", "is_boss"
};

// ---------------------------------------------------------------------------
// Mensajes de error
// ---------------------------------------------------------------------------
static void ponerError(char* destino, int tam, const char* formato, ...) {
    if (destino == nullptr || tam <= 0) return;

    va_list argumentos;
    va_start(argumentos, formato);
    std::vsnprintf(destino, tam, formato, argumentos);
    va_end(argumentos);
}

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
static bool validarEncabezado(const std::string& linea, char* error, int tamError) {
    std::string campos[MAX_CAMPOS_CSV];
    int cantidad = dividirCSV(linea, campos, MAX_CAMPOS_CSV);

    if (cantidad != COLUMNAS) {
        ponerError(error, tamError,
                   "El encabezado debe tener %d columnas y tiene %d",
                   COLUMNAS, cantidad);
        return false;
    }

    for (int i = 0; i < COLUMNAS; i++) {
        if (aMinusculas(campos[i]) != ENCABEZADO[i]) {
            ponerError(error, tamError,
                       "Encabezado invalido: se esperaba '%s' en la columna %d",
                       ENCABEZADO[i], i + 1);
            return false;
        }
    }

    return true;
}

// ---------------------------------------------------------------------------
// Una fila del CSV
// ---------------------------------------------------------------------------
static bool parsearPersona(const std::string& linea, int numeroLinea,
                           Persona*& salida, char* error, int tamError) {
    std::string campos[MAX_CAMPOS_CSV];
    int cantidad = dividirCSV(linea, campos, MAX_CAMPOS_CSV);

    if (cantidad != COLUMNAS) {
        ponerError(error, tamError,
                   "Linea %d: se esperaban %d columnas y se encontraron %d",
                   numeroLinea, COLUMNAS, cantidad);
        return false;
    }

    Persona* persona = new Persona();
    bool valida = true;

    if (!convertirEntero(campos[0], persona->id) || persona->id <= 0) {
        ponerError(error, tamError,
                   "Linea %d: el id debe ser un entero positivo", numeroLinea);
        valida = false;
    }

    if (valida) {
        persona->name = trim(campos[1]);
        persona->last_name = trim(campos[2]);
        if (persona->name.empty() || persona->last_name.empty()) {
            ponerError(error, tamError,
                       "Linea %d: name y last_name no pueden estar vacios",
                       numeroLinea);
            valida = false;
        }
    }

    if (valida && !convertirGenero(campos[3], persona->gender)) {
        ponerError(error, tamError,
                   "Linea %d: gender debe ser H o M", numeroLinea);
        valida = false;
    }

    if (valida &&
        (!convertirEntero(campos[4], persona->age) ||
         persona->age < 0 || persona->age > 120)) {
        ponerError(error, tamError,
                   "Linea %d: age debe estar entre 0 y 120", numeroLinea);
        valida = false;
    }

    if (valida &&
        (!convertirEntero(campos[5], persona->id_boss) ||
         persona->id_boss < 0)) {
        ponerError(error, tamError,
                   "Linea %d: id_boss debe ser un entero no negativo",
                   numeroLinea);
        valida = false;
    }

    if (valida && persona->id_boss == persona->id) {
        ponerError(error, tamError,
                   "Linea %d: una persona no puede ser su propio jefe",
                   numeroLinea);
        valida = false;
    }

    if (valida && !convertirBooleano(campos[6], persona->is_dead)) {
        ponerError(error, tamError,
                   "Linea %d: is_dead debe ser 0 o 1", numeroLinea);
        valida = false;
    }

    if (valida && !convertirBooleano(campos[7], persona->in_jail)) {
        ponerError(error, tamError,
                   "Linea %d: in_jail debe ser 0 o 1", numeroLinea);
        valida = false;
    }

    if (valida && !convertirBooleano(campos[8], persona->was_boss)) {
        ponerError(error, tamError,
                   "Linea %d: was_boss debe ser 0 o 1", numeroLinea);
        valida = false;
    }

    if (valida && !convertirBooleano(campos[9], persona->is_boss)) {
        ponerError(error, tamError,
                   "Linea %d: is_boss debe ser 0 o 1", numeroLinea);
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
static bool validarRelaciones(const ListaPersonas& lista, char* error, int tamError) {
    int* hijos = new int[lista.cantidad];
    for (int i = 0; i < lista.cantidad; i++) hijos[i] = 0;

    for (int i = 0; i < lista.cantidad; i++) {
        Persona* persona = lista.datos[i];
        if (persona->id_boss == 0) continue;   // es la raiz

        int indiceJefe = buscarIndicePorId(lista, persona->id_boss);
        if (indiceJefe == -1) {
            ponerError(error, tamError,
                       "La persona %d tiene un jefe inexistente (id_boss=%d)",
                       persona->id, persona->id_boss);
            delete[] hijos;
            return false;
        }

        hijos[indiceJefe]++;
        if (hijos[indiceJefe] > 2) {
            ponerError(error, tamError,
                       "El jefe %d tiene mas de 2 sucesores",
                       lista.datos[indiceJefe]->id);
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
                ponerError(error, tamError,
                           "Ciclo detectado en id_boss (persona %d)",
                           lista.datos[i]->id);
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
bool cargarCSV(const char* ruta, ListaPersonas& lista, char* error, int tamError) {
    if (error != nullptr && tamError > 0) error[0] = '\0';

    if (lista.datos == nullptr) inicializarLista(lista);

    std::ifstream archivo(ruta);
    if (!archivo.is_open()) {
        ponerError(error, tamError, "No se pudo abrir el archivo: %s", ruta);
        return false;
    }

    std::string linea;
    if (!std::getline(archivo, linea)) {
        ponerError(error, tamError, "El archivo CSV esta vacio");
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
        ponerError(error, tamError, "El archivo CSV esta vacio");
        archivo.close();
        return false;
    }

    if (!validarEncabezado(linea, error, tamError)) {
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
        if (!parsearPersona(linea, numeroLinea, persona, error, tamError)) {
            liberarLista(lista);
            archivo.close();
            return false;
        }

        if (buscarIndicePorId(lista, persona->id) != -1) {
            ponerError(error, tamError,
                       "Linea %d: id duplicado (%d)", numeroLinea, persona->id);
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
        ponerError(error, tamError, "El CSV no contiene personas");
        liberarLista(lista);
        return false;
    }

    if (jefes != 1) {
        ponerError(error, tamError,
                   "Debe haber exactamente un is_boss=1 (se encontraron %d)",
                   jefes);
        liberarLista(lista);
        return false;
    }

    if (raices != 1) {
        ponerError(error, tamError,
                   "Debe haber exactamente una raiz con id_boss=0 (se encontraron %d)",
                   raices);
        liberarLista(lista);
        return false;
    }

    if (!validarRelaciones(lista, error, tamError)) {
        liberarLista(lista);
        return false;
    }

    return true;
}
