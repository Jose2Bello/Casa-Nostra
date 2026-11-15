#include "util.h"

#include <cctype>

std::string trim(const std::string& texto) {
    int inicio = 0;
    int fin = static_cast<int>(texto.size()) - 1;

    while (inicio <= fin &&
           std::isspace(static_cast<unsigned char>(texto[inicio]))) {
        inicio++;
    }
    while (fin >= inicio &&
           std::isspace(static_cast<unsigned char>(texto[fin]))) {
        fin--;
    }

    if (inicio > fin) return "";
    return texto.substr(inicio, fin - inicio + 1);
}

std::string aMinusculas(const std::string& texto) {
    std::string resultado = texto;
    for (int i = 0; i < static_cast<int>(resultado.size()); i++) {
        resultado[i] = static_cast<char>(
            std::tolower(static_cast<unsigned char>(resultado[i])));
    }
    return resultado;
}

int dividirCSV(const std::string& linea, std::string campos[], int maxCampos) {
    int cantidad = 0;
    std::string actual;

    for (int i = 0; i <= static_cast<int>(linea.size()); i++) {
        bool fin = (i == static_cast<int>(linea.size()));

        if (!fin && linea[i] != ',') {
            actual += linea[i];
            continue;
        }

        if (cantidad >= maxCampos) return -1;
        campos[cantidad] = trim(actual);
        cantidad++;
        actual.clear();
    }

    return cantidad;
}

bool convertirEntero(const std::string& texto, int& valor) {
    std::string t = trim(texto);
    if (t.empty()) return false;

    int i = 0;
    bool negativo = false;
    if (t[0] == '+' || t[0] == '-') {
        negativo = (t[0] == '-');
        i = 1;
    }
    if (i >= static_cast<int>(t.size())) return false;

    long long acumulado = 0;
    for (; i < static_cast<int>(t.size()); i++) {
        if (t[i] < '0' || t[i] > '9') return false;
        acumulado = acumulado * 10 + (t[i] - '0');
        if (acumulado > 1000000000LL) return false;
    }

    valor = static_cast<int>(negativo ? -acumulado : acumulado);
    return true;
}

bool convertirBooleano(const std::string& texto, bool& valor) {
    std::string t = trim(texto);
    if (t == "0") { valor = false; return true; }
    if (t == "1") { valor = true;  return true; }
    return false;
}

bool convertirGenero(const std::string& texto, char& valor) {
    std::string t = trim(texto);
    if (t.size() != 1) return false;

    char c = static_cast<char>(
        std::toupper(static_cast<unsigned char>(t[0])));
    if (c != 'H' && c != 'M') return false;

    valor = c;
    return true;
}
