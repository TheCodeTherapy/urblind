#!/bin/bash
set -e # Exit immediately if a command fails

# Get the project name dynamically
PROJECT_NAME=$(basename "$PWD")

# Initialize flags
clean=false
run=false
run_args=() # Array to store arguments for the binary

# Parse command line arguments (order-independent)
for arg in "$@"; do
  case "$arg" in
  clean) clean=true ;;
  --run) run=true ;;
  --)
    shift
    break
    ;; # Stop parsing script options when "--" is found
  *)
    if [[ "$run" == false ]]; then
      echo "Unknown argument: $arg"
      exit 1
    fi
    run_args+=("$arg") # Collect arguments for the binary
    ;;
  esac
done

# If --run is provided, collect remaining args as binary args
if $run; then
  while [[ $# -gt 0 ]]; do
    run_args+=("$1")
    shift
  done
fi

# Remove compile_commands.json if it exists
if [[ -f compile_commands.json ]]; then
  rm compile_commands.json
fi

# Handle "clean" option
if $clean; then
  echo "Building everything from scratch..."
  rm -rf build || {
    echo "Could not remove build directory."
    exit 1
  }
fi

mkdir -p build
cd build

# Run CMake (clearing cache if clean was requested)
if $clean; then
  cmake .. -DCMAKE_CACHE_CLEAR=ON
else
  cmake ..
fi

cmake --build .

# Symlink compile_commands.json if it exists
if [[ -f build/compile_commands.json ]]; then
  ln -s build/compile_commands.json compile_commands.json
fi

# Run the executable if "--run" was provided
if $run; then
  if [[ -f "./$PROJECT_NAME" ]]; then
    echo "Running ./$PROJECT_NAME ${run_args[*]}..."
    ./"$PROJECT_NAME" "${run_args[@]}"
  else
    echo "Build succeeded, but executable '$PROJECT_NAME' not found!"
    exit 1
  fi
fi
