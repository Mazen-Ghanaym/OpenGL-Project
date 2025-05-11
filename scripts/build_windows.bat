@echo off

REM Create build directory if it doesn't exist
if not exist build mkdir build
cd build

REM Configure and build using CMake
cmake -G "MinGW Makefiles" ..
cmake --build .

REM Copy assets to build directory
xcopy /E /I /Y ..\assets assets
