#!/bin/bash

# Check if running with sudo
if [[ $EUID -ne 0 ]]; then
   echo "This script must be run with sudo to install required packages"
   echo "Please run: sudo ./setup.sh"
   exit 1
fi

echo "Updating package list..."
apt-get update

echo "Installing required packages..."
apt-get install -y build-essential cmake libgl1-mesa-dev freeglut3-dev libopenal-dev

# Get the actual user who ran sudo
ACTUAL_USER=$SUDO_USER
if [ -z "$ACTUAL_USER" ]; then
    ACTUAL_USER=$(whoami)
fi

echo "Setting up build environment..."
mkdir -p build
cd build

echo "Building project..."
cmake ..
make -j$(nproc)

if [ -f flappy-ball ]; then
    echo "Build successful!"
    echo "You can now run the game with: ./build/flappy-ball"
    # Make the game executable
    chmod +x flappy-ball
    # Fix ownership
    chown -R $ACTUAL_USER:$ACTUAL_USER .
else
    echo "Build failed! Please check the error messages above."
    exit 1
fi
