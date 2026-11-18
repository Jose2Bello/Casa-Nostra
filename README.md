# Casa Nostra — sucesión de la familia

Prototipo en C++ para administrar la sucesión del jefe de una familia a partir de
un archivo CSV. Cada integrante es un nodo de un **árbol binario**: el hijo
izquierdo es el primer sucesor y el hijo derecho el segundo.

> **Estado:** fase 2 de la infraestructura. Ya están la estructura del proyecto,
> el modelo de persona, las utilidades de texto y el lector de CSV con sus
> validaciones. Faltan el árbol, el índice, los recorridos, el `main` final y el
> menú.

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
│   ├── datos.csv          <- datos de prueba base
│   └── mafia_sucesion.exe <- se genera al compilar
└── src/
    ├── main.cpp
    ├── csv.h / csv.cpp
    ├── persona.h / persona.cpp
    └── util.h / util.cpp
```

Todo el código va en `src/`; el ejecutable y los datos van en `bin/`.

---

## Requisitos

- `g++` con soporte de C++17.
- Windows: `compilar.bat` o `mingw32-make`.
- Linux/macOS: `make`.

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
g++ -std=c++17 -Wall -Wextra -O2 -static src/main.cpp src/persona.cpp src/util.cpp src/csv.cpp -o bin/mafia_sucesion.exe
```

> En Windows se usa `-static` para no depender de las DLL de MinGW al ejecutar.

> `main.cpp` ya carga y valida `bin/datos.csv` y lista las personas. Admite una
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
bool cargarCSV(const char* ruta, ListaPersonas& lista, char* error, int tamError);
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

## División del trabajo

### Infraestructura (en curso)

- Estructura `bin/` + `src/`, build y `.gitignore`.
- Modelo `Persona` y utilidades de texto.
- Lector y validador de CSV, con `bin/datos.csv` base.
- **Pendiente:** construcción del árbol, índice por id y recorridos.
- **Pendiente:** `main.cpp` final y `menu.h`/stub.

### Menú y lógica (compañero)

- Menú interactivo en bucle hasta `0`.
- Línea de sucesión.
- Reglas de sucesión R1–R7 y disparadores (muerte, cárcel, edad > 70).
- Edición de personas (todo menos `id` e `id_boss`).
- Datos de prueba finales y documentación de reglas/pruebas.

---

## Reglas de sucesión

_(Pendiente: las implementa el compañero. Deben documentarse aquí con sus
supuestos y la tabla de escenarios.)_

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
