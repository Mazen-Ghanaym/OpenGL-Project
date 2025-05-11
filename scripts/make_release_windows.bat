@echo off
setlocal enabledelayedexpansion

REM Create release directories
mkdir release\windows 2>nul

REM Build Windows version
echo Building Windows version...
mkdir build_release_windows 2>nul
cd build_release_windows

REM Configure and build with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release -G "MinGW Makefiles"
if errorlevel 1 goto error
cmake --build . --config Release
if errorlevel 1 goto error

REM Copy Windows executable and assets
copy Release\flappy-ball.exe ..\release\windows\
xcopy /E /I /Y ..\assets ..\release\windows\assets

REM Copy required DLLs (you need to adjust paths based on your system)
copy "%PROGRAMFILES%\freeglut\bin\freeglut.dll" ..\release\windows\
copy "%PROGRAMFILES%\OpenAL\bin\OpenAL32.dll" ..\release\windows\

REM Create Windows README
echo Flappy Ball - Windows Version > ..\release\windows\README.txt
echo. >> ..\release\windows\README.txt
echo Requirements: >> ..\release\windows\README.txt
echo - Windows 7 or later >> ..\release\windows\README.txt
echo. >> ..\release\windows\README.txt
echo To run: >> ..\release\windows\README.txt
echo Simply double-click flappy-ball.exe >> ..\release\windows\README.txt
echo. >> ..\release\windows\README.txt
echo Controls: >> ..\release\windows\README.txt
echo - Space/Left Click: Jump >> ..\release\windows\README.txt
echo - P: Pause >> ..\release\windows\README.txt
echo - R: Restart >> ..\release\windows\README.txt
echo - M: Toggle music >> ..\release\windows\README.txt
echo - Esc: Quit >> ..\release\windows\README.txt

echo Windows build complete!
goto :eof

:error
echo Build failed!
exit /b 1
