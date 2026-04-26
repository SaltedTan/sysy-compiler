# SysY Compiler

A C++17 compiler for **SysY**, a teaching language based on a subset of C. The
compiler parses SysY source, builds an abstract syntax tree, lowers the program
to Koopa IR, and can emit either Koopa IR or RISC-V assembly.

This project was built while following the Peking University Compiler Principles
Practice curriculum. I use it as a resume project because it covers the full
front-to-back shape of a compiler: lexical analysis, parsing, AST construction,
IR generation, symbol handling, and backend code generation.

## Highlights

- Implements a Flex/Bison frontend for a C-like language grammar.
- Builds a structured AST in modern C++ using polymorphic nodes and
  `std::unique_ptr` ownership.
- Supports expression lowering, local declarations, constants, assignments, and
  returns.
- Generates Koopa IR as an intermediate representation.
- Uses the Koopa raw program API to traverse IR and emit RISC-V assembly.
- Allocates stack slots for local values and emitted temporaries in the backend.
- Includes Docker-based build scripts so the compiler can be built in a
  reproducible course environment.

## Compilation Pipeline

```text
SysY source
    |
    v
Flex lexer + Bison parser
    |
    v
C++ AST
    |
    v
Koopa IR text
    |
    +--> -koopa output
    |
    v
Koopa raw program
    |
    v
RISC-V assembly
```

## Current Language Support

The implemented subset focuses on scalar integer programs:

- A single zero-argument `int` function definition, including `main`.
- Integer literals in decimal, octal, and hexadecimal forms.
- Line comments and block comments.
- Local `const int` declarations.
- Local `int` variable declarations, with optional initialization.
- Assignment statements.
- `return` statements.
- Unary operators: `+`, `-`, `!`.
- Binary arithmetic operators: `+`, `-`, `*`, `/`, `%`.
- Relational and equality operators: `<`, `>`, `<=`, `>=`, `==`, `!=`.
- Logical operators: `&&`, `||`.

Notable missing pieces are control flow, arrays, global variables, function
parameters, function calls, and multiple function definitions. Those are natural
next steps for expanding the compiler.

## Example

Input SysY program:

```c
int main() {
  const int base = 16;
  int value = base + 10;
  value = value * 2;
  return value != 0;
}
```

Koopa IR output:

```koopa
fun @main(): i32 {
%entry:
  @value = alloc i32
  %0 = add 16, 10
  store %0, @value
  %1 = load @value
  %2 = mul %1, 2
  store %2, @value
  %3 = load @value
  %4 = ne %3, 0
  ret %4
}
```

The same input can also be lowered to RISC-V assembly through the backend.

## Repository Structure

```text
.
├── build.sh             # Runs the build inside the Docker course container
├── compile.sh           # CMake configure/build command used by build.sh
├── CMakeLists.txt       # Build configuration for C++, Flex, Bison, and Koopa
├── shell.sh             # Opens an interactive Docker development shell
├── src/
│   ├── ast.hpp          # AST nodes, symbol table, and Koopa IR emission
│   ├── codegen.cpp      # Koopa raw IR to RISC-V assembly generator
│   ├── codegen.hpp      # RISC-V generator interface
│   ├── main.cpp         # Compiler driver and output mode selection
│   ├── sysy.l           # Flex lexer
│   └── sysy.y           # Bison parser
└── update_lsp.sh        # Generates compile_commands.json for local editors
```

## Requirements

The recommended build path uses Docker:

- Docker
- `maxxing/compiler-dev` image, used by the PKU compiler practice environment

For local editor support, install the usual native tooling as well:

- CMake
- Clang or GCC
- Flex
- Bison
- `clangd`, if you want LSP features

## Build

Build the compiler from the project root:

```bash
./build.sh
```

This mounts the repository into the Docker container and runs `./compile.sh`.
The resulting executable is written to:

```text
build/compiler
```

For an interactive container shell:

```bash
./shell.sh
```

Inside the container, rebuild with:

```bash
./compile.sh
```

## Usage

Generate Koopa IR:

```bash
./build/compiler -koopa debug/hello.c -o /tmp/hello.koopa
```

Generate RISC-V assembly:

```bash
./build/compiler -riscv debug/hello.c -o /tmp/hello.s
```

The driver expects the course-style command shape:

```text
compiler <mode> <input> -o <output>
```

`-koopa` writes textual Koopa IR. Other modes follow the RISC-V backend path;
the conventional mode name is `-riscv`.

## Local IDE Support

The Docker build owns the main `build/` directory, so avoid running raw local
CMake commands into that directory. To generate a local `compile_commands.json`
for `clangd`, run:

```bash
./update_lsp.sh
```

The script creates a temporary `build_local/` directory, copies
`compile_commands.json` to the repo root, and removes the temporary directory so
it does not collide with the Docker build.

## Implementation Notes

The frontend is split between `src/sysy.l` and `src/sysy.y`. The lexer handles
keywords, identifiers, comments, integer literal formats, and multi-character
operators. The parser builds AST nodes directly through Bison semantic actions.

`src/ast.hpp` currently owns both the AST model and the first lowering step to
Koopa IR. Constant declarations are evaluated during AST traversal and stored in
a simple symbol table, while mutable locals are allocated as Koopa `alloc`
values and accessed with `load`/`store`.

The backend in `src/codegen.cpp` consumes Koopa's raw program representation.
It walks functions, basic blocks, and instructions; assigns stack offsets to
non-unit Koopa values; and emits RISC-V instructions for integer constants,
binary operations, loads, stores, and returns.
