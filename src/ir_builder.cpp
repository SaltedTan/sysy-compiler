#include "ir_builder.hpp"
#include "ir.hpp"

#include <cassert>
#include <cstddef>
#include <memory>
#include <string>

using namespace ir;

Function *IRBuilder::currentFunction() const { return current_function_; }

BasicBlock *IRBuilder::currentBlock() const { return current_block_; }

void IRBuilder::setCurrentFunction(Function *function) {
  current_function_ = function;
  current_block_ = nullptr;
}

void IRBuilder::setInsertPoint(BasicBlock *block) {
  assert(block != nullptr);
  current_block_ = block;
}

std::string IRBuilder::makeValueName(const std::string &hint) {
  assert(currentFunction() != nullptr);

  if (hint == "") {
    return "%" + std::to_string(currentFunction()->next_value_id++);
  } else {
    return "%" + hint + "_" +
           std::to_string(currentFunction()->next_value_id++);
  }
}

std::string IRBuilder::makeBlockName(Function *function,
                                     const std::string &hint) {
  assert(function != nullptr);

  if (hint == "entry" && function->blocks.empty()) {
    return "%entry";
  }

  return "%" + hint + "_" + std::to_string(function->next_block_id++);
}

Function *IRBuilder::createFunction(Module &module, const std::string &name,
                                    Type *return_type) {
  assert(return_type != nullptr);

  auto func = std::make_unique<Function>();
  func->name = name;
  func->return_type = return_type;
  Function *raw = func.get();
  module.functions.emplace_back(std::move(func));
  return raw;
}

BasicBlock *IRBuilder::createBlock(Function *function,
                                   const std::string &hint) {
  assert(function != nullptr);

  auto bb = std::make_unique<BasicBlock>();
  bb->name = makeBlockName(function, hint);
  BasicBlock *raw = bb.get();
  function->blocks.emplace_back(std::move(bb));
  return raw;
}

Value *IRBuilder::createLocalValue(Type *type, const std::string &hint) {
  assert(type != nullptr);
  assert(currentFunction() != nullptr);

  auto val = std::make_unique<Value>();
  val->kind = ValueKind::Local;
  val->type = type;
  val->name = makeValueName(hint);
  Value *raw = val.get();
  currentFunction()->values.emplace_back(std::move(val));
  return raw;
}

Instruction *IRBuilder::appendInstruction(std::unique_ptr<Instruction> inst) {
  assert(inst != nullptr);
  assert(currentBlock() != nullptr);
  assert(!currentBlock()->terminated());

  Instruction *raw = inst.get();
  currentBlock()->insts.emplace_back(std::move(inst));
  return raw;
}

Value *IRBuilder::i32(int value) {
  assert(currentFunction() != nullptr);

  auto val = std::make_unique<Value>();
  val->kind = ValueKind::Constant;
  val->type = Type::i32();
  val->const_value = value;
  Value *raw = val.get();
  currentFunction()->values.emplace_back(std::move(val));
  return raw;
}

Value *IRBuilder::emitAlloc(Type *allocated_type, const std::string &hint) {
  assert(allocated_type != nullptr);

  Value *result = createLocalValue(Type::ptr(allocated_type), hint);

  auto instruction = std::make_unique<Instruction>();
  instruction->kind = InstKind::Alloc;
  instruction->result = result;
  result->def = instruction.get();

  appendInstruction(std::move(instruction));
  return result;
}

Value *IRBuilder::emitLoad(Value *addr) {
  assert(addr != nullptr);

  Value *result = createLocalValue(Type::i32(), "load");

  auto instruction = std::make_unique<Instruction>();
  instruction->kind = InstKind::Load;
  instruction->result = result;
  instruction->operands = {addr};
  result->def = instruction.get();

  appendInstruction(std::move(instruction));
  return result;
}

void IRBuilder::emitStore(Value *value, Value *addr) {
  assert(value != nullptr);
  assert(addr != nullptr);

  auto instruction = std::make_unique<Instruction>();
  instruction->kind = InstKind::Store;
  instruction->operands = {value, addr};

  appendInstruction(std::move(instruction));
}

Value *IRBuilder::emitBinary(BinaryOp op, Value *lhs, Value *rhs) {
  assert(lhs != nullptr);
  assert(rhs != nullptr);

  Value *result = createLocalValue(Type::i32(), "bin");

  auto instruction = std::make_unique<Instruction>();
  instruction->kind = InstKind::Binary;
  instruction->result = result;
  instruction->binary_op = op;
  instruction->operands = {lhs, rhs};
  result->def = instruction.get();

  appendInstruction(std::move(instruction));
  return result;
}

void IRBuilder::emitJump(BasicBlock *target) {
  assert(target != nullptr);

  auto instruction = std::make_unique<Instruction>();
  instruction->kind = InstKind::Jump;
  instruction->target = target;

  appendInstruction(std::move(instruction));
}

void IRBuilder::emitBranch(Value *cond, BasicBlock *true_bb,
                           BasicBlock *false_bb) {
  assert(cond != nullptr);
  assert(true_bb != nullptr);
  assert(false_bb != nullptr);

  auto instruction = std::make_unique<Instruction>();
  instruction->kind = InstKind::Branch;
  instruction->operands = {cond};
  instruction->true_bb = true_bb;
  instruction->false_bb = false_bb;

  appendInstruction(std::move(instruction));
}

void IRBuilder::emitReturn(Value *value) {
  assert(value != nullptr);

  auto instruction = std::make_unique<Instruction>();
  instruction->kind = InstKind::Return;
  instruction->operands = {value};

  appendInstruction(std::move(instruction));
}

void IRBuilder::emitReturnVoid() {
  auto instruction = std::make_unique<Instruction>();
  instruction->kind = InstKind::Return;
  instruction->operands = {};

  appendInstruction(std::move(instruction));
}
