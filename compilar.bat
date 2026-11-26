@echo off
setlocal
title Casa Nostra - Compilar
cd /d "%~dp0"

g++ -std=c++17 -Wall -Wextra -O2 -static src\main.cpp src\persona.cpp src\util.cpp src\csv.cpp src\arbol.cpp src\sucesion.cpp src\menu.cpp -o bin\mafia_sucesion.exe

if errorlevel 1 (
    echo [ERROR] No se pudo compilar.
    if /i not "%~1"=="nopause" pause
    exit /b 1
)

echo [OK] Ejecutable generado en bin\mafia_sucesion.exe
if /i not "%~1"=="nopause" pause
endlocal
