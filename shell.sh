#!/bin/bash

echo "Starting interactive Docker shell..."
echo "Type 'exit' or press Ctrl+D to return to your local machine."

# Runs as the host user, so files created in the repo are owned by you rather
# than root, and mounts the repo at its host path (see build.sh). The prompt
# shows "I have no name!" because the container has no such user; that is
# harmless.
docker run -it --rm \
  --user "$(id -u):$(id -g)" -e HOME=/tmp \
  -v "$(pwd)":"$(pwd)" -w "$(pwd)" \
  maxxing/compiler-dev bash
