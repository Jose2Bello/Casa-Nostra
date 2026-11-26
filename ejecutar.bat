@echo off
setlocal
title Casa Nostra - Ejecutar
cd /d "%~dp0"

call compilar.bat nopause
if errorlevel 1 (
    if /i not "%~1"=="nopause" pause
    exit /b 1
)

bin\mafia_sucesion.exe
if /i not "%~1"=="nopause" pause
endlocal
