#ifndef PERSONA_H
#define PERSONA_H

#include <string>

// Un miembro de la familia. Cada persona es un nodo del arbol binario:
//   izq = primer sucesor, der = segundo sucesor, padre = jefe anterior.
struct Persona {
    int id;
    std::string name;
    std::string last_name;
    char gender;          // 'H' o 'M'
    int age;
    int id_boss;          // 0 = jefe fundador (raiz)
    bool is_dead;
    bool in_jail;
    bool was_boss;
    bool is_boss;

    Persona* izq;
    Persona* der;
    Persona* padre;

    Persona()
        : id(0), gender('H'), age(0), id_boss(0),
          is_dead(false), in_jail(false), was_boss(false), is_boss(false),
          izq(nullptr), der(nullptr), padre(nullptr) {}
};

// Consultas basicas (no incluyen la regla de edad: eso es parte de sucesion).
bool estaVivo(const Persona* persona);
bool estaLibre(const Persona* persona);

// Muestra los datos de una persona en una linea.
void mostrarPersona(const Persona* persona);

#endif
