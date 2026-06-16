#pragma once

#include "ir.hpp"

#include <memory>
#include <string>

class IRBuilder {
public:
  IRBuilder() = default;

  ir::Function *createFunction(ir::Module &module, const std::string &name,
                               ir::Type *return_type);

  ir::BasicBlock *createBlock(ir::Function *function, const std::string &hint);

  void setCurrentFunction(ir::Function *function);
  void setInsertPoint(ir::BasicBlock *block);

  ir::Function *currentFunction() const;
  ir::BasicBlock *currentBlock() const;

  ir::Value *i32(int value);

  ir::Value *emitAlloc(ir::Type *allocated_type, const std::string &hint);

  ir::Value *emitLoad(ir::Value *addr);

  void emitStore(ir::Value *value, ir::Value *addr);

  ir::Value *emitBinary(ir::BinaryOp op, ir::Value *lhs, ir::Value rhs);

  void emitJump(ir::BasicBlock *target);

  void emitBranch(ir::Value *cond, ir::BasicBlock *true_bb,
                  ir::BasicBlock *false_bb);

  void emitReturn(ir::Value *value);
  void emitReturnVoid();

private:
  ir::Function *current_function_ = nullptr;
  ir::BasicBlock *current_block_ = nullptr;

  ir::Instruction *appendInstruction(std::unique_ptr<ir::Instruction> inst);
  ir::Value *createLocalValue(ir::Type *type, const std::string &hint);
  std::string makeValueName(const std::string &hint);
  std::string makeBlockName(ir::Function *function, const std::string &hint);
};
