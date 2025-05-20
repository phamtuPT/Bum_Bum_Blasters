:: filepath: d:\3. Project\3. Games\2. ADVANCED PROGRAMMING\SDL2_GANGSTAT\build_and_run.bat
@echo off

:: Build using Makefile
make
if %errorlevel% neq 0 (
    echo Build failed.
    exit /b %errorlevel%
)

:: Run the executable
main.exe

if %errorlevel% neq 0 (
    echo Execution failed.
    exit /b %errorlevel%
)

:: Pause to see the output
pause

