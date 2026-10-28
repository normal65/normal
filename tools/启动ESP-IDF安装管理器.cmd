@echo off
setlocal

set "PYTHON_DIR=E:\game\Chat_program1\Python312Nuget\tools"
set "EIM_EXE=C:\Users\MZ\.espressif\eim_gui\eim-gui-windows-x64-v0.20.0.exe"

if not exist "%PYTHON_DIR%\python.exe" (
    echo Python 3.12 not found: %PYTHON_DIR%\python.exe
    pause
    exit /b 1
)

if not exist "%EIM_EXE%" (
    echo ESP-IDF Install Manager not found: %EIM_EXE%
    pause
    exit /b 1
)

set "PATH=%PYTHON_DIR%;%PATH%"
echo Using:
"%PYTHON_DIR%\python.exe" --version
start "" "%EIM_EXE%"
