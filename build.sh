#!/bin/bash
# Configure the project, then build it inside the course container.
# Runs as the host user, so build/ is owned by you rather than root, and mounts
# the repo at its host path, so CMake's cache and compile_commands.json paths
# are the same inside and outside the container.
docker run --rm \
  --user "$(id -u):$(id -g)" -e HOME=/tmp \
  -v "$(pwd)":"$(pwd)" -w "$(pwd)" \
  maxxing/compiler-dev ./compile.sh
