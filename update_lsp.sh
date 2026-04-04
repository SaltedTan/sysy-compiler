#!/bin/bash

# Exit immediately if a command exits with a non-zero status
set -e

echo "Updating compile_commands.json for LazyVim..."

# 1. Generate the compile commands using local CMake
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -B build_local

# 2. Move the JSON file to the project root
cp build_local/compile_commands.json .

# 3. Clean up the local build directory to avoid Docker conflicts
rm -rf build_local

echo "LSP configuration updated successfully!"
