# Casa Nostra — sucesión de la familia

Prototipo en C++ para administrar la sucesión del jefe de una familia a partir de
un archivo CSV. Cada integrante es un nodo de un **árbol binario**: el hijo
izquierdo es el primer sucesor y el hijo derecho el segundo.

> **Estado:** fase 1 de la infraestructura. Ya están la estructura del proyecto,
> el modelo de persona y las utilidades de texto. Faltan el lector de CSV, el
> árbol, el motor de sucesión y el menú.

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
│   ├── datos.csv          <- (pendiente) datos de prueba
│   └── mafia_sucesion.exe <- se genera al compilar
└── src/
    ├── main.cpp
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
g++ -std=c++17 -Wall -Wextra -O2 -static src/main.cpp src/persona.cpp src/util.cpp -o bin/mafia_sucesion.exe
```

> En Windows se usa `-static` para no depender de las DLL de MinGW al ejecutar.

> Por ahora `main.cpp` es una prueba de humo de la fase 1. En la siguiente fase
> cargará `bin/datos.csv` y arrancará el menú.

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

## División del trabajo

### Infraestructura (en curso)

- Estructura `bin/` + `src/`, build y `.gitignore`.
- Modelo `Persona` y utilidades de texto.
- **Pendiente:** lector y validador de CSV.
- **Pendiente:** construcción del árbol, índice por id y recorridos.
- **Pendiente:** `main.cpp` final y `menu.h`/stub.
- **Pendiente:** `bin/datos.csv` base.

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
- _(compañero: completar nombre y usuario)_
