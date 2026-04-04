#!/bin/bash
# Configure the project, then build it
docker run --rm -v $(pwd):/root/repo -w /root/repo maxxing/compiler-dev bash -c "cmake -DCMAKE_BUILD_TYPE=Debug -B build && cmake --build build"
