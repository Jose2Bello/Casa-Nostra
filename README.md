# Casa Nostra — sucesión de la familia

Prototipo en C++ para administrar la sucesión del jefe de una familia a partir de
un archivo CSV. Cada integrante es un nodo de un **árbol binario**: el hijo
izquierdo es el primer sucesor y el hijo derecho el segundo.

> **Estado:** proyecto completo. Infraestructura, motor de sucesión (`sucesion.h/.cpp`),
> menú interactivo (`menu.cpp`) y datos de prueba finales (`bin/datos.csv`) integrados.

---

## Estructura del proyecto

```
Casa-Nostra/
├── README.md
├── .gitignore
├── Makefile
├── compilar.bat
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

```bat
compilar.bat
bin\mafia_sucesion.exe
```

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

1. Ver jefe actual (normal, interino en cárcel o estado R7 sin jefe).
2. Ver árbol familiar (estructura jerárquica y los 4 recorridos).
3. Ver línea de sucesión (elegibles libres o candidatos interinos).
4. Buscar integrante por ID.
5. Editar integrante (todos los campos excepto `id` e `id_boss`, con validación y disparo automático de sucesión).
6. Forzar revisión de sucesión.
0. Salir.

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

---

## Reglas de sucesión

El motor (`src/sucesion.h` / `src/sucesion.cpp`) aplica las reglas **R1–R7** con `EDAD_MAXIMA_JEFE = 70`:

| Regla | Nombre | Definición implementada |
|---|---|---|
| **R1** | Orden de sucesión | Pre-orden cíclico desde el jefe actual (excluyendo al jefe actual): primero su subárbol, luego ramas hermanas y finalmente los ancestros al dar la vuelta. Si no hay jefe (`R7`), recorre en pre-orden desde la raíz. |
| **R2** | Elegibilidad | Libre elegible: `estaVivo && !in_jail && age <= 70`. Muertos (`is_dead=1`) y mayores de 70 años quedan fuera de toda sucesión. Los presos (`in_jail=1`) quedan fuera de la línea regular y solo cuentan para interinato. `was_boss` es histórico y **nunca** excluye. |
| **R3** | Sucesión normal | Si el jefe actual deja de ser elegible (muerte, cárcel o `age > 70`) y existe al menos un candidato libre elegible, asume el primero de la línea R1. |
| **R4** | Interinato | Si no hay ningún candidato libre elegible, asume el primer integrante en pre-orden cíclico que esté vivo, en la cárcel y con `age <= 70`. El interino preso se identifica con `is_boss && in_jail`. Si un jefe libre cae preso y no hay reemplazo libre, conserva el mando como interino desde la cárcel siempre que siga vivo y `<= 70`. |
| **R5** | Relevo | Cuando el jefe es un interino preso (`is_boss && in_jail`), en la próxima revisión entrega el cargo automáticamente al primer candidato libre elegible (por ejemplo, si alguien sale de la cárcel). |
| **R6** | Disparadores | La revisión se dispara automáticamente al editar `age`, `is_dead`, `in_jail` o `is_boss`, al normalizar el estado inicial al arrancar el menú si el jefe cargado no es elegible, y manualmente desde la opción *"Forzar revisión de sucesión"*. |
| **R7** | Sin sucesor | Si no hay candidatos libres ni interinos y el jefe actual murió o superó los 70 años, se limpia `is_boss` (`was_boss=1, is_boss=0` para el saliente). Nadie queda con `is_boss=1`, se informa la situación y se reintenta en futuras ediciones o revisiones. |

### Supuestos de diseño

- El árbol de `bin/datos.csv` tiene 10 integrantes (`id` 1–10) en pre-orden `1, 2, 4, 8, 9, 5, 3, 6, 10, 7`, cubriendo nodos vivos libres, presos (`3 Fredo`, `10 Peter`), muerto (`9 Mary`) y mayor de 70 (`7 Carlo`, 72 años).
- Al editar `is_boss = 1` sobre otro integrante, se normaliza para mantener como máximo un `is_boss = 1` y se registra `was_boss = 1` en el jefe saliente; luego se ejecuta `revisar_sucesion` para validar elegibilidad.
- Al cargar el CSV, si el jefe marcado con `is_boss = 1` ya es inelegible (muerto, `> 70` o preso con libres disponibles), el arranque del menú dispara una revisión inicial automática.

### Escenarios de prueba verificados

| Escenario | Reglas | Acción | Resultado esperado |
|---|---|---|---|
| Línea inicial con `datos.csv` | R1, R2 | Opción 3 (Ver línea) con Vito (`id=1`) de jefe | Muestra en orden `2 Sonny`, `4 Michael`, `8 Vincent`, `5 Connie`, `6 Tom`; omite `9` (muerto), `3` y `10` (presos) y `7` (`> 70`). |
| Muerte del jefe | R3, R6 | Editar `id=1`: `is_dead=1` | Vito pasa a `was_boss=1, is_boss=0`; asume `2 Sonny` (`id=2`). |
| Edad > 70 del jefe | R2, R3, R6 | Editar jefe actual: `age=71` | El jefe saliente deja el cargo (`was_boss=1`); asume el siguiente libre en R1. |
| Encarcelamiento con libres | R3, R6 | Editar jefe actual: `in_jail=1` | Si existe un libre elegible, el jefe preso entrega el cargo al primero de la línea. |
| Todos libres fuera -> Interinato | R4, R6 | Dejar sin libres elegibles (muertos/`>70`/presos) y tumbar al jefe | Asume el primer preso vivo `<= 70` (`3 Fredo` o `10 Peter`) con `is_boss=1` e `in_jail=1`. |
| Relevo de interino | R5, R2, R6 | Liberar a un candidato `<= 70` (`in_jail=0`, incluso con `was_boss=1`) | El interino entrega el mando automáticamente al nuevo libre elegible. |
| Sin candidatos | R7 | Dejar a toda la familia muerta o `> 70` y revisar | Todos quedan con `is_boss=0`, se informa `[R7 - Sin sucesor]` y al recuperar a alguien reasume en la próxima revisión. |
| Pre-orden cíclico desde rama media | R1 | Poner de jefe a `6 Tom` y provocar sucesión | Recorre `10` (si libre), salta `7` (`>70`) y da la vuelta por la raíz `1 Vito` -> `2 Sonny` -> ... |

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
