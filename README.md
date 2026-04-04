# SysY Compiler

A compiler for the SysY programming language (a subset of C), developed following the Peking University Compiler Principles Practice.

## Environment & Requirements

This project uses a hybrid development setup:
* **Code Editing:** Local environment (WSL2 / macOS) using `clangd` for LSP features.
* **Compilation:** Dockerized environment (`maxxing/compiler-dev`) to ensure consistent builds and dependency management.

## Project Structure

```text
.
├── build.sh             # Wrapper to configure and build the compiler inside Docker
├── CMakeLists.txt       # Project build configuration
├── src/                 # Compiler source code
└── update_lsp.sh        # Script to safely generate local compile_commands.json
```

## Local Setup (For IDE / LSP Support)

To get accurate C/C++ autocompletion in editors like Neovim or VS Code, your editor needs a `compile_commands.json` file. You must have local toolchains installed (CMake, Clang, Flex, Bison) to generate this.

Do not run raw `cmake` locally, as it will cause cache collisions with the Docker build folder. Instead, use the provided script whenever you add or remove source files:
Run the following in the project root to generate the LSP configuration:
```bash
./update_lsp.sh
```

## Build Instructions
Do not use standard local `cmake` commands to build the project. Instead, use the provided Docker wrapper script. This script mounts the current directory into the official course container, configures the project, and executes the build.
```bash
./build.sh
```
The compiled executable will be output to `build/compiler`.
