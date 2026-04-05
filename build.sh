#!/bin/bash
# Configure the project, then build it
docker run --rm -v $(pwd):/root/repo -w /root/repo maxxing/compiler-dev ./compile.sh
