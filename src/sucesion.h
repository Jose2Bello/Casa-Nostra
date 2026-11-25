#ifndef SUCESION_H
#define SUCESION_H

#include <string>

#include "arbol.h"
#include "persona.h"

// Edad maxima para seguir siendo jefe, tanto libre como interino.
const int EDAD_MAXIMA_JEFE = 70;

// R2: elegibilidad como jefe libre (vivo, no preso y sin superar la edad).
bool es_elegible(const Persona* persona);

// R4: elegibilidad como jefe interino (vivo, preso y sin superar la edad).
bool es_elegible_interino(const Persona* persona);

// El interino preso se infiere de los flags: manda y esta en la carcel.
bool es_jefe_interino(const Persona* persona);

// Devuelve el jefe actual (is_boss) o nullptr si no hay ninguno.
Persona* buscar_jefe(const ArbolFamilia& arbol);

// R1: linea de sucesion (pre-orden ciclico desde el jefe) filtrada por R2.
// Devuelve un arreglo nuevo[] con los elegibles y su cantidad por referencia.
// El llamador es duenio del arreglo y debe liberarlo con delete[].
Persona** construir_linea_sucesion(const ArbolFamilia& arbol, int& cantidad);

// Muestra la linea numerada. Si no hay libres, muestra los interinos presos.
void mostrar_linea_sucesion(const ArbolFamilia& arbol);

// Calcula el proximo jefe segun R3/R4/R5. Devuelve nullptr si el jefe actual
// debe seguir o si no hay ningun candidato (R7).
Persona* buscar_sucesor(const ArbolFamilia& arbol);

// Pasa el cargo de `saliente` a `entrante`, actualizando is_boss y was_boss.
// Deja exactamente un is_boss cuando `entrante` no es nullptr.
void asignar_jefe(ArbolFamilia& arbol, Persona* saliente, Persona* entrante);

// Aplica R3/R4/R5/R6/R7 y devuelve un mensaje legible con el resultado.
std::string revisar_sucesion(ArbolFamilia& arbol);

#endif
