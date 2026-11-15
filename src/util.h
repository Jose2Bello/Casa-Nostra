#ifndef UTIL_H
#define UTIL_H

#include <string>

// Utilidades de texto y conversion usadas por el lector de CSV.
// No se usan contenedores de la STL.

const int MAX_CAMPOS_CSV = 16;

// Quita espacios y tabulaciones al inicio y al final.
std::string trim(const std::string& texto);

// Convierte a minusculas.
std::string aMinusculas(const std::string& texto);

// Parte una linea por comas. Guarda cada campo (sin espacios sobrantes) en
// `campos` y devuelve la cantidad de campos encontrados.
// Si hay mas de `maxCampos`, devuelve -1.
int dividirCSV(const std::string& linea, std::string campos[], int maxCampos);

// Conversiones con validacion. Devuelven false si el texto no es valido.
bool convertirEntero(const std::string& texto, int& valor);
bool convertirBooleano(const std::string& texto, bool& valor);   // "0" o "1"
bool convertirGenero(const std::string& texto, char& valor);     // 'H' o 'M'

#endif
