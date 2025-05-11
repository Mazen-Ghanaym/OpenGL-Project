#!/bin/bash

# Create build directory if it doesn't exist
mkdir -p build
cd build

# Configure and build using CMake
cmake ..
make

# Copy assets to build directory
cp -r ../assets .
