# Bison: semantic values, the start rule, and ownership

Written while building `CompUnit` / `CompUnitList` for Lv8. The same ideas
come back for every list rule: function parameters, call arguments, and later
array initialiser lists.

## 1. When does an action run?

Bison generates a bottom-up (LALR(1)) parser. It keeps a stack and picks one of
two moves:

- **Shift**: push the next token.
- **Reduce**: the top of the stack matches a rule's right-hand side, so pop
  those symbols and push the left-hand side. **The rule's action runs now.**

Children are reduced before parents. Trace for a file with three top-level
items `A B C`:

| Step | Stack after | Action that ran |
|---|---|---|
| 1 | `CompUnitItem(A)` | `CompUnitItem` |
| 2 | `CompUnitList` | list **base case** (once, for the leftmost item) |
| 3 | `CompUnitList CompUnitItem(B)` | `CompUnitItem` |
| 4 | `CompUnitList` | list **recursive case** |
| 5 | `CompUnitList CompUnitItem(C)` | `CompUnitItem` |
| 6 | `CompUnitList` | list **recursive case** |
| 7 | `CompUnit` | **start rule** (once, last) |

Consequences for a left-recursive list:

- The base case runs exactly once, first, so it is where the container node is
  created.
- The recursive case runs once per extra item (zero times for a one-item list),
  so it appends and passes the same container on.
- The start rule runs exactly once, after everything else.

## 2. Where values live: `$$` and `$n`

Every symbol on the stack carries a value of the `%union` member named by its
`%type` (`ast_val`, a `BaseAST*`, for nearly everything here).

- `$1`, `$2`, ... are the values of the right-hand-side symbols being popped.
- `$$` is the value the left-hand side carries when it is pushed. It is the
  **only** way to pass something to the parent rule.
- A symbol with no `%type` carries no value; using `$$`/`$n` on it is a Bison
  error ("has no declared type"). `BType` is like this today.
- Bison's C skeleton copies `$1` into `$$` before your action runs. Code that
  forgets to set `$$` can work by accident. Set it explicitly.

### How `%union` and `%type` fit together

A C++ `union` is one piece of memory with several names; only the member you
last stored into is valid, and the union does not remember which one that
was. The `%union` block becomes roughly:

```
union YYSTYPE {
  std::string *str_val;
  int          int_val;
  BaseAST     *ast_val;
};
```

Every stack slot is one of these. `%type <member> Sym` tells Bison which member
`Sym`'s slot uses, and Bison rewrites `$$` / `$n` to that member for you:
`$1` becomes something like `yyvsp[0].ast_val`. Compiler errors show this
expanded form, which helps when reading them.

So inside actions you never name the member yourself. `$$ = new string("int")`
is right for a `<str_val>` symbol; `$$.str_val = ...` picks the member twice
and fails (`(yyval.str_val).str_val`).

Tokens get their value from the lexer the same way: `yylval.str_val = new
string(yytext)` for `IDENT`. (`yylval` is the lexer's side, where you *do*
name the member, because there is no `%type` there.)

`str_val` is a pointer because a plain union cannot run `std::string`'s
constructor and destructor. A heap string is subject to the same ownership
rule as an AST node (section 4): consume it with `*unique_ptr<string>($n)`
(adopt, copy out, free at end of statement), or free it with `delete $n` when
the value is not needed (as `ConstDecl` / `VarDecl` do with `Type`).

### Not everything needs to be an AST node

The function/variable type is just a keyword, so `Type` is `<str_val>`, not an
AST node. Ask what each consumer needs from a value before giving it a class.

## 3. The start rule writes to the parse parameter, not `$$`

The start symbol has no parent, and `yyparse()` returns only an `int` status.
Its `$$` is popped and discarded. The tree leaves the parser only through the
out-parameter:

```
main.cpp:  unique_ptr<BaseAST> ast;           // empty
           yyparse(ast);                       // by reference
sysy.y:    %parse-param { std::unique_ptr<BaseAST> &ast }
           // inside every action, `ast` IS main's variable
```

Rule of thumb: inner rules hand values up with `$$`; the start rule hands the
finished tree out through `ast`.

### Shadowing trap

The template's actions use `auto ast = new FooAST();` as a local. Inside such
a block, `ast` means the local and the parse parameter is hidden, so
`ast = ast;` assigns the local to itself. Harmless in rules that never touch
the parse parameter; a silent bug in one that must. Use another local name
(`comp_unit`, `node`, ...) in any rule near the top of the grammar.

## 4. Ownership: raw pointers on the stack, `unique_ptr` in the tree

A C++ `union` cannot hold a `unique_ptr`, so `%union` holds raw pointers.
While a node sits on the parse stack nobody owns it.

> Every raw pointer is wrapped in a `unique_ptr` exactly once, by whoever
> takes ownership of it.

Zero times leaks; twice double-frees.

| You write | Meaning | Use when |
|---|---|---|
| `make_unique<T>(args)` | **Create** a new `T` and own it | No object exists yet |
| `unique_ptr<T>(p)` | **Adopt** an existing raw pointer, no new object | Something already built it (e.g. `$1`) |
| move-assign one `unique_ptr` into another, or `.reset(p)` | **Transfer** into an existing owner | The owner variable already exists |

Gotchas met along the way:

- `unique_ptr`'s raw-pointer constructor is `explicit`: assigning a raw pointer
  to a `unique_ptr` does not compile.
- `std::move` on a raw pointer does nothing useful; it is still a raw pointer.
- `make_unique<T>(p)` does **not** adopt `p`; it constructs a new `T` and passes
  `p` to `T`'s constructor.
- A `vector<unique_ptr<...>>` can be moved but not copied.

## 5. Base and derived pointers

- Derived to base (`CompUnitAST*` to `BaseAST*`) converts implicitly.
- Base to derived does not: the compiler cannot know what a `BaseAST*`
  really points to. Use `dynamic_cast` (as the recursive list case does).
- So `unique_ptr<CompUnitAST>($1)` fails when `$1` is a `BaseAST*`. The
  template argument must match the pointer you are adopting, and the owner you
  are assigning into (`unique_ptr<BaseAST>`).

## 6. Checklist for a new list rule

1. Declare the list and item nonterminals in a `%type <ast_val>` line.
2. Base case: create the container, adopt the first item, `$$` = container.
3. Recursive case: `dynamic_cast` `$1` back to the container type, adopt the
   new item, `$$` = container.
4. Decide whether the list may be empty. An empty base case and a one-item
   base case accept different languages (think `f()` versus `f(1)`).
5. If the list sits under the start symbol, only the start rule writes to
   `ast`.

## 7. Reading a conflict report

- `-Wconflicts-rr` / `-Wconflicts-sr`: Bison still produces a parser, picking
  the earlier rule (reduce/reduce) or shifting (shift/reduce). The build
  succeeds; some valid inputs then fail with "syntax error".
- `bison -Wcounterexamples` prints example inputs. The `.` marks where the
  parser stands when it cannot decide; the token after the `.` is the
  lookahead. Both derivations must be valid up to that point.
- LALR(1) decides each reduction with one token of lookahead. A conflict means
  the information that would settle it is further to the right.

### Worked example: `FuncType` versus `BType` (Lv8)

With separate `FuncType : INT | VOID` and `BType : INT`, the parser at
`INT . IDENT` had to choose a reduction before seeing whether `(` (function)
or `;` `=` `,` (variable) followed. Bison picked `FuncType` (earlier rule), so
a global `int x;` was a syntax error.

Fix: one `Type : INT | VOID` used by `FuncDef`, `VarDecl` and `ConstDecl`.
After `int` there is only one possible reduction, and the decision between
function and variable happens later, at `(`, where one token is enough.

Cost: the grammar now accepts `void x;`. That is a semantic error, not a
syntax error (GCC reports "variable or field 'x' declared void" after
parsing). The parser checks shape; semantic analysis checks meaning. The
course tests only use valid programs, so the check is optional here.
