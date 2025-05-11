#!/bin/bash

# Create release directory
mkdir -p release/linux
mkdir -p release/windows

# Build Linux version
echo "Building Linux version..."
mkdir -p build_release_linux
cd build_release_linux
cmake .. -DCMAKE_BUILD_TYPE=Release
make
strip flappy-ball # Remove debug symbols to reduce size

# Copy Linux executable and assets
cp flappy-ball ../release/linux/
cp -r ../assets ../release/linux/

# Create Linux README
cat > ../release/linux/README.txt << EOL
Flappy Ball - Linux Version

Requirements:
- OpenGL
- GLUT (freeglut)
- OpenAL

On Ubuntu/Debian, install dependencies with:
sudo apt-get install freeglut3 libopenal1

To run:
1. Open terminal in this directory
2. Make the game executable: chmod +x flappy-ball
3. Run: ./flappy-ball

Controls:
- Space/Left Click: Jump
- P: Pause
- R: Restart
- M: Toggle music
- Esc: Quit
EOL

# Create a run script
cat > ../release/linux/run.sh << EOL
#!/bin/bash
chmod +x flappy-ball
./flappy-ball
EOL
chmod +x ../release/linux/run.sh

echo "Linux build complete!"

# Note: Windows build should be done on Windows
echo "Note: Windows build should be completed on a Windows system"
cat > ../release/windows/BUILD_INSTRUCTIONS.txt << EOL
To build the Windows version:

1. Install required software:
   - Visual Studio Build Tools 2022
   - CMake
   - Git

2. Open CMD and run:
   mkdir build
   cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release
   cmake --build . --config Release

3. Copy the following files to a new folder:
   - build/Release/flappy-ball.exe
   - assets/ folder
   - Required DLLs (freeglut.dll, OpenAL32.dll)

The Windows release should include:
- flappy-ball.exe
- freeglut.dll
- OpenAL32.dll
- assets/ folder
- README.txt
EOL

echo "Release preparation complete!"
