:: filepath: d:\3. Project\3. Games\2. ADVANCED PROGRAMMING\SDL2_GANGSTAT\build_and_run.bat
@echo off
:: Set the DLL directory
set DLL_DIR=%~dp0libs

:: Add DLL directory to PATH
set PATH=%PATH%;%DLL_DIR%

:: Build using Makefile
mingw32-make
if %errorlevel% neq 0 (
    echo Build failed.
    exit /b %errorlevel%
)

:: Run the executable
main.exe