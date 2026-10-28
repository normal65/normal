@echo off
setlocal

powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0build_and_run.ps1"
if errorlevel 1 (
    echo.
    echo UI preview failed. Read the messages above.
    pause
    exit /b 1
)

echo.
echo UI preview generated and opened.
timeout /t 2 >nul
