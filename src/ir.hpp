#pragma once

#include <memory>
#include <string>
#include <vector>

namespace ir {

struct Type;
struct Value;
struct Instruction;
struct BasicBlock;
struct Function;
struct Module;

enum class TypeKind {
  I32,
  Unit,
  Ptr,
};

enum class BinaryOp {
  Add,
  Sub,
  Mul,
  Div,
  Mod,
  Lt,
  Gt,
  Le,
  Ge,
  Eq,
  Ne,
};

enum class ValueKind {
  Constant,
  Local,
};

enum class InstKind {
  Alloc,
  Load,
  Store,
  Binary,
  Jump,
  Branch,
  Return,
};

struct Type {
  TypeKind kind;
  Type *base = nullptr;

  static Type *i32();
  static Type *unit();
  static Type *ptr(Type *base);
};

struct Value {
  ValueKind kind;
  Type *type = nullptr;

  std::string name;    // e.g. "%0"; empty for constants
  int const_value = 0; // only valid for constants

  Instruction *def = nullptr;
};

struct Instruction {
  InstKind kind;

  Value *result = nullptr;
  std::vector<Value *> operands;

  BinaryOp binary_op;

  BasicBlock *target = nullptr;
  BasicBlock *true_bb = nullptr;
  BasicBlock *false_bb = nullptr;

  bool isTerminator() const;
};

struct BasicBlock {
  std::string name;
  std::vector<std::unique_ptr<Instruction>> insts;

  bool terminated() const;
};

struct Function {
  std::string name;
  Type *return_type = nullptr;

  std::vector<std::unique_ptr<BasicBlock>> blocks;
  std::vector<std::unique_ptr<Value>> values;

  int next_value_id = 0;
  int next_block_id = 0;
};

struct Module {
  std::vector<std::unique_ptr<Function>> functions;
};

} // namespace ir
