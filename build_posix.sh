#!/bin/bash
set -e # Exit immediately if a command fails

# Get the project name dynamically
PROJECT_NAME=$(basename "$PWD")

if [[ -f compile_commands.json ]]; then
  rm compile_commands.json
fi

# Check for clean option
if [[ "$1" == "clean" ]]; then
  echo "Building everything from scratch..."
  rm -rf build || {
    echo "Could not remove build directory."
    exit 1
  }
fi

mkdir -p build
cd build

# Run CMake (only clear cache if --clean was passed)
if [[ "$1" == "clean" ]]; then
  cmake .. -DCMAKE_CACHE_CLEAR=ON
else
  cmake ..
fi

cmake --build .

if [[ -f build/compile_commands.json ]]; then
  ln -s build/compile_commands.json compile_commands.json
fi

# Ensure the executable exists before running
if [[ -f "./$PROJECT_NAME" ]]; then
  echo "Running ./$PROJECT_NAME..."
  ./"$PROJECT_NAME"
else
  echo "Build succeeded, but executable '$PROJECT_NAME' not found!"
  exit 1
fi
