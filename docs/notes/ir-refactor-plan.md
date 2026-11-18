# IR refactor: handoff note

Dated 2026-11-18. `main` is at `e7053f9`, pushed, working tree clean.

This note is for a fresh session. Read `AGENTS.md` first, then this. Together
they say where the project stands, what was decided and why, what comes next,
and what is still undecided. Line numbers were checked against `e7053f9`; if
`src/` has moved since, re-check them before quoting.

## Rules for agents picking this up

- This is a learning project. You tutor and review; the user writes the code.
- Never create, edit, or delete anything under `src/` or `CMakeLists.txt`, by
  any means (edit tools, `sed -i`, redirection, `patch`, scripts).
- Never write a working implementation of the feature in progress, in a file
  or in chat. Koopa IR or RISC-V text showing what output should look like is
  fine; C++ for the feature is not.
- Help by climbing the hint ladder in `AGENTS.md` one rung at a time: question,
  then concept and where to read, then prose or an example from a different
  setting, then pseudo-code only when asked.
- Reviews: `file:line`, what goes wrong, an input that triggers it, and why.
  No fix. Correctness before style.
- The open questions below belong to the user. Do not answer them unprompted.
- You may write under `tests/`, `scripts/`, and `docs/notes/`. Commit only when
  asked.

## Status

The pipeline is Flex/Bison, AST, Koopa IR text, libkoopa, RISC-V. Today the
AST prints Koopa text directly: every node has `Dump()` in `src/ast.hpp`, and
`src/main.cpp:34` captures its output.

Levels 1 to 7 are done. Level 8 is in progress:

| Piece | State |
|---|---|
| Parsing of functions, parameters, calls | done |
| Koopa emission for calls, parameters, implicit returns | done |
| Runtime library `decl`s (`src/ast.hpp:188-207`, commit `238b714`) | done |
| Backend skips declared functions (`src/codegen.cpp:34-36`, `e7053f9`) | done |
| Globals | not started |
| Backend: calls, parameters, globals | not started |

Test results reported at the time of writing (`autotest` in the
`maxxing/compiler-dev` container; not re-run while writing this note):

| Level | Koopa | RISC-V |
|---|---|---|
| lv1, lv3 to lv7 | all pass | all pass |
| lv8 | 9/12 | 0/12 |

There is no lv2 test directory.

- lv8 Koopa failures: `09_globals`, `10_complex`, `11_short_circuit`. A
  top-level `int x;` goes through `VarDefAST::Dump()` (`src/ast.hpp:633-643`)
  and comes out as a local `alloc` outside any function. Globals are simply not
  implemented. The expected form is a module-level definition such as
  `global @x = alloc i32, zeroinit`.
- lv8 RISC-V failures: the backend has no calls, parameters, or globals. Also,
  every function's first block is labelled `%entry` (`src/ast.hpp:271`) and
  the backend prints block names as assembly labels with the sigil dropped
  (`src/codegen.cpp:73-77`), so two functions give a duplicate `entry:` symbol.

### The new IR (merged, unused)

Commit `69fb917` merged branch `ir-refactor` (three commits from June 2026,
written by the user). It adds:

- `src/ir.hpp`, `src/ir.cpp`: an in-memory IR, `ir::Module`, `Function`,
  `BasicBlock`, `Instruction`, `Value`, `Type`.
- `src/ir_builder.hpp`, `src/ir_builder.cpp`: `IRBuilder`, which creates
  functions and blocks, tracks an insert point, and appends instructions.

It compiles and nothing calls it. What it covers today: types `i32`, `unit`,
`ptr`; instructions alloc, load, store, binary, jump, branch, return
(`src/ir.hpp:41-49`). What it lacks: a printer, function parameters, calls,
declarations, globals, arrays.

## Decisions

Each is settled. Reopen one only if the user asks.

1. **One type system, in `ir::Type`.** SysY types map one to one onto Koopa
   types (`int` to `i32`, `void` to `unit`, arrays to arrays, array parameters
   to pointers), so a separate front-end type would only duplicate it.
   `BaseType` (`src/ast.hpp:30`) goes away once nothing uses it, including the
   `BaseType::Array` stand-in used for library signatures
   (`src/ast.hpp:57`, `:60`) and the dead `Array` case in the implicit-return
   switch in `FuncDefAST` (`src/ast.hpp:300`).
2. **Types are interned** (the user's choice). Structural equality becomes
   pointer equality. The argument is inductive: if the bases are interned, two
   pointer types are equal exactly when their base pointers are. The `i32` and
   `unit` singletons and `ptr` keyed on its base pointer already exist
   (`src/ir.cpp:10-41`).
3. **`void` is a type**, `TypeKind::Unit`. The parser stays responsible for
   rejecting `void` where it is not allowed (`src/sysy.y:116-120` for
   parameters). See the gap under Known issues.
4. **No function type kind for now.** SysY has no function values; a call
   names its callee directly. A signature is a return type plus a list of
   parameter types held in the function symbol.
5. **The array kind arrives in level 9**, interned on (base, length).
6. **libkoopa stays between the IR and the backend.** The new IR gets a
   printer that emits Koopa text; `src/codegen.cpp` keeps consuming what
   libkoopa parses from that text. Reason: `koopac` and `autotest -koopa`
   validate the printer for free, and the backend is untouched by the
   refactor. Reading `ir::Module` directly in the backend is deferred to the
   optional performance track.
7. **Port side by side, not in place.** Add a second emission path on the AST
   nodes that returns `ir::Value *`. Keep `Dump()` working. Choose the path
   from the command line. Port node by node. Delete `Dump()` only at parity.
8. **Reach lv1 to lv7 parity on the new path before extending the IR for
   level 8**, so that a failing test implicates one change.

## Plan

In order. Do not start a step while the previous one has failing tests.

1. Fix the type issues listed under Known issues.
2. Write the IR printer: `ir::Module` to Koopa text.
3. Port AST emission to the builder, in this order, running both
   `autotest -koopa` and `autotest -riscv` for lv1 and lv3 to lv7 after each:
   1. expressions;
   2. variables and scopes;
   3. `if`, `while`, `break`, `continue`.
4. Extend the IR for level 8 (this needs the open questions answered). Port
   functions, parameters, calls, and library declarations. Get lv8 Koopa back
   to 9/12. Then implement globals for 09 to 11.
5. Backend for level 8: calls, saving `ra`, arguments in `a0` to `a7` then on
   the stack, frame size, block labels unique per function, globals. Reading:
   course doc Lv8.4, and the integer calling convention in the RISC-V psABI.
6. Level 9, arrays.

## Known issues

Findings, not fixes. The first three are step 1 of the plan.

- **`src/ir.hpp:51-58`, `Type` is open to construction and mutation.** It is a
  public aggregate with mutable fields, and the factories return non-const
  `Type *`. Any code can build a second, non-interned `i32` or pointer type,
  after which pointer equality gives false for equal types. Any code can also
  write through a returned pointer: changing the `kind` of the shared `i32`
  changes every integer in the program. Types should be immutable and
  obtainable only from the factories.
- **`src/ir_builder.cpp:122`, `emitLoad` always produces `i32`.** The result
  type should be the pointee type of the address. Harmless until level 9,
  where a stored `int a[]` parameter lives in a `**i32` slot and loading it
  must give `*i32`. Trigger: `int f(int a[]) { return a[0]; }`.
- **`src/ir_builder.cpp:93-94`, `IRBuilder::i32()` asserts a current
  function**, and stores the constant in that function's value list. A global
  initialiser (`int x = 5;` at top level) is built outside any function, so
  its constant has nowhere to live.
- **Types carry no byte size.** Not needed while libkoopa sits in the middle;
  needed if the backend ever reads `ir::Module` directly.

Found while checking this note against the code:

- **`void` variables are not rejected.** Decision 3 relies on the parser, but
  only the parameter rule checks. `VarDecl` and `ConstDecl` accept any `Type`
  (`src/sysy.y:510-514`, `:467-471`) and discard it, so `VarDefAST` emits
  `alloc i32` regardless. Trigger: `int main() { void x; return 0; }` is
  accepted.
- **Block names in the new IR are numbered per function**
  (`src/ir_builder.cpp:36-45`), where the `Dump()` path uses program-wide
  counters. Once several functions exist, hinted names can repeat across
  functions, not only `%entry`. That is the same duplicate-label problem as in
  Status, wider; it lands in plan step 5. Whether the repetition is legal
  Koopa is part of the naming question below.
- **`src/main.cpp:19` asserts exactly five arguments**, and `autotest` invokes
  the compiler in a fixed form (`compiler -koopa input -o output`). The path
  selection in decision 7 has to keep that invocation working.

## Open questions

For the user to decide. Listed, not answered.

- How does an `ir::Function` represent a `decl`, a function with no body? Can
  it be done without a new flag?
- Parameters are values with no defining instruction. `ValueKind` has only
  `Constant` and `Local` (`src/ir.hpp:36-39`). What is missing?
- A call names a function, and a function is not an `ir::Value`. Where does
  the callee go in `Instruction` (`src/ir.hpp:70-83`)?
- Where do global initialiser constants live?
- Value names are numbered per function (`src/ir_builder.cpp:25-34`); globals
  are module-wide. Can a local and a global collide? **Not known; needs
  checking** against the Koopa IR spec page ("Koopa IR 规范") in the course
  doc, which states which names may repeat and in which scope. Do not assume
  either answer.
- Which container keys the array interning table on (base, length)?
  `std::unordered_map` has no hash for a pair.

## How to verify

Start Docker first. From the repo root:

```bash
./build.sh                 # build build/compiler inside the container
./shell.sh                 # interactive container shell
```

Inside the container:

```bash
autotest -koopa -s lv8 "$PWD"   # Koopa IR tests for one level
autotest -riscv -s lv8 "$PWD"   # RISC-V tests for one level
```

One shot from the host, without an interactive shell:

```bash
docker run --rm --user "$(id -u):$(id -g)" -e HOME=/tmp \
  -v "$PWD":"$PWD" -w "$PWD" maxxing/compiler-dev \
  bash -c './compile.sh && autotest -koopa -s lv8 "$PWD"'
```

Replace `lv8` with `lv1`, `lv3` ... `lv7` for the parity runs. What to expect:

| When | Expectation |
|---|---|
| Now, `Dump()` path | lv1, lv3 to lv7 pass in both modes; lv8 Koopa 9/12; lv8 RISC-V 0/12 |
| After each part of plan step 3, new path | lv1, lv3 to lv7 pass in both modes |
| After plan step 4 | lv8 Koopa 9/12, then 12/12 with globals |
| After plan step 5 | lv8 RISC-V 12/12 |

Agents may build and run these and report results. They do not fix failures.
The user's own parse tests for level 8 are in `tests/lv8/parse/`.
