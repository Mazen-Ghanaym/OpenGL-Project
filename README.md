# 🎮 Flappy Ball OpenGL

A modern take on the classic Flappy Bird game with power-ups, particle effects, and immersive sound!

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++: 14](https://img.shields.io/badge/C%2B%2B-14-blue.svg)](https://en.cppreference.com/w/cpp/14)
[![OpenGL: 2.1+](https://img.shields.io/badge/OpenGL-2.1%2B-red.svg)](https://www.opengl.org/)

## 📜 Table of Contents

- [🎮 Flappy Ball OpenGL](#-flappy-ball-opengl)
  - [📜 Table of Contents](#-table-of-contents)
  - [🚀 Quick Start](#-quick-start)
    - [For Linux Users](#for-linux-users)
    - [For Windows Users](#for-windows-users)
  - [🔨 Building from Source](#-building-from-source)
    - [Linux Build Guide](#linux-build-guide)
    - [Windows Build Guide](#windows-build-guide)
  - [📂 Project Structure](#-project-structure)
  - [📦 Dependencies](#-dependencies)
  - [🔧 Development](#-development)
    - [Build Modes](#build-modes)
    - [Development Tips](#development-tips)
  - [🎮 Game Controls](#-game-controls)
  - [🌟 Features](#-features)
  - [👥 Contributing](#-contributing)
  - [📄 License](#-license)

## 🚀 Quick Start

### For Linux Users

First, install the dependencies and build the game:

```bash
# Install dependencies
sudo apt-get update
sudo apt-get install build-essential cmake freeglut3-dev libopenal-dev

# Get the code and build
git clone https://github.com/Mazen-Ghanaym/OpenGL-Project.git flappy-ball
cd flappy-ball
./scripts/build_linux.sh
cd build && ./flappy-ball
```

### For Windows Users

1. Install these prerequisites:

   - [Visual Studio Build Tools 2022](https://visualstudio.microsoft.com/downloads/) (select "Desktop development with C++")
   - [CMake](https://cmake.org/download/) (add to PATH)
   - [Git](https://git-scm.com/download/win)

2. Build and run:

   ```batch
   git clone https://github.com/Mazen-Ghanaym/OpenGL-Project.git flappy-ball
   cd flappy-ball
   scripts\build_windows.bat
   cd build
   flappy-ball.exe
   ```

## 🔨 Building from Source

### Linux Build Guide

Follow these steps to build from source on Linux:

```bash
# 1. Install required packages
sudo apt-get update
sudo apt-get install build-essential cmake freeglut3-dev libopenal-dev

# 2. Clone the repository
git clone https://github.com/Mazen-Ghanaym/OpenGL-Project.git flappy-ball
cd flappy-ball

# 3. Create build directory
mkdir -p build && cd build

# 4. Configure and build
cmake ..
make

# 5. Run the game
./flappy-ball
```

### Windows Build Guide

1. Install Dependencies:

   - Visual Studio Build Tools 2022 with "Desktop development with C++"
   - CMake 3.10 or higher
   - Git

2. Build Steps:

   ```batch
   git clone https://github.com/Mazen-Ghanaym/OpenGL-Project.git flappy-ball
   cd flappy-ball
   mkdir build
   cd build
   cmake .. -G "MinGW Makefiles"
   cmake --build .
   ```

## 📂 Project Structure

```plaintext
flappy-ball/
├── src/              # Source code files
│   └── sample.cpp    # Main game implementation
├── include/          # Header files (for future use)
├── assets/          # Game assets
│   ├── sfx/         # Sound effects (for future use)
│   └── textures/    # Game textures (for future use)
├── scripts/         # Build and utility scripts
│   ├── build_linux.sh        # Debug build script for Linux
│   ├── build_windows.bat     # Debug build script for Windows
│   ├── make_release.sh       # Release build script for Linux
│   ├── make_release_windows.bat  # Release build script for Windows
│   └── setup_linux.sh       # Linux environment setup
├── CMakeLists.txt   # Build system configuration
├── DEVELOPMENT.md   # Development guide
├── LICENSE          # MIT License
└── README.md        # Project documentation
```

## 📦 Dependencies

- C++14 or higher
- OpenGL 2.1+
- GLUT/FreeGLUT
- CMake 3.10 or higher

## 🔧 Development

### Build Modes

The project supports two build modes:

1. Debug (default)

   ```bash
   # Linux
   ./scripts/build_linux.sh

   # Windows
   scripts\build_windows.bat
   ```

2. Release (optimized)

   ```bash
   # Linux
   ./scripts/make_release.sh

   # Windows
   scripts\make_release_windows.bat
   ```

### Development Tips

1. **Code Organization**

   - Main game logic is in `src/sample.cpp`
   - Place header files in `include/`
   - Add sound effects to `assets/sfx/`

2. **Build System**
   - Uses CMake for cross-platform compatibility
   - Supports both MinGW and MSVC on Windows
   - Debug builds include symbols for debugging
   - Release builds are optimized for performance

- OpenAL
- CMake 3.10+

## 🎮 Game Controls

- Spacebar / Left Mouse Button: Make the ball jump
- Esc: Pause game
- P: Toggle pause
- R: Restart game
- M: Toggle music
- S: Toggle sound effects

## 🌟 Features

- Modern OpenGL graphics with smooth animations
- Dynamic particle effects
- Immersive sound effects and background music
- Difficulty progression
- Power-up system
- Score tracking and high scores
- Cross-platform compatibility (Windows, Linux)

## 👥 Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
