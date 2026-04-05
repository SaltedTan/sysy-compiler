#!/bin/bash

set -e

cmake -DCMAKE_BUILD_TYPE=Debug -B build
cmake --build build -j $(nproc)
