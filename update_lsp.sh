#!/bin/bash

# Exit immediately if a command exits with a non-zero status
set -e

echo "Updating compile_commands.json for LazyVim..."

# Configure build/ inside the course container (the host needs no CMake, Flex,
# or Bison) with compile commands exported. The repo is mounted at its host
# path, so the paths in compile_commands.json are valid for clangd outside. It
# also builds once, so clangd can find the files Flex and Bison generate.
docker run --rm \
  --user "$(id -u):$(id -g)" -e HOME=/tmp \
  -v "$(pwd)":"$(pwd)" -w "$(pwd)" \
  maxxing/compiler-dev bash -c '
    cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -B build >/dev/null
    cmake --build build -j"$(nproc)" >/dev/null
    cp build/compile_commands.json .
  '

echo "LSP configuration updated successfully!"
