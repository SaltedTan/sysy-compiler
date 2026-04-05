#!/bin/bash

echo "Starting interactive Docker shell..."
echo "Type 'exit' or press Ctrl+D to return to your local machine."

docker run -it --rm \
  -v $(pwd):/root/repo \
  -w /root/repo \
  maxxing/compiler-dev bash
