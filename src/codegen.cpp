#include "codegen.hpp"
#include "koopa.h"
#include <cassert>
#include <iostream>
#include <string>

using namespace std;

void RiscVGenerator::Generate(const koopa_raw_program_t &program) {
  Visit(program.values);
  Visit(program.funcs);
}

void RiscVGenerator::Visit(const koopa_raw_slice_t &slice) {
  for (size_t i = 0; i < slice.len; ++i) {
    auto ptr = slice.buffer[i];
    switch (slice.kind) {
    case KOOPA_RSIK_FUNCTION:
      Visit(reinterpret_cast<koopa_raw_function_t>(ptr));
      break;
    case KOOPA_RSIK_BASIC_BLOCK:
      Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
      break;
    case KOOPA_RSIK_VALUE:
      Visit(reinterpret_cast<koopa_raw_value_t>(ptr));
      break;
    default:
      assert(false);
    }
  }
}

void RiscVGenerator::Visit(const koopa_raw_function_t &func) {
  stack_map.clear();
  current_stack_size = 0;
  aligned_stack_size = 0;

  for (size_t i = 0; i < func->bbs.len; ++i) {
    auto ptr = func->bbs.buffer[i];
    auto bb = reinterpret_cast<koopa_raw_basic_block_t>(ptr);

    for (size_t j = 0; j < bb->insts.len; ++j) {
      auto ptr_isnt = bb->insts.buffer[j];
      auto inst = reinterpret_cast<koopa_raw_value_t>(ptr_isnt);

      if (inst->ty->tag != KOOPA_RTT_UNIT) {
        stack_map[inst] = current_stack_size;
        current_stack_size += 4;
      }
    }
  }

  aligned_stack_size = (current_stack_size + 15) / 16 * 16;

  string func_name = func->name;
  func_name = func_name.substr(1); // Drops the '@'

  cout << "  .text" << endl;
  cout << "  .globl " << func_name << endl;
  cout << func_name << ":" << endl;

  if (aligned_stack_size > 0) {
    cout << "  addi sp, sp, -" << aligned_stack_size << endl;
  }

  Visit(func->bbs);
}

void RiscVGenerator::Visit(const koopa_raw_basic_block_t &bb) {
  string name = bb->name;
  name.erase(0, 1);
  cout << endl << name << ":" << endl;
  Visit(bb->insts);
}

void RiscVGenerator::Visit(const koopa_raw_value_t &value) {
  const auto &kind = value->kind;

  switch (kind.tag) {
  case KOOPA_RVT_RETURN:
    VisitReturn(kind.data.ret);
    break;

  case KOOPA_RVT_INTEGER: {
    // FetchOperand handles integers
    break;
  }

  case KOOPA_RVT_BINARY: {
    VisitBinary(kind.data.binary, value);
    break;
  }

  case KOOPA_RVT_ALLOC: {
    // The memory was already allocated by the prologue in Pass 1
    break;
  }

  case KOOPA_RVT_STORE: {
    VisitStore(kind.data.store);
    break;
  }

  case KOOPA_RVT_LOAD: {
    VisitLoad(kind.data.load, value);
    break;
  }

  case KOOPA_RVT_BRANCH: {
    VisitBranch(kind.data.branch);
    break;
  }

  case KOOPA_RVT_JUMP: {
    VisitJump(kind.data.jump);
    break;
  }

  default:
    assert(false);
  }
}

void RiscVGenerator::VisitReturn(const koopa_raw_return_t &ret) {
  if (ret.value) {
    string reg = FetchOperand(ret.value, "a0");
    if (reg != "a0") {
      cout << "  mv a0, " << reg << endl;
    }
  }

  if (aligned_stack_size > 0) {
    cout << "  addi sp, sp, " << aligned_stack_size << endl;
  }
  cout << "  ret" << endl;
}

void RiscVGenerator::VisitBinary(const koopa_raw_binary_t &binary,
                                 const koopa_raw_value_t &value) {
  string left_reg = FetchOperand(binary.lhs, "t0");
  string right_reg = FetchOperand(binary.rhs, "t1");

  string dest_reg = "t2";

  switch (binary.op) {
  case KOOPA_RBO_EQ:
    cout << " xor " << dest_reg << ", " << left_reg << ", " << right_reg
         << endl;
    cout << " seqz " << dest_reg << ", " << dest_reg << endl;
    break;
  case KOOPA_RBO_MUL:
    cout << " mul " << dest_reg << ", " << left_reg << ", " << right_reg
         << endl;
    break;
  case KOOPA_RBO_DIV:
    cout << " div " << dest_reg << ", " << left_reg << ", " << right_reg
         << endl;
    break;
  case KOOPA_RBO_MOD:
    cout << " rem " << dest_reg << ", " << left_reg << ", " << right_reg
         << endl;
    break;
  case KOOPA_RBO_ADD:
    cout << " add " << dest_reg << ", " << left_reg << ", " << right_reg
         << endl;
    break;
  case KOOPA_RBO_SUB:
    cout << " sub " << dest_reg << ", " << left_reg << ", " << right_reg
         << endl;
    break;
  case KOOPA_RBO_LT:
    cout << " slt " << dest_reg << ", " << left_reg << ", " << right_reg
         << endl;
    break;
  case KOOPA_RBO_GT:
    cout << " sgt " << dest_reg << ", " << left_reg << ", " << right_reg
         << endl;
    break;
  case KOOPA_RBO_LE:
    cout << " sgt " << dest_reg << ", " << left_reg << ", " << right_reg
         << endl;
    cout << " seqz " << dest_reg << ", " << dest_reg << endl;
    break;
  case KOOPA_RBO_GE:
    cout << " slt " << dest_reg << ", " << left_reg << ", " << right_reg
         << endl;
    cout << " seqz " << dest_reg << ", " << dest_reg << endl;
    break;
  case KOOPA_RBO_NOT_EQ:
    cout << " xor " << dest_reg << ", " << left_reg << ", " << right_reg
         << endl;
    cout << " snez " << dest_reg << ", " << dest_reg << endl;
    break;
  case KOOPA_RBO_AND:
    cout << " and " << dest_reg << ", " << left_reg << ", " << right_reg
         << endl;
    break;
  case KOOPA_RBO_OR:
    cout << " or " << dest_reg << ", " << left_reg << ", " << right_reg << endl;
    break;
  default:
    assert(false);
  }
  int dest_offset = stack_map[value];
  cout << "  sw " << dest_reg << ", " << dest_offset << "(sp)" << endl;
}

void RiscVGenerator::VisitStore(const koopa_raw_store_t &store) {
  string val_reg = FetchOperand(store.value, "t0");
  int offset = stack_map[store.dest];

  cout << "  sw " << val_reg << ", " << offset << "(sp)" << endl;
}

void RiscVGenerator::VisitLoad(const koopa_raw_load_t &load,
                               const koopa_raw_value_t &value) {
  int src_offset = stack_map[load.src];
  cout << "  lw t0, " << src_offset << "(sp)" << endl;

  int dest_offset = stack_map[value];
  cout << "  sw t0, " << dest_offset << "(sp)" << endl;
}

void RiscVGenerator::VisitBranch(const koopa_raw_branch_t &branch) {
  string cond_reg = FetchOperand(branch.cond, "t0");

  string true_target_name = branch.true_bb->name;
  true_target_name.erase(0, 1);

  string false_target_name = branch.false_bb->name;
  false_target_name.erase(0, 1);

  cout << "  bnez " << cond_reg << ", " << true_target_name << endl;
  cout << "  j " << false_target_name << endl;
}

void RiscVGenerator::VisitJump(const koopa_raw_jump_t &jump) {
  string target_name = jump.target->name;
  target_name.erase(0, 1);

  cout << "  j " << target_name << endl;
}

string RiscVGenerator::FetchOperand(const koopa_raw_value_t &operand,
                                    const string &temp_reg) {
  if (operand->kind.tag == KOOPA_RVT_INTEGER) {
    int val = operand->kind.data.integer.value;
    if (val == 0)
      return "x0";

    cout << "  li " << temp_reg << ", " << val << endl;
    return temp_reg;
  }

  int offset = stack_map[operand];
  cout << "  lw " << temp_reg << ", " << offset << "(sp)" << endl;
  return temp_reg;
}
