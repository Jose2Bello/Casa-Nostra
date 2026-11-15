@echo off
setlocal

g++ -std=c++17 -Wall -Wextra -O2 -static src\main.cpp src\persona.cpp src\util.cpp -o bin\mafia_sucesion.exe

if errorlevel 1 (
    echo [ERROR] No se pudo compilar.
    exit /b 1
)

echo [OK] Ejecutable generado en bin\mafia_sucesion.exe
endlocal
