# Casa Nostra — sucesión de la familia

Prototipo en C++ para administrar la sucesión del jefe de una familia a partir de
un archivo CSV. Cada integrante es un nodo de un **árbol binario**: el hijo
izquierdo es el primer sucesor y el hijo derecho el segundo.

> **Estado:** proyecto completo y verificado. Infraestructura, motor de sucesión
> (`sucesion.h/.cpp`), menú interactivo (`menu.cpp`) y datos de prueba
> (`bin/datos.csv`) integrados. Las ediciones se mantienen en memoria durante la
> ejecución; el CSV no se reescribe.

---

## Estructura del proyecto

```
Casa-Nostra/
├── README.md
├── .gitignore
├── Makefile
├── compilar.bat
├── ejecutar.bat
├── bin/
│   ├── .gitkeep
│   ├── datos.csv          <- datos de prueba finales (10 personas)
│   └── mafia_sucesion.exe <- se genera al compilar
└── src/
    ├── main.cpp
    ├── arbol.h / arbol.cpp
    ├── csv.h / csv.cpp
    ├── menu.h / menu.cpp
    ├── persona.h / persona.cpp
    ├── sucesion.h / sucesion.cpp
    └── util.h / util.cpp
```

Todo el código va en `src/`; el ejecutable y los datos van en `bin/`.

---

## Requisitos

- `g++` con soporte de C++17.
- Windows: `compilar.bat` o `mingw32-make`.
- Linux/macOS: `make`.
- Solo biblioteca estándar: `<iostream>`, `<fstream>`, `<string>` y `<cctype>`.
  Sin librerías externas ni contenedores de la STL.

---

## Compilar y ejecutar

### Windows

Doble clic en:

- `compilar.bat`: compila y deja la ventana abierta.
- `ejecutar.bat`: compila y abre el menú.

Desde una terminal:

```bat
compilar.bat nopause
bin\mafia_sucesion.exe
```

> `nopause` evita la pausa final cuando se ejecuta desde una terminal.
> No hace falta Windows Terminal: los scripts usan la consola clásica.

### Con Make

```bash
make
./bin/mafia_sucesion.exe
```

### Compilación directa

```bash
g++ -std=c++17 -Wall -Wextra -O2 -static src/main.cpp src/persona.cpp src/util.cpp src/csv.cpp src/arbol.cpp src/sucesion.cpp src/menu.cpp -o bin/mafia_sucesion.exe
```

> En Windows se usa `-static` para no depender de las DLL de MinGW al ejecutar.

> `main.cpp` carga y valida `bin/datos.csv`, construye el árbol, muestra los
> recorridos iniciales y lanza el menú interactivo (`ejecutarMenu`). Admite una
> ruta alternativa: `bin\mafia_sucesion.exe otra_ruta.csv`.

---

## Formato del CSV

Archivo `bin/datos.csv`, con encabezado y estas columnas:

```csv
id,name,last_name,gender,age,id_boss,is_dead,in_jail,was_boss,is_boss
1,Vito,Corleone,H,65,0,0,0,0,1
2,Sonny,Corleone,H,40,1,0,0,0,0
3,Fredo,Corleone,H,38,1,0,0,0,0
```

| Columna | Tipo | Regla |
|---|---|---|
| `id` | entero | único y positivo |
| `name`, `last_name` | texto | sin comas internas |
| `gender` | carácter | `H` o `M` |
| `age` | entero | 0–120 |
| `id_boss` | entero | id del jefe; `0` para el fundador (raíz) |
| `is_dead`, `in_jail`, `was_boss`, `is_boss` | 0/1 | 1 = sí, 0 = no |

Reglas de construcción:

- `izq` = primer sucesor según el orden del CSV; `der` = segundo.
- Máximo **2 hijos** por jefe.
- Un solo nodo raíz (`id_boss = 0`) y un solo `is_boss = 1`.
- Sin ciclos en `id_boss`.

---

## Modelo de datos

```cpp
struct Persona {
    int id;
    std::string name, last_name;
    char gender;               // 'H' o 'M'
    int age, id_boss;
    bool is_dead, in_jail, was_boss, is_boss;
    Persona *izq, *der, *padre;
};
```

Consultas básicas disponibles en `persona.h`:

```cpp
bool estaVivo(const Persona*);
bool estaLibre(const Persona*);   // vivo y fuera de la cárcel
```

La elegibilidad para ser jefe (añadir `age <= 70`) corresponde al módulo de
sucesión.

---

## Utilidades de texto (`util.h`)

```cpp
std::string trim(const std::string&);
std::string aMinusculas(const std::string&);
int  dividirCSV(const std::string& linea, std::string campos[], int maxCampos);
bool convertirEntero(const std::string&, int&);
bool convertirBooleano(const std::string&, bool&);
bool convertirGenero(const std::string&, char&);
```

No se usan contenedores de la STL.

---

## Lector de CSV (`csv.h`)

```cpp
struct ListaPersonas {
    Persona** datos;
    int cantidad;
    int capacidad;
};

void inicializarLista(ListaPersonas&);
void liberarLista(ListaPersonas&);
bool cargarCSV(const char* ruta, ListaPersonas& lista, std::string& error);
```

`cargarCSV` valida:

- encabezado y cantidad de columnas;
- tipos y rangos (`age` 0–120, `gender` H/M, booleanos 0/1);
- IDs únicos y positivos;
- exactamente un `is_boss = 1` y una raíz (`id_boss = 0`);
- jefe existente, máximo 2 hijos por jefe y ausencia de ciclos.

Los errores se informan con el número de línea. `ListaPersonas` es la dueña de
los objetos `Persona` y se libera con `liberarLista`.

---

## Árbol (`arbol.h`)

```cpp
struct ArbolFamilia {
    Persona* raiz;
    Persona** indice;   // punteros ordenados por id
    int cantidad;
};

void inicializarArbol(ArbolFamilia&);
void liberarArbol(ArbolFamilia&);
bool construirArbol(ListaPersonas&, ArbolFamilia&, std::string& error);
Persona* buscarPorId(const ArbolFamilia&, int id);

void mostrarPreOrden(const ArbolFamilia&);
void mostrarInOrden(const ArbolFamilia&);
void mostrarPostOrden(const ArbolFamilia&);
void mostrarPorNiveles(const ArbolFamilia&);
```

`construirArbol` enlaza `izq` y `der` en el orden del CSV (primer sucesor a la
izquierda, segundo a la derecha) y rellena el puntero `padre`. El índice se
ordena por id y `buscarPorId` usa búsqueda binaria.

**Propiedad de memoria:** `ListaPersonas` es dueña de los objetos `Persona`;
`ArbolFamilia` solo guarda punteros. `liberarArbol` libera únicamente el índice.
Orden de liberación: primero `liberarArbol` y después `liberarLista`.

---

## Motor de sucesión (`sucesion.h`) y menú (`menu.h`)

```cpp
const int EDAD_MAXIMA_JEFE = 70;

bool es_elegible(const Persona*);
bool es_elegible_interino(const Persona*);
bool es_jefe_interino(const Persona*);
Persona* buscar_jefe(const ArbolFamilia&);
Persona** construir_linea_sucesion(const ArbolFamilia&, int& cantidad);
void mostrar_linea_sucesion(const ArbolFamilia&);
Persona* buscar_sucesor(const ArbolFamilia&);
void asignar_jefe(ArbolFamilia&, Persona* saliente, Persona* entrante);
std::string revisar_sucesion(ArbolFamilia&);
```

El menú interactivo (`ejecutarMenu`) ofrece las opciones:

1. Ver jefe actual (normal, interino en cárcel o sin jefe).
2. Ver árbol familiar (estructura jerárquica y los 4 recorridos).
3. Ver línea de sucesión (elegibles libres o candidatos interinos).
4. Buscar integrante por ID.
5. Editar integrante: nombre, apellido, género, edad, `is_dead`, `in_jail`,
   `was_boss` e `is_boss` (nunca `id` ni `id_boss`).
6. Forzar revisión de sucesión.
0. Salir.

Al editar `age`, `is_dead` o `in_jail` se dispara la revisión automática. Si se
pone `is_boss = 0` al jefe, la familia queda sin jefe hasta usar la opción 6.

El menú se dibuja centrado, limpia la pantalla después de cada acción y usa UTF-8
para mostrar `ñ`, `á`, `é`, etc. En Windows se configura la consola a UTF-8 al
arrancar.

---

## División del trabajo

### Infraestructura (José Bello)

- Estructura `bin/` + `src/`, build y `.gitignore`.
- Modelo `Persona` y utilidades de texto.
- Lector y validador de CSV, con `bin/datos.csv` base.
- Árbol binario, índice por id y recorridos.
- `main.cpp` conectado a `ejecutarMenu`.

### Menú y lógica (Sebastián Alsleben)

- Motor de sucesión (`src/sucesion.h`, `src/sucesion.cpp`) con reglas R1–R7.
- Menú interactivo en bucle hasta `0` (`src/menu.cpp`).
- Línea de sucesión y vista de árbol/jefe actual.
- Edición de personas con validación (todo menos `id` e `id_boss`), normalización de `is_boss` y disparadores automáticos.
- Datos de prueba finales (10 personas en `bin/datos.csv`) y documentación de reglas/escenarios.

### Ajustes finales de integración

- Búsqueda por ámbitos R1–R5, fase de presos por cercanía R6 y corrección del caso `is_boss = 0`.
- Limpieza del arranque y actualización de reglas/escenarios.

---

## Reglas de sucesión

El motor aplica esta formalización de las reglas del enunciado, con
`EDAD_MAXIMA_JEFE = 70`.

### Definiciones

- **Elegible libre:** vivo, no preso, edad ≤ 70.
- **Elegible interino:** vivo, preso, edad ≤ 70.
- **Primer sucesor:** recorrido pre-orden, izquierda primero.
- **Compañero sucesor:** el otro hijo del padre del jefe.
- **Jefe anterior:** el padre; **jefe del jefe:** el abuelo.

Disparadores: muerte, cárcel o edad > 70 del jefe actual. Si el jefe sigue vivo,
libre y con edad ≤ 70, conserva el cargo.

### Orden de búsqueda

| Regla | Condición | Acción |
|---|---|---|
| **R1** | El jefe tiene sucesores | Primer elegible libre en el subárbol de sus hijos (izquierda, luego derecha). |
| **R2** | El jefe no tiene sucesores | Primer elegible libre en el árbol del **hermano**. |
| **R3** | El hermano no tiene sucesores libres y es elegible | El hermano asume. |
| **R4** | No hay hermano o no se encontró candidato | Primer elegible libre en el árbol del **tío**; si el tío no tiene sucesores libres y es elegible, él asume. |
| **R5** | Todavía no hay candidato | Subir por los ancestros: el más cercano con **dos hijos elegibles libres**; elegir el primer elegible de su subárbol. |
| **R6** | No hay ningún libre | Buscar el **preso vivo ≤ 70 más cercano** al jefe actual (BFS por padre/hijos). |
| **R7** | El jefe supera los 70 o va a prisión | Se aplica R1 desde su árbol. Si no hay candidatos, se limpia `is_boss` y la familia queda sin jefe. |

Si ningún ámbito produce candidato, se usa como respaldo el primer elegible libre
del árbol completo. El jefe interino preso se mantiene mientras no aparezca un
libre elegible.

### Supuestos de diseño

- `bin/datos.csv` tiene 10 integrantes (`id` 1–10), en pre-orden
  `1, 2, 4, 8, 9, 5, 3, 6, 10, 7`, con libres, presos (`3`, `10`), un muerto
  (`9`) y un mayor de 70 (`7`, 72 años).
- Para hermano y tío se buscan primero sus descendientes; si no hay, el propio
  hermano/tío puede asumir (R3).
- `was_boss` se pone en `1` al asumir el cargo, incluido el jefe actual.
- Al editar `is_boss = 1` sobre otro integrante se normaliza para dejar un solo
  jefe y se valida su elegibilidad.
- Al editar `is_boss = 0` sobre el jefe, no se reasigna automáticamente: la
  familia queda sin jefe hasta usar la opción 6 u otro disparador.
- Al arrancar, si el jefe cargado no es elegible, se ejecuta una revisión
  inicial.
- Las ediciones se mantienen en memoria; el CSV no se reescribe.

### Escenarios de prueba verificados

| Escenario | Acción | Resultado verificado |
|---|---|---|
| Línea inicial | Opción 3 con Vito de jefe | `2 Sonny`, `4 Michael`, `8 Vincent`, `5 Connie`, `6 Tom`; omite muerto, presos y `> 70`. |
| Muerte del jefe | Editar `id=1` con `is_dead=1` | Asume Sonny (`id=2`). |
| Edad > 70 | Editar `id=1` con `age=71` | Asume Sonny (`id=2`). |
| Cárcel con libres | Editar `id=1` con `in_jail=1` | Asume Sonny (`id=2`). |
| Jefe hijo derecho sin sucesores | Jefe `7` muerto; hermano `6` libre | Asume Tom (`id=6`) por R2. |
| Ancestro con dos libres | Jefe `5` muerto; ancestro `1` con hijos `2` y `3` libres | Asume Sonny (`id=2`) por R5. |
| Dos presos a distinta distancia | Jefe `1` muerto; `3` preso a distancia 1 y `4` a distancia 2 | Asume Fredo (`id=3`) por R6. |
| Sin libres ni presos | Jefe muerto y sin candidatos | `is_boss=0` para todos; mensaje sin sucesor. |
| Quitar `is_boss` al jefe | Editar `is_boss=0` | Queda sin jefe; no se reasigna hasta la opción 6. |

---

## Convenciones

- Nombres de archivos y carpetas en `snake_case`, sin espacios ni símbolos.
- Commits en **Conventional Commits** (`feat:`, `fix:`, `docs:`, `test:`, `chore:`).
- Compilar sin advertencias con `-Wall -Wextra`.
- Sin contenedores de la STL (arreglos dinamicos y estructuras propias).

---

## Integrantes

- Jose Bello (@Jose2Bello)
- Sebastián Asleben
