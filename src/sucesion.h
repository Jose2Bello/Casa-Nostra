#ifndef SUCESION_H
#define SUCESION_H

#include <string>

#include "arbol.h"
#include "persona.h"

// Edad maxima para seguir siendo jefe, libre o interino.
const int EDAD_MAXIMA_JEFE = 70;

// Elegibilidad libre: vivo, no preso y edad <= 70.
bool es_elegible(const Persona* persona);

// Elegibilidad interina: vivo, preso y edad <= 70.
bool es_elegible_interino(const Persona* persona);

// Un jefe interino manda y esta en la carcel.
bool es_jefe_interino(const Persona* persona);

// Devuelve el jefe actual (is_boss) o nullptr si no hay.
Persona* buscar_jefe(const ArbolFamilia& arbol);

// Linea de sucesion para mostrar: elegibles libres en pre-orden desde el jefe.
// Devuelve un arreglo nuevo[] que el llamador libera con delete[].
Persona** construir_linea_sucesion(const ArbolFamilia& arbol, int& cantidad);

// Muestra la linea de sucesion. Si no hay libres, muestra los interinos.
void mostrar_linea_sucesion(const ArbolFamilia& arbol);

// Aplica R1-R6 y devuelve el proximo jefe.
// nullptr si el jefe actual sigue elegible o si no hay candidatos.
Persona* buscar_sucesor(const ArbolFamilia& arbol);

// Traspasa el cargo de `saliente` a `entrante`.
// El entrante queda con is_boss = true y was_boss = true.
void asignar_jefe(ArbolFamilia& arbol, Persona* saliente, Persona* entrante);

// Aplica la revision completa y devuelve un mensaje legible.
std::string revisar_sucesion(ArbolFamilia& arbol);

#endif
