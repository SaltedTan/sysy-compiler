# SysY Compiler

A compiler for the SysY programming language (a subset of C), developed following the Peking University Compiler Principles Practice.

## Environment & Requirements

This project uses a hybrid development setup:
* **Code Editing:** Local environment (WSL2 / macOS) using `clangd` for LSP features.
* **Compilation:** Dockerized environment (`maxxing/compiler-dev`) to ensure consistent builds and dependency management.

### Local Setup (For IDE / LSP Support)
To generate the `compile_commands.json` for accurate C/C++ autocompletion, you need local toolchains installed (CMake, Clang, Flex, Bison).

Run the following in the project root to generate the LSP configuration:
```bash
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -B build
cp build/compile_commands.json .
rm -rf build
```

## Build Instructions
Do not use standard local cmake commands to build the project. Instead, use the provided Docker wrapper script, which mounts the current directory into the official course container and executes the build.
```bash
./build.sh
```
The compiled executable will be output to `build/compiler`.
