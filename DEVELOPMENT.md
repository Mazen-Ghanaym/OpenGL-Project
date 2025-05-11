# Development Guide

## Project Structure

```
flappy-ball/
├── src/              # Source code files
│   └── sample.cpp    # Main game implementation
├── include/          # Header files for game components
├── assets/          # Game assets
│   ├── sfx/         # Sound effects
│   └── textures/    # Game textures and images
├── scripts/         # Build and utility scripts
├── build/           # Debug build outputs
├── release/         # Release packages
├── CMakeLists.txt   # CMake build configuration
└── README.md        # Project documentation
```

## Development Setup

### Prerequisites

1. CMake 3.10 or higher
2. C++14 compatible compiler
3. OpenGL development libraries
4. FreeGLUT development libraries
5. OpenAL development libraries

### Linux Setup

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get update
sudo apt-get install build-essential cmake freeglut3-dev libopenal-dev

# Clone and build
git clone https://github.com/yourusername/flappy-ball.git
cd flappy-ball
./scripts/build_linux.sh
```

### Windows Setup

1. Install required tools:

   - Visual Studio Build Tools 2022 with "Desktop development with C++"
   - CMake 3.10 or higher
   - Git

2. Clone and build:
   ```batch
   git clone https://github.com/yourusername/flappy-ball.git
   cd flappy-ball
   scripts\build_windows.bat
   ```

## Build Modes

1. Debug Build (development)

   - Includes debug symbols
   - No optimizations
   - More verbose logging

   ```bash
   ./scripts/build_linux.sh  # Linux
   scripts\build_windows.bat # Windows
   ```

2. Release Build (distribution)
   - Full optimizations
   - No debug symbols
   - Minimal logging
   ```bash
   ./scripts/make_release.sh        # Linux
   scripts\make_release_windows.bat # Windows
   ```

## Adding New Features

1. Game Components

   - Add header files in `include/`
   - Implement in `src/`
   - Update CMakeLists.txt if adding new source files

2. Assets
   - Sound effects go in `assets/sfx/`
   - Textures go in `assets/textures/`
   - Update README.md with new asset credits/licenses

## Code Style

1. Use consistent indentation (spaces preferred)
2. Add comments for complex logic
3. Use meaningful variable and function names
4. Keep functions focused and small
5. Add header guards in all header files

## Testing

1. Build and run in Debug mode first
2. Test on both Linux and Windows
3. Verify all features work before committing

## Making a Release

1. Update version numbers
2. Run full release builds:
   ```bash
   ./scripts/make_release.sh        # Linux
   scripts\make_release_windows.bat # Windows
   ```
3. Test release packages
4. Tag the release in git
